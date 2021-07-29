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

void PhysicsManager::UploadTransform(const GlobalTransform &globalTransform, RigidBody &rigidBody)
{
    GlobalTransform ltw;
    ltw.m_value = globalTransform.m_value * rigidBody.m_shapeTransform;
    ltw.SetScale(glm::vec3(1.0f));

    if (rigidBody.m_currentRegistered && rigidBody.m_kinematic)
    {
        static_cast<PxRigidDynamic *>(rigidBody.m_rigidActor)
            ->setKinematicTarget(PxTransform(*(PxMat44 *)(void *)&ltw.m_value));
    }
    else
    {
        rigidBody.m_rigidActor->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&ltw.m_value));
    }
}

void PhysicsManager::PreUpdate()
{
    const bool playing = Application::IsPlaying();
    UploadTransforms(!playing);
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
    if (physicsManager.m_physVisDebugger)
    {
        PxPvdTransport *transport = physicsManager.m_physVisDebugger->getTransport();
        physicsManager.m_physVisDebugger->release();
        physicsManager.m_physVisDebugger = nullptr;
        PX_RELEASE(transport);
    }
    PX_RELEASE(physicsManager.m_physicsFoundation);
}
void PhysicsManager::UploadTransforms(const bool &updateAll, const bool& freeze)
{
    if (const std::vector<Entity> *entities = EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>();
            entities != nullptr)
    {
        for (auto entity : *entities)
        {
            auto &rigidBody = entity.GetPrivateComponent<RigidBody>();
            auto globalTransform = entity.GetDataComponent<GlobalTransform>();
            globalTransform.m_value = globalTransform.m_value * rigidBody.m_shapeTransform;
            globalTransform.SetScale(glm::vec3(1.0f));
            if (rigidBody.m_currentRegistered && rigidBody.m_kinematic)
            {
                if(freeze){
                    static_cast<PxRigidDynamic *>(rigidBody.m_rigidActor)
                            ->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                }
                else{
                    static_cast<PxRigidDynamic *>(rigidBody.m_rigidActor)
                        ->setKinematicTarget(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                }
                if(freeze){
                    rigidBody.SetLinearVelocity(glm::vec3(0.0f));
                    rigidBody.SetAngularVelocity(glm::vec3(0.0f));
                }
            }
            else if(updateAll && !rigidBody.m_kinematic)
            {
                rigidBody.m_rigidActor->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
                if(freeze){
                    rigidBody.SetLinearVelocity(glm::vec3(0.0f));
                    rigidBody.SetAngularVelocity(glm::vec3(0.0f));
                }
            }
        }
    }
}

void PhysicsSystem::OnCreate()
{
    auto physics = PhysicsManager::GetInstance().m_physics;
    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.solverType = PxSolverType::eTGS;
    sceneDesc.cpuDispatcher = PhysicsManager::GetInstance().m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_physicsScene = physics->createScene(sceneDesc);

    PxPvdSceneClient *pvdClient = m_physicsScene->getScenePvdClient();
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
    PX_RELEASE(m_physicsScene);
}

void PhysicsSystem::FixedUpdate()
{
    Simulate(Application::Time().TimeStep());
}

void PhysicsSystem::Simulate(float time) const
{
    const std::vector<Entity> *rigidBodyEntities = EntityManager::UnsafeGetPrivateComponentOwnersList<RigidBody>();
    if (!rigidBodyEntities)
        return;
#pragma region Update shape
    if (rigidBodyEntities)
    {
        for (auto entity : *rigidBodyEntities)
        {
            auto &rigidBody = entity.GetPrivateComponent<RigidBody>();
            if (rigidBody.m_currentRegistered == false && entity.IsValid() && entity.IsEnabled() &&
                rigidBody.IsEnabled())
            {
                rigidBody.m_currentRegistered = true;
                m_physicsScene->addActor(*rigidBody.m_rigidActor);
            }
            else if (
                rigidBody.m_currentRegistered == true &&
                (!entity.IsValid() || !entity.IsEnabled() || !rigidBody.IsEnabled()))
            {
                rigidBody.m_currentRegistered = false;
                m_physicsScene->removeActor(*rigidBody.m_rigidActor);
            }
        }
    }
#pragma endregion
#pragma region Simulate
    m_physicsScene->simulate(time);
    m_physicsScene->fetchResults(true);
#pragma endregion
#pragma region Download transforms from physX
    if (rigidBodyEntities)
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
                                          auto &rigidBody = rigidBodyEntity.GetPrivateComponent<RigidBody>();
                                          if (rigidBody.m_currentRegistered && !rigidBody.m_kinematic)
                                          {
                                              PxTransform transform = rigidBody.m_rigidActor->getGlobalPose();
                                              glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                              glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                              glm::vec3 scale =
                                                  rigidBodyEntity.GetDataComponent<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              rigidBodyEntity.SetDataComponent(globalTransform);

                                          }
                                      }
                                      if (reminder > i)
                                      {
                                          size_t index = capacity * threadSize + i;
                                          const auto &rigidBodyEntity = list->at(index);
                                          auto &rigidBody = rigidBodyEntity.GetPrivateComponent<RigidBody>();
                                          if (rigidBody.m_currentRegistered && !rigidBody.m_kinematic)
                                          {
                                              PxTransform transform = rigidBody.m_rigidActor->getGlobalPose();
                                              glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                              glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                              glm::vec3 scale =
                                                  rigidBodyEntity.GetDataComponent<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              rigidBodyEntity.SetDataComponent(globalTransform);
                                          }
                                      }
                                  })
                                  .share());
        }
        for (const auto &i : futures)
            i.wait();
    }
#pragma endregion
#pragma region Recalculate local transforms
    TransformManager::GetInstance().m_physicsSystemOverride = true;
#pragma endregion
}
void PhysicsSystem::OnEnable()
{
    PhysicsManager::UploadTransforms(true);
}
