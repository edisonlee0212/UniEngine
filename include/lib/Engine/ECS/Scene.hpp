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

struct SceneDataStorage
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

class UNIENGINE_API Scene
{
    friend class Application;
    friend class EntityManager;
    friend class SerializationManager;
    SceneDataStorage m_sceneDataStorage;
    std::multimap<float, std::shared_ptr<ISystem>> m_systems;
    std::map<size_t, std::shared_ptr<ISystem>> m_indexedSystems;
    size_t m_index;
    Bound m_worldBound;
  public:
    std::string m_name = "New Scene";
    void Purge();
    Scene &operator=(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    [[nodiscard]] size_t GetIndex() const;
    Scene(size_t index);
    template <typename T = ISystem> void DestroySystem();
    ~Scene();
    void FixedUpdate();
    void PreUpdate();
    void Update();
    void LateUpdate();
    void OnGui();
};

template <typename T> void Scene::DestroySystem()
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
} // namespace UniEngine