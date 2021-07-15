#include <Application.hpp>
#include <PhysicsManager.hpp>
#include <ResourceManager.hpp>
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
            if(!playing) UpdateShape(rigidBody);
            auto globalTransform = entity.GetComponentData<GlobalTransform>();
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
            if(!playing) UpdateShape(articulation);
            auto globalTransform = entity.GetComponentData<GlobalTransform>();
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

    ResourceManager::RegisterResourceType<PhysicsMaterial>("PhysicsMaterial");

    physicsManager.m_defaultMaterial = ResourceManager::CreateResource<PhysicsMaterial>();
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

void PhysicsManager::UpdateShape(RigidBody &rigidBody)
{
    if (rigidBody.m_shapeUpdated)
        return;
    if (rigidBody.m_shape != nullptr)
    {
        rigidBody.m_rigidActor->detachShape(*rigidBody.m_shape);
        rigidBody.m_shape->release();
    }

    switch (rigidBody.m_shapeType)
    {
    case ShapeType::Sphere:
        rigidBody.m_shape = GetInstance().m_physics->createShape(
            PxSphereGeometry(rigidBody.m_shapeParam.x), *rigidBody.m_material->m_value);
        break;
    case ShapeType::Box:
        rigidBody.m_shape = GetInstance().m_physics->createShape(
            PxBoxGeometry(rigidBody.m_shapeParam.x, rigidBody.m_shapeParam.y, rigidBody.m_shapeParam.z),
            *rigidBody.m_material->m_value);
        break;
    case ShapeType::Capsule:
        rigidBody.m_shape = GetInstance().m_physics->createShape(
            PxCapsuleGeometry(rigidBody.m_shapeParam.x, rigidBody.m_shapeParam.y), *rigidBody.m_material->m_value);
        break;
    }
    rigidBody.m_rigidActor->attachShape(*rigidBody.m_shape);
    if (!rigidBody.m_static)
        PxRigidBodyExt::updateMassAndInertia(
            *reinterpret_cast<PxRigidDynamic *>(rigidBody.m_rigidActor), rigidBody.m_density);

    rigidBody.m_shapeUpdated = true;
}

void PhysicsManager::UpdateShape(Articulation &articulation)
{
    if (articulation.m_shapeUpdated)
        return;
    if (articulation.m_shape != nullptr)
    {
        articulation.m_root->detachShape(*articulation.m_shape);
        articulation.m_shape->release();
    }

    switch (articulation.m_shapeType)
    {
    case ShapeType::Sphere:
        articulation.m_shape = GetInstance().m_physics->createShape(
            PxSphereGeometry(articulation.m_shapeParam.x), *articulation.m_material->m_value);
        break;
    case ShapeType::Box:
        articulation.m_shape = GetInstance().m_physics->createShape(
            PxBoxGeometry(articulation.m_shapeParam.x, articulation.m_shapeParam.y, articulation.m_shapeParam.z),
            *articulation.m_material->m_value);
        break;
    case ShapeType::Capsule:
        articulation.m_shape = GetInstance().m_physics->createShape(
            PxCapsuleGeometry(articulation.m_shapeParam.x, articulation.m_shapeParam.y),
            *articulation.m_material->m_value);
        break;
    }
    articulation.m_root->attachShape(*articulation.m_shape);

    PxRigidBodyExt::updateMassAndInertia(
        *reinterpret_cast<PxRigidDynamic *>(articulation.m_root), articulation.m_density);

    articulation.m_shapeUpdated = true;
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

    m_enabled = true;
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
                                                  rigidBodyEntity.GetComponentData<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              rigidBodyEntity.SetComponentData(globalTransform);
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
                                                  rigidBodyEntity.GetComponentData<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              rigidBodyEntity.SetComponentData(globalTransform);
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
                                                  articulationEntity.GetComponentData<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              articulationEntity.SetComponentData(globalTransform);
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
                                                  articulationEntity.GetComponentData<GlobalTransform>().GetScale();
                                              GlobalTransform globalTransform;
                                              globalTransform.SetValue(position, rotation, scale);
                                              articulationEntity.SetComponentData(globalTransform);
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
