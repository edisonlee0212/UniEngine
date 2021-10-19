#include "Joint.hpp"
#include <Application.hpp>
#include <AssetManager.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>
#include <TransformManager.hpp>
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

void PhysicsManager::UploadTransform(
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

void PhysicsManager::PreUpdate()
{
    auto physicsManager = GetInstance();
    const bool playing = Application::IsPlaying();
    auto activeScene = EntityManager::GetCurrentScene();
    UploadRigidBodyShapes(activeScene);
    UploadTransforms(!playing);
    UploadJointLinks(activeScene);
}

void PhysicsManager::Init()
{
    auto &physicsManager = GetInstance();
    physicsManager.m_physicsFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, physicsManager.m_allocator, physicsManager.m_errorCallback);

    physicsManager.m_pvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    if (physicsManager.m_pvdTransport != NULL)
    {
        physicsManager.m_physVisDebugger = PxCreatePvd(*physicsManager.m_physicsFoundation);
        physicsManager.m_physVisDebugger->connect(*physicsManager.m_pvdTransport, PxPvdInstrumentationFlag::eALL);
    }
    physicsManager.m_physics = PxCreatePhysics(
        PX_PHYSICS_VERSION,
        *physicsManager.m_physicsFoundation,
        PxTolerancesScale(),
        true,
        physicsManager.m_physVisDebugger);
    PxInitExtensions(*physicsManager.m_physics, physicsManager.m_physVisDebugger);
    physicsManager.m_dispatcher = PxDefaultCpuDispatcherCreate(JobManager::PrimaryWorkers().Size());
}

#define PX_RELEASE(x)                                                                                                  \
    if (x)                                                                                                             \
    {                                                                                                                  \
        x->release();                                                                                                  \
        x = nullptr;                                                                                                   \
    }
void PhysicsManager::Destroy()
{
    auto &physicsManager = GetInstance();
    PX_RELEASE(physicsManager.m_dispatcher);
    PX_RELEASE(physicsManager.m_physics);
    PX_RELEASE(physicsManager.m_physVisDebugger);
    PX_RELEASE(physicsManager.m_pvdTransport);

    // PX_RELEASE(physicsManager.m_physicsFoundation);
}
void PhysicsManager::UploadTransforms(const bool &updateAll, const bool &freeze)
{
    if (const std::vector<Entity> *entities =
            EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>(EntityManager::GetCurrentScene());
        entities != nullptr)
    {
        for (auto entity : *entities)
        {
            auto rigidBody = entity.GetOrSetPrivateComponent<RigidBody>().lock();
            auto globalTransform = entity.GetDataComponent<GlobalTransform>();
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
    PxPvdSceneClient *pvdClient = m_scene->m_physicsScene->getScenePvdClient();
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
    m_physicsScene->simulate(time);
    m_physicsScene->fetchResults(true);
}

void PhysicsSystem::Simulate(float time) const
{
    const std::vector<Entity> *rigidBodyEntities =
        EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>(EntityManager::GetCurrentScene());
    if (!rigidBodyEntities)
        return;
    m_scene->Simulate(time);
    DownloadRigidBodyTransforms(rigidBodyEntities);
    TransformManager::GetInstance().m_physicsSystemOverride = true;
}

void PhysicsSystem::OnEnable()
{
}
void PhysicsSystem::DownloadRigidBodyTransforms() const
{
    const std::vector<Entity> *rigidBodyEntities =
        EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>(EntityManager::GetCurrentScene());
    if (rigidBodyEntities)
        DownloadRigidBodyTransforms(rigidBodyEntities);
}

void PhysicsManager::UploadRigidBodyShapes(
    const std::shared_ptr<Scene> &scene,
    const std::shared_ptr<PhysicsScene> &physicsScene,
    const std::vector<Entity> *rigidBodyEntities)
{
#pragma region Update shape
    for (auto entity : *rigidBodyEntities)
    {
        auto rigidBody = EntityManager::GetOrSetPrivateComponent<RigidBody>(scene, entity).lock();
        bool shouldRegister = EntityManager::IsEntityValid(scene, entity) &&
                              EntityManager::IsEntityEnabled(scene, entity) && rigidBody->IsEnabled();
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
void PhysicsManager::UploadRigidBodyShapes(const std::shared_ptr<Scene> &scene)
{
    auto physicsManager = GetInstance();
    auto physicsSystem = scene->GetSystem<PhysicsSystem>();
    if (!physicsSystem)
        return;
    const std::vector<Entity> *rigidBodyEntities = EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>(scene);
    if (rigidBodyEntities)
    {
        auto physicsScene = physicsSystem->m_scene;
        UploadRigidBodyShapes(scene, physicsScene, rigidBodyEntities);
    }
}
void PhysicsManager::UploadJointLinks(const std::shared_ptr<Scene> &scene)
{
    auto physicsManager = GetInstance();
    auto physicsSystem = scene->GetSystem<PhysicsSystem>();
    if (!physicsSystem)
        return;
    const std::vector<Entity> *jointEntities = EntityManager::UnsafeGetPrivateComponentOwnersList<Joint>(scene);
    if (jointEntities)
    {
        auto physicsScene = physicsSystem->m_scene;
        UploadJointLinks(scene, physicsScene, jointEntities);
    }
}
void PhysicsManager::UploadJointLinks(
    const std::shared_ptr<Scene> &scene,
    const std::shared_ptr<PhysicsScene> &physicsScene,
    const std::vector<Entity> *jointEntities)
{
#pragma region Update shape
    for (auto entity : *jointEntities)
    {
        auto joint = EntityManager::GetOrSetPrivateComponent<Joint>(scene, entity).lock();
        auto rigidBody1 = joint->m_rigidBody1.Get<RigidBody>();
        auto rigidBody2 = joint->m_rigidBody2.Get<RigidBody>();
        bool shouldRegister = EntityManager::IsEntityValid(scene, entity) &&
                              EntityManager::IsEntityEnabled(scene, entity) && joint->IsEnabled() &&
                              (rigidBody1 && rigidBody2);
        if (joint->m_linked == false && shouldRegister)
        {
            PxTransform localFrame1;
            auto ownerGT = EntityManager::GetDataComponent<GlobalTransform>(scene, rigidBody1->GetOwner());
            ownerGT.SetScale(glm::vec3(1.0f));
            auto linkerGT = EntityManager::GetDataComponent<GlobalTransform>(scene, rigidBody2->GetOwner());
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
                    *PhysicsManager::GetInstance().m_physics,
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
                    *PhysicsManager::GetInstance().m_physics,
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
        else if (joint->m_linked == true && !shouldRegister)
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
    auto threadSize = JobManager::PrimaryWorkers().Size();
    size_t capacity = rigidBodyEntities->size() / threadSize;
    size_t reminder = rigidBodyEntities->size() % threadSize;
    for (size_t i = 0; i < threadSize; i++)
    {
        futures.push_back(JobManager::PrimaryWorkers()
                              .Push([&list, i, capacity, reminder, threadSize](int id) {
                                  for (size_t j = 0; j < capacity; j++)
                                  {
                                      size_t index = capacity * i + j;
                                      const auto &rigidBodyEntity = list->at(index);
                                      auto rigidBody = rigidBodyEntity.GetOrSetPrivateComponent<RigidBody>().lock();
                                      if (rigidBody->m_currentRegistered && !rigidBody->m_kinematic)
                                      {
                                          PxTransform transform = rigidBody->m_rigidActor->getGlobalPose();
                                          glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                          glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                          glm::vec3 scale =
                                              rigidBodyEntity.GetDataComponent<GlobalTransform>().GetScale();
                                          GlobalTransform globalTransform;
                                          globalTransform.SetValue(position, rotation, scale);
                                          rigidBodyEntity.SetDataComponent(globalTransform);
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
                                      const auto &rigidBodyEntity = list->at(index);
                                      auto rigidBody = rigidBodyEntity.GetOrSetPrivateComponent<RigidBody>().lock();
                                      if (rigidBody->m_currentRegistered && !rigidBody->m_kinematic)
                                      {
                                          PxTransform transform = rigidBody->m_rigidActor->getGlobalPose();
                                          glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                          glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                          glm::vec3 scale =
                                              rigidBodyEntity.GetDataComponent<GlobalTransform>().GetScale();
                                          GlobalTransform globalTransform;
                                          globalTransform.SetValue(position, rotation, scale);
                                          rigidBodyEntity.SetDataComponent(globalTransform);
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
    auto physics = PhysicsManager::GetInstance().m_physics;
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.solverType = PxSolverType::eTGS;
    sceneDesc.cpuDispatcher = PhysicsManager::GetInstance().m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_physicsScene = physics->createScene(sceneDesc);
}

PhysicsScene::~PhysicsScene()
{
    PX_RELEASE(m_physicsScene);
}
