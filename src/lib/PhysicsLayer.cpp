#include "DefaultResources.hpp"
#include "Joint.hpp"
#include <Application.hpp>
#include <ProjectManager.hpp>
#include <PhysicsLayer.hpp>
#include <RigidBody.hpp>
#include <TransformLayer.hpp>
using namespace UniEngine;

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const PxVec2 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const PxVec3 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const PxVec4 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const PxMat44 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v[0] << v[1] << v[2] << v[3] << YAML::EndSeq;
    return out;
}

void PhysicsLayer::UploadTransform(
    const GlobalTransform &globalTransform, const std::shared_ptr<RigidBody> &rigidBody)
{
    GlobalTransform ltw;
    ltw.m_value = globalTransform.m_value * rigidBody->m_shapeTransform;
    ltw.SetScale(glm::vec3(1.0f));

    if (rigidBody->m_currentRegistered && rigidBody->m_kinematic)
    {
        static_cast<PxRigidDynamic *>(rigidBody->m_rigidActor)
            ->setKinematicTarget(PxTransform(*(PxMat44 *)(void *)&ltw.m_value));
    }
    else
    {
        rigidBody->m_rigidActor->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&ltw.m_value));
    }
}

void PhysicsLayer::PreUpdate()
{
    const bool playing = Application::IsPlaying();
    auto activeScene = GetScene();
    UploadRigidBodyShapes(activeScene);
    UploadTransforms(activeScene, !playing);
    UploadJointLinks(activeScene);
}
void PhysicsLayer::OnCreate()
{
    m_physicsFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

    m_pvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    if (m_pvdTransport != NULL)
    {
        m_physVisDebugger = PxCreatePvd(*m_physicsFoundation);
        m_physVisDebugger->connect(*m_pvdTransport, PxPvdInstrumentationFlag::eALL);
    }
    m_physics = PxCreatePhysics(
        PX_PHYSICS_VERSION,
        *m_physicsFoundation,
        PxTolerancesScale(),
        true,
        m_physVisDebugger);
    PxInitExtensions(*m_physics, m_physVisDebugger);
    m_dispatcher = PxDefaultCpuDispatcherCreate(Jobs::Workers().Size());

    #pragma region Physics
    m_defaultPhysicsMaterial =
        ProjectManager::CreateDefaultResource<PhysicsMaterial>(DefaultResources::GenerateNewHandle(), "Default");
#pragma endregion
}

#define PX_RELEASE(x)                                                                                                  \
    if (x)                                                                                                             \
    {                                                                                                                  \
        x->release();                                                                                                  \
        x = nullptr;                                                                                                   \
    }
void PhysicsLayer::OnDestroy()
{
    PX_RELEASE(m_dispatcher);
    PX_RELEASE(m_physics);
    PX_RELEASE(m_physVisDebugger);
    PX_RELEASE(m_pvdTransport);

    // PX_RELEASE(m_physicsFoundation);
}
void PhysicsLayer::UploadTransforms(const std::shared_ptr<Scene> &scene, const bool &updateAll, const bool &freeze)
{
    if (!scene)
        return;
    if (const std::vector<Entity> *entities = scene->UnsafeGetPrivateComponentOwnersList<RigidBody>();
        entities != nullptr)
    {
        for (auto entity : *entities)
        {
            auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(entity).lock();
            auto globalTransform = scene->GetDataComponent<GlobalTransform>(entity);
            globalTransform.m_value = globalTransform.m_value * rigidBody->m_shapeTransform;
            globalTransform.SetScale(glm::vec3(1.0f));
            if (rigidBody->m_currentRegistered)
            {
                if (rigidBody->m_kinematic)
                {
                    if (freeze || updateAll)
                    {
                        static_cast<PxRigidDynamic *>(rigidBody->m_rigidActor)
                            ->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                    }
                    else
                    {
                        static_cast<PxRigidDynamic *>(rigidBody->m_rigidActor)
                            ->setKinematicTarget(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                    }
                }
                else if (updateAll)
                {
                    rigidBody->m_rigidActor->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                    if (freeze)
                    {
                        rigidBody->SetLinearVelocity(glm::vec3(0.0f));
                        rigidBody->SetAngularVelocity(glm::vec3(0.0f));
                    }
                }
            }
        }
    }
}

void PhysicsSystem::OnCreate()
{
    m_scene = std::make_shared<PhysicsScene>();
    auto physicsScene = m_scene->m_physicsScene;
    if(!physicsScene) return;
    PxPvdSceneClient *pvdClient = physicsScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    Enable();
}

void PhysicsSystem::OnDestroy()
{
}

void PhysicsSystem::FixedUpdate()
{
    Simulate(Application::Time().TimeStep());
}

void PhysicsScene::Simulate(float time) const
{
    if(!m_physicsScene) return;
    m_physicsScene->simulate(time);
    m_physicsScene->fetchResults(true);
}

void PhysicsSystem::Simulate(float time) const
{
    const std::vector<Entity> *rigidBodyEntities =
        GetScene()->UnsafeGetPrivateComponentOwnersList<RigidBody>();
    if (!rigidBodyEntities)
        return;
    m_scene->Simulate(time);
    DownloadRigidBodyTransforms(rigidBodyEntities);

    Application::GetLayer<TransformLayer>()->m_physicsSystemOverride = true;
}

void PhysicsSystem::OnEnable()
{
}
void PhysicsSystem::DownloadRigidBodyTransforms() const
{
    const std::vector<Entity> *rigidBodyEntities =
        GetScene()->UnsafeGetPrivateComponentOwnersList<RigidBody>();
    if (rigidBodyEntities)
        DownloadRigidBodyTransforms(rigidBodyEntities);
}

void PhysicsLayer::UploadRigidBodyShapes(
    const std::shared_ptr<Scene> &scene,
    const std::shared_ptr<PhysicsScene> &physicsScene,
    const std::vector<Entity> *rigidBodyEntities)
{
#pragma region Update shape
    for (auto entity : *rigidBodyEntities)
    {
        auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(entity).lock();
        bool shouldRegister = scene->IsEntityValid(entity) && scene->IsEntityEnabled(entity) && rigidBody->IsEnabled();
        if (rigidBody->m_currentRegistered == false && shouldRegister)
        {
            rigidBody->m_currentRegistered = true;
            physicsScene->m_physicsScene->addActor(*rigidBody->m_rigidActor);
        }
        else if (rigidBody->m_currentRegistered == true && !shouldRegister)
        {
            rigidBody->m_currentRegistered = false;
            physicsScene->m_physicsScene->removeActor(*rigidBody->m_rigidActor);
        }
    }
#pragma endregion
}
void PhysicsLayer::UploadRigidBodyShapes(const std::shared_ptr<Scene> &scene)
{
    if (!scene)
        return;
    auto physicsSystem = scene->GetSystem<PhysicsSystem>();
    if (!physicsSystem)
        return;
    const std::vector<Entity> *rigidBodyEntities = scene->UnsafeGetPrivateComponentOwnersList<RigidBody>();
    if (rigidBodyEntities)
    {
        auto physicsScene = physicsSystem->m_scene;
        UploadRigidBodyShapes(scene, physicsScene, rigidBodyEntities);
    }
}
void PhysicsLayer::UploadJointLinks(const std::shared_ptr<Scene> &scene)
{
    if (!scene)
        return;
    auto physicsSystem = scene->GetSystem<PhysicsSystem>();
    if (!physicsSystem)
        return;
    const std::vector<Entity> *jointEntities = scene->UnsafeGetPrivateComponentOwnersList<Joint>();
    if (jointEntities)
    {
        auto physicsScene = physicsSystem->m_scene;
        UploadJointLinks(scene, physicsScene, jointEntities);
    }
}
void PhysicsLayer::UploadJointLinks(
    const std::shared_ptr<Scene> &scene,
    const std::shared_ptr<PhysicsScene> &physicsScene,
    const std::vector<Entity> *jointEntities)
{
    auto physicsLayer = Application::GetLayer<PhysicsLayer>();
    if(!physicsLayer) return;
#pragma region Update shape
    for (auto entity : *jointEntities)
    {
        auto joint = scene->GetOrSetPrivateComponent<Joint>(entity).lock();
        auto rigidBody1 = joint->m_rigidBody1.Get<RigidBody>();
        auto rigidBody2 = joint->m_rigidBody2.Get<RigidBody>();
        bool shouldRegister = scene->IsEntityValid(entity) && scene->IsEntityEnabled(entity) && joint->IsEnabled() &&
                              (rigidBody1 && rigidBody2);
        if (!joint->m_linked && shouldRegister)
        {
            PxTransform localFrame1;
            auto ownerGT = scene->GetDataComponent<GlobalTransform>(rigidBody1->GetOwner());
            ownerGT.SetScale(glm::vec3(1.0f));
            auto linkerGT = scene->GetDataComponent<GlobalTransform>(rigidBody2->GetOwner());
            linkerGT.SetScale(glm::vec3(1.0f));
            Transform transform;
            transform.m_value = glm::inverse(ownerGT.m_value) * linkerGT.m_value;
            joint->m_localPosition1 = glm::vec3(0.0f);
            joint->m_localRotation1 = glm::vec3(0.0f);

            joint->m_localPosition2 = transform.GetPosition();
            joint->m_localRotation2 = transform.GetRotation();

            switch (joint->m_jointType)
            {
            case JointType::Fixed:
                joint->m_joint = PxFixedJointCreate(
                    *physicsLayer->m_physics,
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(joint->m_localPosition1.x, joint->m_localPosition1.y, joint->m_localPosition1.z),
                        PxQuat(
                            joint->m_localRotation1.x,
                            joint->m_localRotation1.y,
                            joint->m_localRotation1.z,
                            joint->m_localRotation1.w)),
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(joint->m_localPosition2.x, joint->m_localPosition2.y, joint->m_localPosition2.z),
                        PxQuat(
                            joint->m_localRotation2.x,
                            joint->m_localRotation2.y,
                            joint->m_localRotation2.z,
                            joint->m_localRotation2.w)));
                break;
                /*
            case JointType::Distance:
                m_joint = PxDistanceJointCreate(
                    *PhysicsManager::GetInstance().m_physics,
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition1.x, m_localPosition1.y, m_localPosition1.z),
                        PxQuat(m_localRotation1.x, m_localRotation1.y, m_localRotation1.z, m_localRotation1.w)),
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition2.x, m_localPosition2.y, m_localPosition2.z),
                        PxQuat(m_localRotation2.x, m_localRotation2.y, m_localRotation2.z, m_localRotation2.w)));
                break;
            case JointType::Spherical: {
                m_joint = PxSphericalJointCreate(
                    *PhysicsManager::GetInstance().m_physics,
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition2.x, m_localPosition2.y, m_localPosition2.z),
                        PxQuat(m_localRotation2.x, m_localRotation2.y, m_localRotation2.z, m_localRotation2.w)),
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition1.x, m_localPosition1.y, m_localPosition1.z),
                        PxQuat(m_localRotation1.x, m_localRotation1.y, m_localRotation1.z, m_localRotation1.w)));
                // static_cast<PxSphericalJoint *>(m_joint)->setLimitCone(PxJointLimitCone(PxPi / 2, PxPi / 6,
            0.01f));
                // static_cast<PxSphericalJoint
            *>(m_joint)->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED,
                // true);
            }
            break;
            case JointType::Revolute:
                m_joint = PxRevoluteJointCreate(
                    *PhysicsManager::GetInstance().m_physics,
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition1.x, m_localPosition1.y, m_localPosition1.z),
                        PxQuat(m_localRotation1.x, m_localRotation1.y, m_localRotation1.z, m_localRotation1.w)),
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition2.x, m_localPosition2.y, m_localPosition2.z),
                        PxQuat(m_localRotation2.x, m_localRotation2.y, m_localRotation2.z, m_localRotation2.w)));
                break;
            case JointType::Prismatic:
                m_joint = PxPrismaticJointCreate(
                    *PhysicsManager::GetInstance().m_physics,
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition1.x, m_localPosition1.y, m_localPosition1.z),
                        PxQuat(m_localRotation1.x, m_localRotation1.y, m_localRotation1.z, m_localRotation1.w)),
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(m_localPosition2.x, m_localPosition2.y, m_localPosition2.z),
                        PxQuat(m_localRotation2.x, m_localRotation2.y, m_localRotation2.z, m_localRotation2.w)));
                break;
                 */
            case JointType::D6:
                joint->m_joint = PxD6JointCreate(
                    *physicsLayer->m_physics,
                    rigidBody2->m_rigidActor,
                    PxTransform(
                        PxVec3(joint->m_localPosition1.x, joint->m_localPosition1.y, joint->m_localPosition1.z),
                        PxQuat(
                            joint->m_localRotation1.x,
                            joint->m_localRotation1.y,
                            joint->m_localRotation1.z,
                            joint->m_localRotation1.w)),
                    rigidBody1->m_rigidActor,
                    PxTransform(
                        PxVec3(joint->m_localPosition2.x, joint->m_localPosition2.y, joint->m_localPosition2.z),
                        PxQuat(
                            joint->m_localRotation2.x,
                            joint->m_localRotation2.y,
                            joint->m_localRotation2.z,
                            joint->m_localRotation2.w)));
                static_cast<PxD6Joint *>(joint->m_joint)->setProjectionAngularTolerance(1.0f);
                static_cast<PxD6Joint *>(joint->m_joint)->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);

                for (int i = 0; i < 6; i++)
                {
                    joint->SetMotion((MotionAxis)i, (MotionType)joint->m_motionTypes[i]);
                }
                for (int i = 0; i < 6; i++)
                {
                    joint->SetDrive(
                        (DriveType)i,
                        joint->m_drives[i].stiffness,
                        joint->m_drives[i].damping,
                        joint->m_drives[i].flags == PxD6JointDriveFlag::eACCELERATION);
                }
                break;
            }
            joint->m_linked = true;
        }
        else if (joint->m_linked && !shouldRegister)
        {
            joint->Unlink();
        }
    }
#pragma endregion
}

void PhysicsSystem::DownloadRigidBodyTransforms(const std::vector<Entity> *rigidBodyEntities) const
{
    std::vector<std::shared_future<void>> futures;
    auto &list = rigidBodyEntities;
    auto threadSize = Jobs::Workers().Size();
    size_t capacity = rigidBodyEntities->size() / threadSize;
    size_t reminder = rigidBodyEntities->size() % threadSize;
    auto scene = GetScene();
    for (size_t i = 0; i < threadSize; i++)
    {
        futures.push_back(Jobs::Workers()
                              .Push([&list, i, capacity, reminder, threadSize, &scene](int id) {
                                  for (size_t j = 0; j < capacity; j++)
                                  {
                                      size_t index = capacity * i + j;
                                      auto rigidBodyEntity = list->at(index);
                                      auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(rigidBodyEntity).lock();
                                      if (rigidBody->m_currentRegistered && !rigidBody->m_kinematic)
                                      {
                                          PxTransform transform = rigidBody->m_rigidActor->getGlobalPose();
                                          glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                          glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                          glm::vec3 scale =
                                              scene->GetDataComponent<GlobalTransform>(rigidBodyEntity).GetScale();
                                          GlobalTransform globalTransform;
                                          globalTransform.SetValue(position, rotation, scale);
                                          scene->SetDataComponent(rigidBodyEntity, globalTransform);
                                          if (!rigidBody->m_static)
                                          {
                                              PxRigidBody *rb = static_cast<PxRigidBody *>(rigidBody->m_rigidActor);
                                              rigidBody->m_linearVelocity = rb->getLinearVelocity();
                                              rigidBody->m_angularVelocity = rb->getAngularVelocity();
                                          }
                                      }
                                  }
                                  if (reminder > i)
                                  {
                                      size_t index = capacity * threadSize + i;
                                      auto rigidBodyEntity = list->at(index);
                                      auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(rigidBodyEntity).lock();
                                      if (rigidBody->m_currentRegistered && !rigidBody->m_kinematic)
                                      {
                                          PxTransform transform = rigidBody->m_rigidActor->getGlobalPose();
                                          glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                          glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                          glm::vec3 scale =
                                              scene->GetDataComponent<GlobalTransform>(rigidBodyEntity).GetScale();
                                          GlobalTransform globalTransform;
                                          globalTransform.SetValue(position, rotation, scale);
                                          scene->SetDataComponent(rigidBodyEntity, globalTransform);
                                          if (!rigidBody->m_static)
                                          {
                                              PxRigidBody *rb = static_cast<PxRigidBody *>(rigidBody->m_rigidActor);
                                              rigidBody->m_linearVelocity = rb->getLinearVelocity();
                                              rigidBody->m_angularVelocity = rb->getAngularVelocity();
                                          }
                                      }
                                  }
                              })
                              .share());
    }
    for (const auto &i : futures)
        i.wait();
}

PhysicsScene::PhysicsScene()
{
    auto physicsLayer = Application::GetLayer<PhysicsLayer>();
    if(!physicsLayer) return;
    auto physics = physicsLayer->m_physics;
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.solverType = PxSolverType::eTGS;
    sceneDesc.cpuDispatcher = physicsLayer->m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_physicsScene = physics->createScene(sceneDesc);
}

PhysicsScene::~PhysicsScene()
{
    PX_RELEASE(m_physicsScene);
}
