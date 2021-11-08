#pragma once
#include <Entity.hpp>
#include <EntityMetadata.hpp>
#include <IAsset.hpp>
#include <IPrivateComponent.hpp>
#include <ISystem.hpp>
#include <PrivateComponentStorage.hpp>
#include <Utilities.hpp>
#include <PrivateComponentRef.hpp>
namespace UniEngine
{

enum UNIENGINE_API SystemGroup
{
    PreparationSystemGroup = 0,
    SimulationSystemGroup = 1,
    PresentationSystemGroup = 2
};

enum class UNIENGINE_API EnvironmentType
{
    EnvironmentalMap,
    Color
};

struct UNIENGINE_API EnvironmentSettings
{
    AssetRef m_environmentalMap;
    glm::vec3 m_backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float m_environmentGamma = 1.0f;
    float m_ambientLightIntensity = 0.8f;
    EnvironmentType m_environmentType = EnvironmentType::EnvironmentalMap;

    void Serialize(YAML::Emitter &out);
    void Deserialize(const YAML::Node &in);
};

struct SceneDataStorage
{
    std::vector<Entity> m_entities;
    std::vector<EntityMetadata> m_entityInfos;
    std::vector<DataComponentStorage> m_dataComponentStorages;
    std::unordered_map<Handle, Entity> m_entityMap;
    PrivateComponentStorage m_entityPrivateComponentStorage;

    void Clone(const SceneDataStorage &source, const std::shared_ptr<Scene> &newScene);
};

class UNIENGINE_API Scene : public IAsset
{
    friend class Application;
    friend class EntityManager;
    friend class EditorManager;
    friend class EditorLayer;
    friend class SerializationManager;
    friend class SystemRef;
    friend struct Entity;
    friend class Prefab;
    friend class TransformLayer;
    friend class PrivateComponentStorage;
    SceneDataStorage m_sceneDataStorage;
    std::multimap<float, std::shared_ptr<ISystem>> m_systems;
    std::map<size_t, std::shared_ptr<ISystem>> m_indexedSystems;
    std::map<Handle, std::shared_ptr<ISystem>> m_mappedSystems;
    Bound m_worldBound;
    void SerializeDataComponentStorage(const DataComponentStorage &storage, YAML::Emitter &out);
    void SerializeSystem(const std::shared_ptr<ISystem> &system, YAML::Emitter &out);

  protected:
    bool LoadInternal(const std::filesystem::path &path) override;

  public:
    template <typename T = ISystem>
    std::shared_ptr<T> GetOrCreateSystem(float order);
    template <typename T = ISystem> std::shared_ptr<T> GetSystem();
    template <typename T = ISystem> bool HasSystem();
    std::shared_ptr<ISystem> GetOrCreateSystem(
        const std::string &systemName, float order);

    EnvironmentSettings m_environmentSettings;
    PrivateComponentRef m_mainCamera;
    void Purge();
    void OnCreate() override;
    static void Clone(const std::shared_ptr<Scene> &source, const std::shared_ptr<Scene> &newScene);
    [[nodiscard]] Bound GetBound() const;
    void SetBound(const Bound &value);
    template <typename T = ISystem> void DestroySystem();
    ~Scene();
    void FixedUpdate();
    void Start();
    void Update();
    void LateUpdate();
    void OnInspect() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
template <typename T> std::shared_ptr<T> Scene::GetSystem()
{
    const auto search = m_indexedSystems.find(typeid(T).hash_code());
    if (search != m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
template <typename T> bool Scene::HasSystem()
{
    const auto search = m_indexedSystems.find(typeid(T).hash_code());
    if (search != m_indexedSystems.end())
        return true;
    return false;
}

template <typename T> void Scene::DestroySystem()
{
    auto system = GetSystem<T>();
    if (system != nullptr)
        return;
    m_indexedSystems.erase(typeid(T).hash_code());
    for (auto &i : m_systems)
    {
        if (i.second.get() == system.get())
        {
            m_systems.erase(i.first);
            return;
        }
    }
}
} // namespace UniEngine