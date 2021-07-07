#pragma once
#include <Entity.hpp>
#include <PrivateComponentStorage.hpp>
#include <SystemBase.hpp>
#include <Utilities.hpp>
namespace UniEngine
{

enum class UNIENGINE_API SystemGroup
{
    PreparationSystemGroup,
    SimulationSystemGroup,
    PresentationSystemGroup
};

struct WorldEntityStorage
{
    size_t m_parentHierarchyVersion = 0;
    std::vector<Entity> m_entities;
    std::vector<EntityInfo> m_entityInfos;
    std::vector<EntityComponentDataStorage> m_entityComponentStorage;
    PrivateComponentStorage m_entityPrivateComponentStorage;
    std::vector<EntityQuery> m_entityQueries;
    std::vector<EntityQueryInfo> m_entityQueryInfos;
    std::queue<EntityQuery> m_entityQueryPools;
};

class UNIENGINE_API World
{
    friend class Application;
    friend class EntityManager;
    friend class SerializationManager;
    WorldEntityStorage m_worldEntityStorage;
    std::vector<SystemBase *> m_preparationSystems;
    std::vector<SystemBase *> m_simulationSystems;
    std::vector<SystemBase *> m_presentationSystems;
    std::vector<std::function<void()>> m_externalFixedUpdateFunctions;
    size_t m_index;
    Bound m_worldBound;
  public:
    void Purge();
    World &operator=(World &&) = delete;
    World &operator=(const World &) = delete;
    void RegisterFixedUpdateFunction(const std::function<void()> &func);
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    [[nodiscard]] size_t GetIndex() const;
    World(size_t index);
    template <class T = SystemBase> T *CreateSystem(SystemGroup group);
    template <class T = SystemBase> void DestroySystem();
    template <class T = SystemBase> T *GetSystem();
    ~World();
    void PreUpdate();
    void Update();
    void LateUpdate();
};

template <class T> T *World::CreateSystem(SystemGroup group)
{
    T *system = GetSystem<T>();
    if (system != nullptr)
    {
        return system;
    }
    system = new T();
    system->m_world = this;
    switch (group)
    {
    case SystemGroup::PreparationSystemGroup:
        m_preparationSystems.push_back(static_cast<SystemBase *>(system));
        break;
    case SystemGroup::SimulationSystemGroup:
        m_simulationSystems.push_back(static_cast<SystemBase *>(system));
        break;
    case SystemGroup::PresentationSystemGroup:
        m_presentationSystems.push_back(static_cast<SystemBase *>(system));
        break;
    default:
        break;
    }
    system->OnCreate();
    return system;
}
template <class T> void World::DestroySystem()
{
    T *system = GetSystem<T>();
    if (system != nullptr)
    {
        system->OnDestroy();
        delete system;
    }
}
template <class T> T *World::GetSystem()
{
    for (auto i : m_preparationSystems)
    {
        if (dynamic_cast<T *>(i) != nullptr)
        {
            return dynamic_cast<T *>(i);
        }
    }
    for (auto i : m_simulationSystems)
    {
        if (dynamic_cast<T *>(i) != nullptr)
        {
            return dynamic_cast<T *>(i);
        }
    }
    for (auto i : m_presentationSystems)
    {
        if (dynamic_cast<T *>(i) != nullptr)
        {
            return dynamic_cast<T *>(i);
        }
    }
    return nullptr;
}
} // namespace UniEngine