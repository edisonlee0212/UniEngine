#pragma once
#include <Entity.hpp>
#include <ISystem.hpp>
#include <PrivateComponentStorage.hpp>
#include <Utilities.hpp>
#include <IAsset.hpp>
#include <EntityMetadata.hpp>
#include <IPrivateComponent.hpp>
namespace UniEngine
{

enum UNIENGINE_API SystemGroup
{
    PreparationSystemGroup = 0,
    SimulationSystemGroup = 1,
    PresentationSystemGroup = 2
};
struct EnvironmentalMapSettingsBlock
{
    glm::vec4 m_backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_environmentalMapGamma = 1.0f;
    float m_environmentalLightingIntensity = 1.0f;
    float m_environmentalPadding1 = 0.0f;
    float m_environmentalPadding2 = 0.0f;
};
struct SceneDataStorage
{
    std::vector<Entity> m_entities;
    std::vector<EntityMetadata> m_entityInfos;
    std::vector<DataComponentStorage> m_dataComponentStorages;
    std::unordered_map<Handle, Entity> m_entityMap;
    PrivateComponentStorage m_entityPrivateComponentStorage;
};

class UNIENGINE_API Scene : public IAsset
{
    friend class Application;
    friend class EntityManager;
    friend class EditorManager;
    friend class SerializationManager;
    friend class SystemRef;
    SceneDataStorage m_sceneDataStorage;
    std::multimap<float, std::shared_ptr<ISystem>> m_systems;
    std::map<size_t, std::shared_ptr<ISystem>> m_indexedSystems;
    std::map<Handle, std::shared_ptr<ISystem>> m_mappedSystems;
    Bound m_worldBound;
    void SerializeDataComponentStorage(const DataComponentStorage& storage, YAML::Emitter &out);
    void SerializeSystem(const std::shared_ptr<ISystem>& system, YAML::Emitter &out);

  protected:
    bool LoadInternal(const std::filesystem::path &path) override;

  public:
    AssetRef m_environmentalMap;
    EnvironmentalMapSettingsBlock m_environmentalMapSettings;
    void Purge();
    void OnCreate() override;
    Scene &operator=(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    template <typename T = ISystem> void DestroySystem();
    ~Scene();
    void FixedUpdate();
    void PreUpdate();
    void Update();
    void LateUpdate();
    void OnInspect() override;

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