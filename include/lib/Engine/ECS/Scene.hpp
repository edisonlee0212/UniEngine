#pragma once
#include <Entity.hpp>
#include <ISystem.hpp>
#include <PrivateComponentStorage.hpp>
#include <Utilities.hpp>
#include <IAsset.hpp>
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
    std::vector<Entity> m_entities;
    std::vector<EntityInfo> m_entityInfos;
    std::vector<DataComponentStorage> m_dataComponentStorages;
    PrivateComponentStorage m_entityPrivateComponentStorage;
};

class UNIENGINE_API Scene : public IAsset
{
    friend class Application;
    friend class EntityManager;
    friend class SerializationManager;
    SceneDataStorage m_sceneDataStorage;
    std::multimap<float, std::shared_ptr<ISystem>> m_systems;
    std::map<size_t, std::shared_ptr<ISystem>> m_indexedSystems;
    Bound m_worldBound;


    void SerializeDataComponentStorage(const DataComponentStorage& storage, YAML::Emitter &out);
    void SerializeEntityInfo(const EntityInfo& entityInfo, YAML::Emitter &out);
    void SerializeSystem(const std::shared_ptr<ISystem>& system, YAML::Emitter &out);


  public:
    std::string m_name = "New Scene";
    void Purge();
    Scene();
    Scene &operator=(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    [[nodiscard]] size_t GetIndex() const;
    template <typename T = ISystem> void DestroySystem();
    ~Scene();
    void FixedUpdate();
    void PreUpdate();
    void Update();
    void LateUpdate();
    void OnGui();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
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