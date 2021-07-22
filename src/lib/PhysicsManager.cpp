#include <Application.hpp>
#include <AssetManager.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>
#include <TransformManager.hpp>
using namespace UniEngine;

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
void PhysicsManager::UploadTransform(const GlobalTransform &globalTransform, Articulation &rigidBody)
{
    GlobalTransform ltw;
    ltw.m_value = globalTransform.m_value * rigidBody.m_shapeTransform;
    ltw.SetScale(glm::vec3(1.0f));

    rigidBody.m_root->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&ltw.m_value));
}
void PhysicsManager::PreUpdate()
{
    const bool playing = Application::IsPlaying();
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
                static_cast<PxRigidDynamic *>(rigidBody.m_rigidActor)
                    ->setKinematicTarget(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
            }
            else if(!playing && !rigidBody.m_kinematic)
            {
                rigidBody.m_rigidActor->setGlobalPose(PxTransform(*(PxMat44 *)(void *)&globalTransform.m_value));
            }
        }
    }
    if (const std::vector<Entity> *entities = EntityManager::UnsafeGetPrivateComponentOwnersList<Articulation>();
        entities != nullptr)
    {
        for (auto entity : *entities)
        {
            auto &articulation = entity.GetPrivateComponent<Articulation>();
            auto globalTransform = entity.GetDataComponent<GlobalTransform>();
            if(!playing) UploadTransform(globalTransform, articulation);
        }
    }
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
    physicsManager.m_defaultMaterial = AssetManager::CreateResource<PhysicsMaterial>();
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
    const std::vector<Entity> *articulationEntities =
        EntityManager::UnsafeGetPrivateComponentOwnersList<Articulation>();
    if (!rigidBodyEntities && !articulationEntities)
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
    if (articulationEntities)
    {
        for (auto entity : *articulationEntities)
        {
            auto &articulation = entity.GetPrivateComponent<Articulation>();
            if (articulation.m_currentRegistered == false && entity.IsValid() && entity.IsEnabled() &&
                articulation.IsEnabled())
            {
                articulation.m_currentRegistered = true;
                m_physicsScene->addArticulation(*articulation.m_articulation);
            }
            else if (
                articulation.m_currentRegistered == true &&
                (!entity.IsValid() || !entity.IsEnabled() || !articulation.IsEnabled()))
            {
                articulation.m_currentRegistered = false;
                m_physicsScene->removeArticulation(*articulation.m_articulation);
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
    if (articulationEntities)
    {
        std::vector<std::shared_future<void>> futures;
        auto &list = articulationEntities;
        auto threadSize = JobManager::PrimaryWorkers().Size();
        size_t capacity = articulationEntities->size() / threadSize;
        size_t reminder = articulationEntities->size() % threadSize;
        for (size_t i = 0; i < threadSize; i++)
        {
            futures.push_back(JobManager::PrimaryWorkers()
                                  .Push([&list, i, capacity, reminder, threadSize](int id) {
                                      for (size_t j = 0; j < capacity; j++)
                                      {
                                          size_t index = capacity * i + j;
                                          const auto &articulationEntity = list->at(index);
                                          auto &articulation = articulationEntity.GetPrivateComponent<Articulation>();
                                          if (articulation.IsEnabled())
                                          {
                                              PxTransform transform = articulation.m_root->getGlobalPose();
                                              glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                              glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                              glm::vec3 scale =
                                                  articulationEntity.GetDataComponent<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              articulationEntity.SetDataComponent(globalTransform);
                                          }
                                      }
                                      if (reminder > i)
                                      {
                                          size_t index = capacity * threadSize + i;
                                          const auto &articulationEntity = list->at(index);
                                          auto &articulation = articulationEntity.GetPrivateComponent<Articulation>();
                                          if (articulation.IsEnabled())
                                          {
                                              PxTransform transform = articulation.m_root->getGlobalPose();
                                              glm::vec3 position = *(glm::vec3 *)(void *)&transform.p;
                                              glm::quat rotation = *(glm::quat *)(void *)&transform.q;
                                              glm::vec3 scale =
                                                  articulationEntity.GetDataComponent<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              articulationEntity.SetDataComponent(globalTransform);
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
