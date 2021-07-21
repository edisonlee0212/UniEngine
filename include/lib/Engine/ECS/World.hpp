#pragma once
#include <Entity.hpp>
#include <ISystem.hpp>
#include <PrivateComponentStorage.hpp>
#include <Utilities.hpp>
namespace UniEngine
{

enum UNIENGINE_API SystemGroup
{
    PreparationSystemGroup = 0,
    SimulationSystemGroup = 1,
    PresentationSystemGroup = 2
};

struct WorldEntityStorage
{
    size_t m_parentHierarchyVersion = 0;
    std::vector<Entity> m_entities;
    std::vector<EntityInfo> m_entityInfos;
    std::vector<DataComponentStorage> m_entityComponentStorage;
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
    std::multimap<float, std::shared_ptr<ISystem>> m_systems;
    std::map<size_t, std::shared_ptr<ISystem>> m_indexedSystems;
    size_t m_index;
    Bound m_worldBound;

  public:
    void Purge();
    World &operator=(World &&) = delete;
    World &operator=(const World &) = delete;
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    [[nodiscard]] size_t GetIndex() const;
    World(size_t index);
    template <class T = ISystem> std::shared_ptr<T> CreateSystem(const std::string &name, const float &order);
    template <class T = ISystem> void DestroySystem();
    template <class T = ISystem> std::shared_ptr<T> GetSystem();
    ~World();
    void FixedUpdate();
    void PreUpdate();
    void Update();
    void LateUpdate();
    void OnGui();
};

template <class T> std::shared_ptr<T> World::CreateSystem(const std::string &name, const float &order)
{
    auto system = GetSystem<T>();
    if (system != nullptr)
        return system;
    system = std::make_shared<T>();
    system->m_world = this;
    system->m_name = name;
    m_systems.insert({order, system});
    m_indexedSystems[typeid(T).hash_code()] = system;
    system->OnCreate();
    return system;
}
template <class T> void World::DestroySystem()
{
    auto system = GetSystem<T>();
    if (system != nullptr)
        return;
    m_indexedSystems.erase(typeid(T).hash_code());
    for (auto &i : m_systems)
    {
        if (i.second.get() == system.get()){
            m_systems.erase(i.first);
            return;
        }
    }
}
template <class T> std::shared_ptr<T> World::GetSystem()
{
    const auto search = m_indexedSystems.find(typeid(T).hash_code());
    if (search != m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
} // namespace UniEngine