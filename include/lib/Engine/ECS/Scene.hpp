#pragma once
#include "Entities.hpp"
#include "Entity.hpp"
#include "EntityMetadata.hpp"
#include "IAsset.hpp"
#include "IPrivateComponent.hpp"
#include "ISystem.hpp"
#include "PrivateComponentRef.hpp"
#include "PrivateComponentStorage.hpp"
#include "Utilities.hpp"
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
    float m_environmentGamma = 2.2f;
    float m_ambientLightIntensity = 0.8f;
    EnvironmentType m_environmentType = EnvironmentType::EnvironmentalMap;

    void Serialize(YAML::Emitter &out);
    void Deserialize(const YAML::Node &in);
};

struct SceneDataStorage
{
    std::vector<Entity> m_entities;
    std::vector<EntityMetadata> m_entityMetadataList;
    std::vector<DataComponentStorage> m_dataComponentStorages;
    std::unordered_map<Handle, Entity> m_entityMap;
    PrivateComponentStorage m_entityPrivateComponentStorage;

    void Clone(
        std::unordered_map<Handle, Handle> &entityMap,
        const SceneDataStorage &source,
        const std::shared_ptr<Scene> &newScene);
};

class UNIENGINE_API Scene : public IAsset
{
    friend class Application;
    friend class Entities;
    friend class Editor;
    friend class EditorLayer;
    friend class Serialization;
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

  private:
#pragma region Entity Management
    void DeleteEntityInternal(unsigned entityIndex);

    std::vector<std::reference_wrapper<DataComponentStorage>> QueryDataComponentStorages(unsigned entityQueryIndex);
    std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> GetDataComponentStorage(
        unsigned entityArchetypeIndex);
    template <typename T = IDataComponent>
    void GetDataComponentArrayStorage(const DataComponentStorage &storage, std::vector<T> &container, bool checkEnable);
    void GetEntityStorage(const DataComponentStorage &storage, std::vector<Entity> &container, bool checkEnable);
    size_t SwapEntity(DataComponentStorage &storage, size_t index1, size_t index2);
    void SetEnableSingle(const Entity &entity, const bool &value);
    void SetDataComponent(const unsigned &entityIndex, size_t id, size_t size, IDataComponent *data);
    friend class Serialization;
    IDataComponent *GetDataComponentPointer(const Entity &entity, const size_t &id);
    IDataComponent *GetDataComponentPointer(unsigned entityIndex, const size_t &id);

    void SetPrivateComponent(const Entity &entity, const std::shared_ptr<IPrivateComponent> &ptr);

    void ForEachDescendantHelper(const Entity &target, const std::function<void(const Entity &entity)> &func);
    void GetDescendantsHelper(const Entity &target, std::vector<Entity> &results);

    void RemoveDataComponent(const Entity &entity, const size_t &typeID);
    template <typename T = IDataComponent> T GetDataComponent(const size_t &index);
    template <typename T = IDataComponent> bool HasDataComponent(const size_t &index);
    template <typename T = IDataComponent> void SetDataComponent(const size_t &index, const T &value);

#pragma region ForEach
    template <typename T1 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent,
        typename T8 = IDataComponent>
    void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);

#pragma endregion


#pragma endregion

  protected:
    bool LoadInternal(const std::filesystem::path &path) override;

  public:
    template <typename T = IDataComponent>
    void GetComponentDataArray(const EntityQuery &entityQuery, std::vector<T> &container, bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void GetComponentDataArray(
        const EntityQuery &entityQuery,
        std::vector<T1> &container,
        const std::function<bool(const T2 &)> &filterFunc,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    void GetComponentDataArray(
        const EntityQuery &entityQuery,
        std::vector<T1> &container,
        const std::function<bool(const T2 &, const T3 &)> &filterFunc,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void GetComponentDataArray(
        const EntityQuery &entityQuery, const T1 &filter, std::vector<T2> &container, bool checkEnable = true);
    void GetEntityArray(const EntityQuery &entityQuery, std::vector<Entity> &container, bool checkEnable = true);
    template <typename T1 = IDataComponent>
    void GetEntityArray(
        const EntityQuery &entityQuery,
        std::vector<Entity> &container,
        const std::function<bool(const Entity &, const T1 &)> &filterFunc,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void GetEntityArray(
        const EntityQuery &entityQuery,
        std::vector<Entity> &container,
        const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc,
        bool checkEnable = true);
    template <typename T1 = IDataComponent>
    void GetEntityArray(
        const EntityQuery &entityQuery, const T1 &filter, std::vector<Entity> &container, bool checkEnable = true);
    size_t GetEntityAmount(EntityQuery entityQuery, bool checkEnable = true);

    [[nodiscard]] Handle GetEntityHandle(const Entity &entity);
    template <typename T = ISystem> std::shared_ptr<T> GetOrCreateSystem(float order);
    template <typename T = ISystem> std::shared_ptr<T> GetSystem();
    template <typename T = ISystem> bool HasSystem();
    std::shared_ptr<ISystem> GetOrCreateSystem(const std::string &systemName, float order);

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

#pragma region Entity Management
#pragma region Entity methods
    void RemovePrivateComponent(const Entity &entity, size_t typeId);
    // Enable or Disable an Entity. Note that the disable action will recursively disable the children of current
    // entity.
    void SetEnable(const Entity &entity, const bool &value);
    bool IsEntityValid(const Entity &entity);
    bool IsEntityEnabled(const Entity &entity);
    bool IsEntityRoot(const Entity &entity);
    bool IsEntityStatic(const Entity &entity);
    Entity GetRoot(const Entity &entity);
    std::string GetEntityName(const Entity &entity);
    void SetEntityName(const Entity &entity, const std::string &name);
    void SetEntityStatic(const Entity &entity, bool value);

    void SetParent(const Entity &entity, const Entity &parent, const bool &recalculateTransform = false);
    Entity GetParent(const Entity &entity);
    std::vector<Entity> GetChildren(const Entity &entity);
    Entity GetChild(const Entity &entity, int index);
    size_t GetChildrenAmount(const Entity &entity);
    void ForEachChild(const Entity &entity, const std::function<void(Entity child)> &func);
    void RemoveChild(const Entity &entity, const Entity &parent);
    std::vector<Entity> GetDescendants(const Entity &entity);
    void ForEachDescendant(
        const Entity &target, const std::function<void(const Entity &entity)> &func, const bool &fromRoot = true);

    template <typename T = IDataComponent> void AddDataComponent(const Entity &entity, const T &value);
    template <typename T = IDataComponent> void RemoveDataComponent(const Entity &entity);
    template <typename T = IDataComponent> void SetDataComponent(const Entity &entity, const T &value);
    template <typename T = IDataComponent> T GetDataComponent(const Entity &entity);
    template <typename T = IDataComponent> bool HasDataComponent(const Entity &entity);

    template <typename T = IPrivateComponent> std::weak_ptr<T> GetOrSetPrivateComponent(const Entity &entity);
    std::weak_ptr<IPrivateComponent> GetPrivateComponent(const Entity &entity, const std::string &typeName);

    template <typename T = IPrivateComponent> void RemovePrivateComponent(const Entity &entity);
    template <typename T = IPrivateComponent> bool HasPrivateComponent(const Entity &entity);
    bool HasPrivateComponent(const Entity &entity, const std::string &typeName);
#pragma endregion
#pragma region Entity Management
    Entity CreateEntity(const std::string &name = "New Entity");
    Entity CreateEntity(
        const EntityArchetype &archetype, const std::string &name = "New Entity", const Handle &handle = Handle());
    std::vector<Entity> CreateEntities(
        const EntityArchetype &archetype, const size_t &amount, const std::string &name = "New Entity");
    std::vector<Entity> CreateEntities(const size_t &amount, const std::string &name = "New Entity");
    void DeleteEntity(const Entity &entity);
    Entity GetEntity(const Handle &handle);
    Entity GetEntity(const size_t &index);
    template <typename T> std::vector<Entity> GetPrivateComponentOwnersList(const std::shared_ptr<Scene> &scene);
    void ForEachPrivateComponent(const Entity &entity, const std::function<void(PrivateComponentElement &data)> &func);
    void GetAllEntities(std::vector<Entity> &target);
    void ForAllEntities(const std::function<void(int i, Entity entity)> &func);
#pragma endregion
    std::vector<std::reference_wrapper<DataComponentStorage>> QueryDataComponentStorages(
        const EntityQuery &entityQuery);
    std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> GetDataComponentStorage(
        const EntityArchetype &entityArchetype);

#pragma region Unsafe
    // Unsafe zone, allow directly manipulation of entity data, which may result in data corruption.
    /**
     * \brief Unsafe method, retrieve the internal storage of the entities.
     * \return A pointer to the internal storage for all arrays.
     */
    const std::vector<Entity> &UnsafeGetAllEntities();
    void UnsafeForEachDataComponent(

        const Entity &entity, const std::function<void(const DataComponentType &type, void *data)> &func);
    void UnsafeForEachEntityStorage(

        const std::function<void(int i, const std::string &name, const DataComponentStorage &storage)> &func);

    /**
     * \brief Unsafe method, directly retrieve the pointers and sizes of component data array.
     * \tparam T The type of data
     * \param entityQuery The query to filter the data for targeted entity type.
     * \return If the entity type contains the data, return a list of pointer and size pairs, which the pointer points
     * to the first data instance and the size indicates the amount of data instances.
     */
    template <typename T>
    std::vector<std::pair<T *, size_t>> UnsafeGetDataComponentArray(const EntityQuery &entityQuery);
    template <typename T> const std::vector<Entity> *UnsafeGetPrivateComponentOwnersList();

#pragma region For Each
    template <typename T1 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent,
        typename T8 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);
    // For implicit parallel task dispatching
    template <typename T1 = IDataComponent>
    void ForEach(

        ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &)> &func, bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent,
        typename T8 = IDataComponent>
    void ForEach(

        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);

    // For explicit parallel task dispatching
    template <typename T1 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &)> &func);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(

        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(

        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(

        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(

        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent,
        typename T8 = IDataComponent>
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func);

#pragma endregion
#pragma endregion
#pragma endregion
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
template <typename T> std::shared_ptr<T> Scene::GetOrCreateSystem(float rank)
{
    const auto search = m_indexedSystems.find(typeid(T).hash_code());
    if (search != m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    auto ptr = Serialization::ProduceSerializable<T>();
    auto system = std::dynamic_pointer_cast<ISystem>(ptr);
    system->m_scene = std::dynamic_pointer_cast<Scene>(m_self.lock());
    system->m_handle = Handle();
    system->m_rank = rank;
    m_systems.insert({rank, system});
    m_indexedSystems[typeid(T).hash_code()] = system;
    m_mappedSystems[system->m_handle] = system;
    system->m_started = false;
    system->OnCreate();
    m_saved = false;
    return ptr;
}

#pragma region Entity Management

#pragma region GetSetHas
template <typename T> void Scene::AddDataComponent(const Entity &entity, const T &value)
{
    assert(IsEntityValid(entity));
    const auto id = typeid(T).hash_code();
    auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);

#pragma region Check if componentdata already exists.If yes, go to SetComponentData
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages.at(entityInfo.m_dataComponentStorageIndex);
    auto originalComponentTypes = dataComponentStorage.m_dataComponentTypes;
    const auto chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const auto chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    ComponentDataChunk &chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            UNIENGINE_ERROR("Data Component already exists!");
            return;
        }
    }
    m_saved = false;
#pragma endregion
#pragma region If not exist, we first need to create a new archetype
    EntityArchetypeInfo newArchetypeInfo;
    newArchetypeInfo.m_name = "New archetype";
    newArchetypeInfo.m_dataComponentTypes = originalComponentTypes;
    newArchetypeInfo.m_dataComponentTypes.push_back(Typeof<T>());
    std::sort(
        newArchetypeInfo.m_dataComponentTypes.begin() + 3,
        newArchetypeInfo.m_dataComponentTypes.end(),
        ComponentTypeComparator);
    size_t offset = 0;
    DataComponentType prev = newArchetypeInfo.m_dataComponentTypes[0];
    // Erase duplicates

    std::vector<DataComponentType> copy;
    copy.insert(
        copy.begin(), newArchetypeInfo.m_dataComponentTypes.begin(), newArchetypeInfo.m_dataComponentTypes.end());
    newArchetypeInfo.m_dataComponentTypes.clear();
    for (const auto &i : copy)
    {
        bool found = false;
        for (const auto j : newArchetypeInfo.m_dataComponentTypes)
        {
            if (i == j)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        newArchetypeInfo.m_dataComponentTypes.push_back(i);
    }

    for (auto &i : newArchetypeInfo.m_dataComponentTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    newArchetypeInfo.m_entitySize =
        newArchetypeInfo.m_dataComponentTypes.back().m_offset + newArchetypeInfo.m_dataComponentTypes.back().m_size;
    newArchetypeInfo.m_chunkCapacity = Entities::GetArchetypeChunkSize() / newArchetypeInfo.m_entitySize;
    auto archetype = Entities::CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype.
    Entity newEntity = CreateEntity(archetype);
    auto& originalEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    // Transfer component data
    for (const auto &type : originalComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    SetDataComponent(newEntity, value);
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = originalEntityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = originalEntityInfo.m_chunkArrayIndex;
    originalEntityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    originalEntityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    m_sceneDataStorage.m_dataComponentStorages.at(originalEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[originalEntityInfo.m_chunkArrayIndex] = entity;
    m_sceneDataStorage.m_dataComponentStorages.at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
}

template <typename T> void Scene::RemoveDataComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));
    const auto id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code() || id == typeid(GlobalTransform).hash_code() ||
        id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return;
    }
    auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
#pragma region Check if componentdata already exists.If yes, go to SetComponentData
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    if (dataComponentStorage.m_dataComponentTypes.size() <= 3)
    {
        UNIENGINE_ERROR("Remove Component Data failed: Entity must have at least 1 data component besides 3 basic data "
                        "components!");
        return;
    }
    m_saved = false;
#pragma region Create new archetype
    EntityArchetypeInfo newArchetypeInfo;
    newArchetypeInfo.m_name = "New archetype";
    newArchetypeInfo.m_dataComponentTypes = dataComponentStorage.m_dataComponentTypes;
    bool found = false;
    for (int i = 0; i < newArchetypeInfo.m_dataComponentTypes.size(); i++)
    {
        if (newArchetypeInfo.m_dataComponentTypes[i].m_typeId == id)
        {
            newArchetypeInfo.m_dataComponentTypes.erase(newArchetypeInfo.m_dataComponentTypes.begin() + i);
            found = true;
            break;
        }
    }
    if (!found)
    {
        UNIENGINE_ERROR("Failed to remove component data: Component not found");
        return;
    }
    size_t offset = 0;
    for (auto &i : newArchetypeInfo.m_dataComponentTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    newArchetypeInfo.m_entitySize =
        newArchetypeInfo.m_dataComponentTypes.back().m_offset + newArchetypeInfo.m_dataComponentTypes.back().m_size;
    newArchetypeInfo.m_chunkCapacity = Entities::GetArchetypeChunkSize() / newArchetypeInfo.m_entitySize;
    auto archetype = Entities::CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(archetype);
    auto& originalEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    // Transfer component data
    for (const auto &type : newArchetypeInfo.m_dataComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    T retVal = GetDataComponent<T>(entity);
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = originalEntityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = originalEntityInfo.m_chunkArrayIndex;
    originalEntityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    originalEntityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    m_sceneDataStorage.m_dataComponentStorages.at(originalEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[originalEntityInfo.m_chunkArrayIndex] = entity;
    m_sceneDataStorage.m_dataComponentStorages.at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
    return;
}

template <typename T> void Scene::SetDataComponent(const Entity &entity, const T &value)
{
    assert(IsEntityValid(entity));
    SetDataComponent(entity.m_index, typeid(T).hash_code(), sizeof(T), (IDataComponent *)&value);
}
template <typename T> void Scene::SetDataComponent(const size_t &index, const T &value)
{
    const size_t id = typeid(T).hash_code();
    assert(index < m_sceneDataStorage.m_entityMetadataList.size());
    SetDataComponent(index, id, sizeof(T), (IDataComponent *)&value);
}
template <typename T> T Scene::GetDataComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    const size_t chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const size_t chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    ComponentDataChunk &chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    const size_t id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(chunkPointer * sizeof(Transform)));
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(
            sizeof(Transform) * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(GlobalTransform)));
    }
    if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(
            (sizeof(Transform) + sizeof(GlobalTransform)) * dataComponentStorage.m_chunkCapacity +
            chunkPointer * sizeof(GlobalTransformUpdateFlag)));
    }
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            return chunk.GetData<T>(
                static_cast<size_t>(type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(T)));
        }
    }
    UNIENGINE_LOG("ComponentData doesn't exist");
    return T();
}
template <typename T> bool Scene::HasDataComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));

    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    const size_t id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code())
    {
        return true;
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        return true;
    }
    if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return true;
    }
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            return true;
        }
    }
    return false;
}
template <typename T> T Scene::GetDataComponent(const size_t &index)
{
    if (index > m_sceneDataStorage.m_entityMetadataList.size())
        return T();
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    const size_t chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const size_t chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    ComponentDataChunk &chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    const size_t id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(chunkPointer * sizeof(Transform)));
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(
            sizeof(Transform) * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(GlobalTransform)));
    }
    if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return chunk.GetData<T>(static_cast<size_t>(
            (sizeof(Transform) + sizeof(GlobalTransform)) * dataComponentStorage.m_chunkCapacity +
            chunkPointer * sizeof(GlobalTransformUpdateFlag)));
    }
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            return chunk.GetData<T>(
                static_cast<size_t>(type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(T)));
        }
    }
    UNIENGINE_LOG("ComponentData doesn't exist");
    return T();
}
template <typename T> bool Scene::HasDataComponent(const size_t &index)
{

    if (index > m_sceneDataStorage.m_entityMetadataList.size())
        return false;
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];

    const size_t id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code())
    {
        return true;
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        return true;
    }
    if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return true;
    }
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            return true;
        }
    }
    return false;
}

template <typename T> std::weak_ptr<T> Scene::GetOrSetPrivateComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));

    auto typeName = Serialization::GetSerializableTypeName<T>();
    size_t i = 0;
    auto &elements = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (typeName == element.m_privateComponentData->GetTypeName())
        {
            return std::static_pointer_cast<T>(element.m_privateComponentData);
        }
        i++;
    }
    auto ptr = m_sceneDataStorage.m_entityPrivateComponentStorage.GetOrSetPrivateComponent<T>(entity);
    elements.emplace_back(typeid(T).hash_code(), ptr, entity, std::dynamic_pointer_cast<Scene>(m_self.lock()));
    m_saved = false;
    return std::move(ptr);
}
template <typename T> void Scene::RemovePrivateComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));

    auto &elements = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
    for (auto i = 0; i < elements.size(); i++)
    {
        if (std::dynamic_pointer_cast<T>(elements[i].m_privateComponentData))
        {
            m_sceneDataStorage.m_entityPrivateComponentStorage.RemovePrivateComponent<T>(
                entity, elements[i].m_privateComponentData);
            elements.erase(elements.begin() + i);
            m_saved = false;
            return;
        }
    }
}

template <typename T> bool Scene::HasPrivateComponent(const Entity &entity)
{
    assert(IsEntityValid(entity));

    for (auto &element : m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements)
    {
        if (std::dynamic_pointer_cast<T>(element.m_privateComponentData))
        {
            return true;
        }
    }
    return false;
}

#pragma endregion
#pragma region For Each
template <typename T1>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Scene::ForEach(

    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        ForEachStorage(workers, i.get(), func, checkEnable);
    }
}

template <typename T1>
void Scene::ForEach(

    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &)> &func, bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2>
void Scene::ForEach(

    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &, T2 &)> &func, bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3>
void Scene::ForEach(

    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func, bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4>
void Scene::ForEach(

    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
    bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void Scene::ForEach(

    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
    bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Scene::ForEach(

    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
    bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Scene::ForEach(

    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
    bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Scene::ForEach(

    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
    bool checkEnable)
{
    auto &storages = m_sceneDataStorage.m_dataComponentStorages;
    for (auto i = storages.begin() + 1; i < storages.end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(

    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(

    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(

    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> Scene::CreateParallelTask(

    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [&](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            auto queriedStorages = QueryDataComponentStorages(entityQuery);
            for (const auto i : queriedStorages)
            {
                ForEachStorage(workers, i.get(), func, checkEnable);
            }
        });
    return task;
}
#pragma endregion
template <typename T>
void Scene::GetComponentDataArray(const EntityQuery &entityQuery, std::vector<T> &container, bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        GetDataComponentArrayStorage(i.get(), container, checkEnable);
    }
}

template <typename T1, typename T2>
void Scene::GetComponentDataArray(

    const EntityQuery &entityQuery,
    std::vector<T1> &container,
    const std::function<bool(const T2 &)> &filterFunc,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<T2> componentDataList;
    std::vector<T1> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList, checkEnable);
    GetComponentDataArray(entityQuery, targetDataList, checkEnable);
    if (targetDataList.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList.size();
    std::vector<std::vector<T1>> collectedDataLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T1>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T1> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            Jobs::Workers()
                .Push(
                    [&targetDataList, &componentDataList, size, collectedDataList, i, filterFunc, threadSize](int id) {
                        for (int j = 0; j < size / threadSize; j++)
                        {
                            if (filterFunc(componentDataList[j * threadSize + i]))
                            {
                                collectedDataList->push_back(targetDataList[j * threadSize + i]);
                            }
                        }
                    })
                .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        auto listSize = collectedDataLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedDataLists[i].data(), listSize * sizeof(T1));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filterFunc(componentDataList[size - remainder + i]))
        {
            container.push_back(targetDataList[size - remainder + i]);
        }
    }
}

template <typename T1, typename T2, typename T3>
void Scene::GetComponentDataArray(

    const EntityQuery &entityQuery,
    std::vector<T1> &container,
    const std::function<bool(const T2 &, const T3 &)> &filterFunc,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<T3> componentDataList2;
    std::vector<T2> componentDataList1;
    std::vector<T1> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList2, checkEnable);
    GetComponentDataArray(entityQuery, componentDataList1, checkEnable);
    GetComponentDataArray(entityQuery, targetDataList, checkEnable);
    if (targetDataList.size() != componentDataList1.size() || componentDataList1.size() != componentDataList2.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList1.size();
    std::vector<std::vector<T1>> collectedDataLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T1>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T1> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            Jobs::Workers()
                .Push([&targetDataList,
                       &componentDataList1,
                       &componentDataList2,
                       size,
                       collectedDataList,
                       i,
                       filterFunc,
                       threadSize](int id) {
                    for (int j = 0; j < size / threadSize; j++)
                    {
                        if (filterFunc(componentDataList1[j * threadSize + i], componentDataList2[j * threadSize + i]))
                        {
                            collectedDataList->push_back(targetDataList[j * threadSize + i]);
                        }
                    }
                })
                .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        auto listSize = collectedDataLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedDataLists[i].data(), listSize * sizeof(T1));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filterFunc(componentDataList1[size - remainder + i], componentDataList2[size - remainder + i]))
        {
            container.push_back(targetDataList[size - remainder + i]);
        }
    }
}

template <typename T1, typename T2>
void Scene::GetComponentDataArray(

    const EntityQuery &entityQuery, const T1 &filter, std::vector<T2> &container, bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<T1> componentDataList;
    std::vector<T2> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList, checkEnable);
    GetComponentDataArray(entityQuery, targetDataList, checkEnable);
    if (targetDataList.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList.size();
    std::vector<std::vector<T2>> collectedDataLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T2>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T2> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            Jobs::Workers()
                .Push([&targetDataList, &componentDataList, size, filter, collectedDataList, i, threadSize](int id) {
                    for (int j = 0; j < size / threadSize; j++)
                    {
                        if (filter == componentDataList[j * threadSize + i])
                        {
                            collectedDataList->push_back(targetDataList[j * threadSize + i]);
                        }
                    }
                })
                .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        auto listSize = collectedDataLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedDataLists[i].data(), listSize * sizeof(T2));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filter == componentDataList[size - remainder + i])
        {
            container.push_back(targetDataList[size - remainder + i]);
        }
    }
}

template <typename T1>
void Scene::GetEntityArray(

    const EntityQuery &entityQuery,
    std::vector<Entity> &container,
    const std::function<bool(const Entity &, const T1 &)> &filterFunc,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList;
    GetEntityArray(entityQuery, allEntities, checkEnable);
    GetComponentDataArray(entityQuery, componentDataList, checkEnable);
    if (allEntities.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(
            Jobs::Workers()
                .Push([&allEntities, &componentDataList, size, collectedEntityList, i, filterFunc, threadSize](int id) {
                    for (int j = 0; j < size / threadSize; j++)
                    {
                        if (filterFunc(allEntities[j * threadSize + i], componentDataList[j * threadSize + i]))
                        {
                            collectedEntityList->push_back(allEntities[j * threadSize + i]);
                        }
                    }
                })
                .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        const auto listSize = collectedEntityLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedEntityLists[i].data(), listSize * sizeof(Entity));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filterFunc(allEntities[size - remainder + i], componentDataList[size - remainder + i]))
        {
            container.push_back(allEntities[size - remainder + i]);
        }
    }
}

template <typename T1, typename T2>
void Scene::GetEntityArray(

    const EntityQuery &entityQuery,
    std::vector<Entity> &container,
    const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList1;
    std::vector<T2> componentDataList2;
    GetEntityArray(entityQuery, allEntities, checkEnable);
    GetComponentDataArray(entityQuery, componentDataList1, checkEnable);
    GetComponentDataArray(entityQuery, componentDataList2, checkEnable);
    if (allEntities.size() != componentDataList1.size() || componentDataList1.size() != componentDataList2.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(Jobs::Workers()
                              .Push([=, &allEntities, &componentDataList1, &componentDataList2](int id) {
                                  for (int j = 0; j < size / threadSize; j++)
                                  {
                                      if (filterFunc(
                                              allEntities[j * threadSize + i],
                                              componentDataList1[j * threadSize + i],
                                              componentDataList2[j * threadSize + i]))
                                      {
                                          collectedEntityList->push_back(allEntities[j * threadSize + i]);
                                      }
                                  }
                              })
                              .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        const auto listSize = collectedEntityLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedEntityLists[i].data(), listSize * sizeof(Entity));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filterFunc(
                allEntities[size - remainder + i],
                componentDataList1[size - remainder + i],
                componentDataList2[size - remainder + i]))
        {
            container.push_back(allEntities[size - remainder + i]);
        }
    }
}

template <typename T1>
void Scene::GetEntityArray(

    const EntityQuery &entityQuery, const T1 &filter, std::vector<Entity> &container, bool checkEnable)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList;
    GetEntityArray(entityQuery, allEntities, checkEnable);
    GetComponentDataArray(entityQuery, componentDataList, checkEnable);
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = Jobs::Workers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(
            Jobs::Workers()
                .Push([&allEntities, &componentDataList, size, filter, collectedEntityList, i, threadSize](int id) {
                    for (int j = 0; j < size / threadSize; j++)
                    {
                        if (filter == componentDataList[j * threadSize + i])
                        {
                            collectedEntityList->push_back(allEntities[j * threadSize + i]);
                        }
                    }
                })
                .share());
    }
    for (const auto &i : futures)
        i.wait();
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        const auto listSize = collectedEntityLists[i].size();
        if (listSize == 0)
            continue;
        container.resize(container.size() + listSize);
        memcpy(&container.at(container.size() - listSize), collectedEntityLists[i].data(), listSize * sizeof(Entity));
    }

    const size_t remainder = size % threadSize;
    for (int i = 0; i < remainder; i++)
    {
        if (filter == componentDataList[size - remainder + i])
        {
            container.push_back(allEntities[size - remainder + i]);
        }
    }
}

template <typename T>
std::vector<std::pair<T *, size_t>> Scene::UnsafeGetDataComponentArray(const EntityQuery &entityQuery)
{
    std::vector<std::pair<T *, size_t>> retVal;
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto storage : queriedStorages)
    {
        auto &i = storage.get();
        auto targetType = Typeof<T>();
        const auto entityCount = i.m_entityAliveCount;
        auto found = false;
        for (const auto &type : i.m_dataComponentTypes)
        {
            if (type.m_typeId == targetType.m_typeId)
            {
                targetType = type;
                found = true;
            }
        }
        if (!found)
            continue;
        const auto capacity = i.m_chunkCapacity;
        const auto &chunkArray = i.m_chunkArray;
        const auto chunkSize = entityCount / capacity;
        const auto chunkReminder = entityCount % capacity;
        for (int chunkIndex = 0; chunkIndex < chunkSize; chunkIndex++)
        {
            auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
            T *ptr = reinterpret_cast<T *>(data + targetType.m_offset * capacity);
            retVal.emplace_back(ptr, capacity);
        }
        if (chunkReminder > 0)
        {
            auto *data = static_cast<char *>(chunkArray.m_chunks[chunkSize].m_data);
            T *ptr = reinterpret_cast<T *>(data + targetType.m_offset * capacity);
            retVal.emplace_back(ptr, chunkReminder);
        }
    }
    return retVal;
}

template <typename T> const std::vector<Entity> *Scene::UnsafeGetPrivateComponentOwnersList()
{
    return m_sceneDataStorage.m_entityPrivateComponentStorage.UnsafeGetOwnersList<T>();
}

template <typename T>
void Scene::GetDataComponentArrayStorage(
    const DataComponentStorage &storage, std::vector<T> &container, bool checkEnable)
{
    auto targetType = Typeof<T>();
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType.m_typeId)
        {
            targetType = type;
            size_t amount = storage.m_entityAliveCount;
            if (amount == 0)
                return;
            if (checkEnable)
            {
                auto &workers = Jobs::Workers();
                const auto capacity = storage.m_chunkCapacity;
                const auto &chunkArray = storage.m_chunkArray;
                const auto &entities = chunkArray.m_entities;
                std::vector<std::shared_future<void>> results;
                const auto threadSize = workers.Size();
                const auto threadLoad = amount / threadSize;
                const auto loadReminder = amount % threadSize;
                std::vector<std::vector<T>> tempStorage;
                tempStorage.resize(threadSize);
                for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
                {
                    results.push_back(
                        workers
                            .Push([=, &chunkArray, &entities, &tempStorage](int id) {
                                for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                {
                                    const auto chunkIndex = i / capacity;
                                    const auto remainder = i % capacity;
                                    auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                                    T *address1 = reinterpret_cast<T *>(data + type.m_offset * capacity);
                                    const auto entity = entities.at(i);
                                    if (!m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                                        continue;
                                    tempStorage[threadIndex].push_back(address1[remainder]);
                                }
                                if (threadIndex < loadReminder)
                                {
                                    const int i = threadIndex + threadSize * threadLoad;
                                    const auto chunkIndex = i / capacity;
                                    const auto remainder = i % capacity;
                                    auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                                    T *address1 = reinterpret_cast<T *>(data + type.m_offset * capacity);
                                    const auto entity = entities.at(i);
                                    if (!m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                                        return;
                                    tempStorage[threadIndex].push_back(address1[remainder]);
                                }
                            })
                            .share());
                }
                for (const auto &i : results)
                    i.wait();
                for (auto &i : tempStorage)
                {
                    container.insert(container.end(), i.begin(), i.end());
                }
            }
            else
            {
                container.resize(container.size() + amount);
                const auto capacity = storage.m_chunkCapacity;
                const auto chunkAmount = amount / capacity;
                const auto remainAmount = amount % capacity;
                for (size_t i = 0; i < chunkAmount; i++)
                {
                    memcpy(
                        &container.at(container.size() - remainAmount - capacity * (chunkAmount - i)),
                        reinterpret_cast<void *>(
                            static_cast<char *>(storage.m_chunkArray.m_chunks[i].m_data) +
                            capacity * targetType.m_offset),
                        capacity * targetType.m_size);
                }
                if (remainAmount > 0)
                    memcpy(
                        &container.at(container.size() - remainAmount),
                        reinterpret_cast<void *>(
                            static_cast<char *>(storage.m_chunkArray.m_chunks[chunkAmount].m_data) +
                            capacity * targetType.m_offset),
                        remainAmount * targetType.m_size);
            }
        }
    }
}
#pragma region ForEachStorage
template <typename T1>
void Scene::ForEachStorage(

    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    const auto entityCount = storage.m_entityAliveCount;
    auto found1 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
    }
    if (!found1)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(static_cast<int>(i), entity, address1[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(static_cast<int>(i), entity, address1[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
    }

    if (!found1 || !found2)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(static_cast<int>(i), entity, address1[remainder], address2[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(static_cast<int>(i), entity, address1[remainder], address2[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
    }
    if (!found1 || !found2 || !found3)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i), entity, address1[remainder], address2[remainder], address3[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i), entity, address1[remainder], address2[remainder], address3[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3, typename T4>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    auto targetType4 = Typeof<T4>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
        else if (type.m_typeId == targetType4.m_typeId)
        {
            targetType4 = type;
            found4 = true;
        }
    }
    if (!found1 || !found2 || !found3 || !found4)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder]);
                    }

                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    auto targetType4 = Typeof<T4>();
    auto targetType5 = Typeof<T5>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
        else if (type.m_typeId == targetType4.m_typeId)
        {
            targetType4 = type;
            found4 = true;
        }
        else if (type.m_typeId == targetType5.m_typeId)
        {
            targetType5 = type;
            found5 = true;
        }
    }
    if (!found1 || !found2 || !found3 || !found4 || !found5)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    auto targetType4 = Typeof<T4>();
    auto targetType5 = Typeof<T5>();
    auto targetType6 = Typeof<T6>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    bool found6 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
        else if (type.m_typeId == targetType4.m_typeId)
        {
            targetType4 = type;
            found4 = true;
        }
        else if (type.m_typeId == targetType5.m_typeId)
        {
            targetType5 = type;
            found5 = true;
        }
        else if (type.m_typeId == targetType6.m_typeId)
        {
            targetType6 = type;
            found6 = true;
        }
    }
    if (!found1 || !found2 || !found3 || !found4 || !found5 || !found6)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Scene::ForEachStorage(
    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    auto targetType4 = Typeof<T4>();
    auto targetType5 = Typeof<T5>();
    auto targetType6 = Typeof<T6>();
    auto targetType7 = Typeof<T7>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    bool found6 = false;
    bool found7 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
        else if (type.m_typeId == targetType4.m_typeId)
        {
            targetType4 = type;
            found4 = true;
        }
        else if (type.m_typeId == targetType5.m_typeId)
        {
            targetType5 = type;
            found5 = true;
        }
        else if (type.m_typeId == targetType6.m_typeId)
        {
            targetType6 = type;
            found6 = true;
        }
        else if (type.m_typeId == targetType7.m_typeId)
        {
            targetType7 = type;
            found7 = true;
        }
    }
    if (!found1 || !found2 || !found3 || !found4 || !found5 || !found6 || !found7)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        T7 *address7 = reinterpret_cast<T7 *>(data + targetType7.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder],
                            address7[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        T7 *address7 = reinterpret_cast<T7 *>(data + targetType7.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder],
                            address7[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Scene::ForEachStorage(

    ThreadPool &workers,
    const DataComponentStorage &storage,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
    bool checkEnable)
{
    auto targetType1 = Typeof<T1>();
    auto targetType2 = Typeof<T2>();
    auto targetType3 = Typeof<T3>();
    auto targetType4 = Typeof<T4>();
    auto targetType5 = Typeof<T5>();
    auto targetType6 = Typeof<T6>();
    auto targetType7 = Typeof<T7>();
    auto targetType8 = Typeof<T8>();
    const auto entityCount = storage.m_entityAliveCount;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    bool found6 = false;
    bool found7 = false;
    bool found8 = false;
    for (const auto &type : storage.m_dataComponentTypes)
    {
        if (type.m_typeId == targetType1.m_typeId)
        {
            targetType1 = type;
            found1 = true;
        }
        else if (type.m_typeId == targetType2.m_typeId)
        {
            targetType2 = type;
            found2 = true;
        }
        else if (type.m_typeId == targetType3.m_typeId)
        {
            targetType3 = type;
            found3 = true;
        }
        else if (type.m_typeId == targetType4.m_typeId)
        {
            targetType4 = type;
            found4 = true;
        }
        else if (type.m_typeId == targetType5.m_typeId)
        {
            targetType5 = type;
            found5 = true;
        }
        else if (type.m_typeId == targetType6.m_typeId)
        {
            targetType6 = type;
            found6 = true;
        }
        else if (type.m_typeId == targetType7.m_typeId)
        {
            targetType7 = type;
            found7 = true;
        }
        else if (type.m_typeId == targetType8.m_typeId)
        {
            targetType8 = type;
            found8 = true;
        }
    }
    if (!found1 || !found2 || !found3 || !found4 || !found5 || !found6 || !found7 || !found8)
        return;
    const auto capacity = storage.m_chunkCapacity;
    const auto &chunkArray = storage.m_chunkArray;
    const auto &entities = chunkArray.m_entities;
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = entityCount / threadSize;
    const auto loadReminder = entityCount % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=, &chunkArray, &entities](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        T7 *address7 = reinterpret_cast<T7 *>(data + targetType7.m_offset * capacity);
                        T8 *address8 = reinterpret_cast<T8 *>(data + targetType8.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            continue;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder],
                            address7[remainder],
                            address8[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        T2 *address2 = reinterpret_cast<T2 *>(data + targetType2.m_offset * capacity);
                        T3 *address3 = reinterpret_cast<T3 *>(data + targetType3.m_offset * capacity);
                        T4 *address4 = reinterpret_cast<T4 *>(data + targetType4.m_offset * capacity);
                        T5 *address5 = reinterpret_cast<T5 *>(data + targetType5.m_offset * capacity);
                        T6 *address6 = reinterpret_cast<T6 *>(data + targetType6.m_offset * capacity);
                        T7 *address7 = reinterpret_cast<T7 *>(data + targetType7.m_offset * capacity);
                        T8 *address8 = reinterpret_cast<T8 *>(data + targetType8.m_offset * capacity);
                        const auto entity = entities.at(i);
                        if (checkEnable && !m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                            return;
                        func(
                            static_cast<int>(i),
                            entity,
                            address1[remainder],
                            address2[remainder],
                            address3[remainder],
                            address4[remainder],
                            address5[remainder],
                            address6[remainder],
                            address7[remainder],
                            address8[remainder]);
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
}

#pragma endregion

#pragma endregion

} // namespace UniEngine