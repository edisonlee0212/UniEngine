#include <PhysicsManager.hpp>
using namespace UniEngine;

void PhysicsSimulationManager::Init()
{
    GetInstance().m_physicsFoundation =
        PxCreateFoundation(PX_PHYSICS_VERSION, GetInstance().m_allocator, GetInstance().m_errorCallback);

    GetInstance().m_physVisDebugger = PxCreatePvd(*GetInstance().m_physicsFoundation);
    PxPvdTransport *transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    GetInstance().m_physVisDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);

    GetInstance().m_physics = PxCreatePhysics(
        PX_PHYSICS_VERSION,
        *GetInstance().m_physicsFoundation,
        PxTolerancesScale(),
        true,
        GetInstance().m_physVisDebugger);

    PxSceneDesc sceneDesc(GetInstance().m_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    GetInstance().m_dispatcher = PxDefaultCpuDispatcherCreate(JobManager::PrimaryWorkers().Size());
    sceneDesc.cpuDispatcher = GetInstance().m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    GetInstance().m_physicsScene = GetInstance().m_physics->createScene(sceneDesc);

    PxPvdSceneClient *pvdClient = GetInstance().m_physicsScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    GetInstance().m_defaultMaterial = GetInstance().m_physics->createMaterial(0.5f, 0.5f, 0.6f);
    GetInstance().m_enabled = true;
}

void PhysicsSimulationManager::Destroy()
{
    PX_RELEASE(GetInstance().m_physicsScene);
    PX_RELEASE(GetInstance().m_dispatcher);
    PX_RELEASE(GetInstance().m_physics);
    if (GetInstance().m_physVisDebugger)
    {
        PxPvdTransport *transport = GetInstance().m_physVisDebugger->getTransport();
        GetInstance().m_physVisDebugger->release();
        GetInstance().m_physVisDebugger = nullptr;
        PX_RELEASE(transport);
    }
    PX_RELEASE(GetInstance().m_physicsFoundation);
}

void PhysicsSimulationManager::UploadTransforms()
{
#pragma region Send transform to physX
    const std::vector<Entity> *rigidBodyEntities = EntityManager::GetPrivateComponentOwnersList<RigidBody>();
    if (rigidBodyEntities != nullptr)
    {
        for (auto rigidBodyEntity : *rigidBodyEntities)
        {
            auto &rigidBody = rigidBodyEntity.GetPrivateComponent<RigidBody>();
            if (rigidBody->IsEnabled())
            {
                auto temp = rigidBodyEntity.GetComponentData<GlobalTransform>();
                temp.SetScale(glm::vec3(1.0f));
                auto ltw = temp.m_value * rigidBody->m_shapeTransform;
                PxMat44 matrix = *(PxMat44 *)(void *)&ltw;
                rigidBody->m_rigidBody->setGlobalPose(PxTransform(matrix));
            }
        }
    }
#pragma endregion
}

void PhysicsSimulationManager::Simulate(float time)
{
    const std::vector<Entity> *rigidBodyEntities = EntityManager::GetPrivateComponentOwnersList<RigidBody>();
#pragma region Get transforms from physX
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
                                  .Push([&list, i, capacity](int id) {
                                      for (size_t j = 0; j < capacity; j++)
                                      {
                                          size_t index = capacity * i + j;
                                          const auto &rigidBodyEntity = list->at(index);
                                          auto &rigidBody = rigidBodyEntity.GetPrivateComponent<RigidBody>();
                                          if (rigidBody->IsEnabled())
                                          {
                                              PxTransform transform = rigidBody->m_rigidBody->getGlobalPose();
                                              auto matrix = PxMat44(transform);
                                              GlobalTransform temp = *(GlobalTransform *)(void *)&matrix;
                                              temp.m_value *= glm::inverse(rigidBody->m_shapeTransform);
                                              glm::vec3 scale;
                                              glm::vec3 pos;
                                              glm::quat rot;
                                              temp.Decompose(pos, rot, scale);
                                              scale = rigidBodyEntity.GetComponentData<GlobalTransform>().GetScale();
                                              temp.SetValue(pos, rot, scale);
                                              Transform ltp;
                                              auto parentEntity = EntityManager::GetParent(rigidBodyEntity);
                                              const glm::mat4 pltw =
                                                  parentEntity.IsNull()
                                                      ? GlobalTransform().m_value
                                                      : parentEntity.GetComponentData<GlobalTransform>().m_value;
                                              ltp.m_value = glm::inverse(pltw) * temp.m_value;
                                              rigidBodyEntity.SetComponentData(ltp);
                                          }
                                      }
                                  })
                                  .share());
        }
        for (size_t i = 0; i < reminder; i++)
        {
            size_t index = capacity * threadSize + i;
            const auto &rigidBodyEntity = list->at(index);
            auto &rigidBody = rigidBodyEntity.GetPrivateComponent<RigidBody>();
            if (rigidBody->IsEnabled())
            {
                PxTransform transform = rigidBody->m_rigidBody->getGlobalPose();
                auto matrix = PxMat44(transform);
                GlobalTransform temp = *(GlobalTransform *)(void *)&matrix;
                temp.m_value *= glm::inverse(rigidBody->m_shapeTransform);
                glm::vec3 scale;
                glm::vec3 pos;
                glm::quat rot;
                temp.Decompose(pos, rot, scale);
                scale = rigidBodyEntity.GetComponentData<GlobalTransform>().GetScale();
                temp.SetValue(pos, rot, scale);
                Transform ltp;
                auto parentEntity = EntityManager::GetParent(rigidBodyEntity);
                const glm::mat4 pltw = parentEntity.IsNull() ? GlobalTransform().m_value
                                                             : parentEntity.GetComponentData<GlobalTransform>().m_value;
                ltp.m_value = glm::inverse(pltw) * temp.m_value;
                rigidBodyEntity.SetComponentData(ltp);
            }
        }
        for (const auto &i : futures)
            i.wait();
    }
#pragma endregion

    GetInstance().m_physicsScene->simulate(time);
    GetInstance().m_physicsScene->fetchResults(true);
}
