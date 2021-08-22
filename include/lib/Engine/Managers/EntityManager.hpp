#pragma once
#include <ISerializable.hpp>
#include <Debug.hpp>
#include <Entity.hpp>
#include <EntityMetadata.hpp>
#include <IPrivateComponent.hpp>
#include <ISingleton.hpp>
#include <JobManager.hpp>
#include <Scene.hpp>
#include <SerializationManager.hpp>
#include <Transform.hpp>

namespace UniEngine
{
template <typename T> DataComponentType Typeof()
{
    DataComponentType type;
    type.m_name = SerializationManager::GetDataComponentTypeName<T>();
    type.m_size = sizeof(T);
    type.m_offset = 0;
    type.m_typeId = typeid(T).hash_code();
    return type;
}
inline UNIENGINE_API bool ComponentTypeComparator(const DataComponentType &a, const DataComponentType &b)
{
    return a.m_typeId < b.m_typeId;
}
#pragma region EntityManager

class UNIENGINE_API EntityManager final : ISingleton<EntityManager>
{
    friend class PhysicsSystem;
    friend class PrefabHolder;
    friend class PrivateComponentStorage;
    friend class TransformManager;
    friend class EditorManager;
    friend class Scene;
    friend class SerializationManager;
    friend struct EntityArchetype;
    friend struct EntityQuery;
    friend struct Entity;
    friend struct Application;
    friend struct AssetManager;
    friend struct PrivateComponentRef;
    friend class Prefab;
    size_t m_archetypeChunkSize = ARCHETYPE_CHUNK_SIZE;
    EntityArchetype m_basicArchetype = EntityArchetype();

    std::vector<EntityArchetypeInfo> m_entityArchetypeInfos;
    std::vector<EntityQueryInfo> m_entityQueryInfos;
#pragma region Data Storage
    std::shared_ptr<Scene> m_scene;

    SceneDataStorage *m_currentAttachedWorldEntityStorage = nullptr;
    std::vector<Entity> *m_entities = nullptr;
    std::vector<EntityMetadata> *m_entityMetaDataCollection = nullptr;
    std::vector<DataComponentStorage> *m_entityDataComponentStorage = nullptr;
    PrivateComponentStorage *m_entityPrivateComponentStorage = nullptr;
#pragma endregion
#pragma region Helpers
    template <typename T = IDataComponent> static bool CheckDataComponentTypes(T arg);
    template <typename T = IDataComponent, typename... Ts> static bool CheckDataComponentTypes(T arg, Ts... args);
    template <typename T = IDataComponent>
    static size_t CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg);
    template <typename T = IDataComponent, typename... Ts>
    static size_t CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts>
    static std::vector<DataComponentType> CollectDataComponentTypes(T arg, Ts... args);
    static void DeleteEntityInternal(const Entity &entity);

    static void RefreshAllEntityQueryInfos();
    static void RefreshEntityQueryInfo(const size_t &index);
    static void RefreshAllEntityArchetypeInfos();
    static void RefreshEntityArchetypeInfo(const size_t &index);

    static void EraseDuplicates(std::vector<DataComponentType> &types);
    template <typename T = IDataComponent>
    static void GetDataComponentArrayStorage(const DataComponentStorage &storage, std::vector<T> &container);
    static void GetEntityStorage(const DataComponentStorage &storage, std::vector<Entity> &container);
    static size_t SwapEntity(DataComponentStorage &storage, size_t index1, size_t index2);
    static void SetEnableSingle(const Entity &entity, const bool &value);
    static void SetDataComponent(const unsigned &entityIndex, size_t id, size_t size, IDataComponent *data);
    friend class SerializationManager;
    static IDataComponent *GetDataComponentPointer(const Entity &entity, const size_t &id);
    static EntityArchetype CreateEntityArchetype(const std::string &name, const std::vector<DataComponentType> &types);

    static void SetPrivateComponent(const Entity &entity, std::shared_ptr<IPrivateComponent> ptr);

    static void ForEachDescendantHelper(const Entity &target, const std::function<void(const Entity &entity)> &func);
    static void GetDescendantsHelper(const Entity &target, std::vector<Entity> &results);

    static void RemoveDataComponent(const Entity &entity, const size_t &typeID);
    template <typename T = IDataComponent> static T GetDataComponent(const size_t &index);
    template <typename T = IDataComponent> static bool HasDataComponent(const size_t &index);
    template <typename T = IDataComponent> static void SetDataComponent(const size_t &index, const T &value);
    static void RemovePrivateComponent(const Entity &entity, size_t typeId);
#pragma endregion
#pragma region ForEach
    template <typename T1 = IDataComponent>
    static void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    static void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    static void ForEachStorage(
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
    static void ForEachStorage(
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
    static void ForEachStorage(
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
    static void ForEachStorage(
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
    static void ForEachStorage(
        ThreadPool &workers,
        const DataComponentStorage &storage,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);

#pragma endregion
#pragma region Entity methods
    // Enable or Disable an Entity. Note that the disable action will recursively disable the children of current
    // entity.
    static void SetEnable(const Entity &entity, const bool &value);

    static Entity GetRoot(const Entity &entity);
    static std::string GetEntityName(const Entity &entity);
    static void SetEntityName(const Entity &entity, const std::string &name);
    static void SetParent(const Entity &entity, const Entity &parent, const bool &recalculateTransform);
    static Entity GetParent(const Entity &entity);
    static std::vector<Entity> GetChildren(const Entity &entity);
    static size_t GetChildrenAmount(const Entity &entity);
    static void ForEachChild(const Entity &entity, const std::function<void(Entity child)> &func);
    static void RemoveChild(const Entity &entity, const Entity &parent);
    static std::vector<Entity> GetDescendants(const Entity &entity);
    static void ForEachDescendant(
        const Entity &target, const std::function<void(const Entity &entity)> &func, const bool &fromRoot = true);

    template <typename T = IDataComponent> static void AddDataComponent(const Entity &entity, const T &value);
    template <typename T = IDataComponent> static void RemoveDataComponent(const Entity &entity);
    template <typename T = IDataComponent> static void SetDataComponent(const Entity &entity, const T &value);
    template <typename T = IDataComponent> static T GetDataComponent(const Entity &entity);
    template <typename T = IDataComponent> static bool HasDataComponent(const Entity &entity);

    template <typename T = IPrivateComponent> static std::weak_ptr<T> GetOrSetPrivateComponent(const Entity &entity);
    static std::weak_ptr<IPrivateComponent> GetPrivateComponent(const Entity &entity, const std::string& typeName);

    template <typename T = IPrivateComponent> static void RemovePrivateComponent(const Entity &entity);
    template <typename T = IPrivateComponent> static bool HasPrivateComponent(const Entity &entity);
    static bool HasPrivateComponent(const Entity &entity, const std::string &typeName);
#pragma endregion
#pragma region EntityArchetype Methods
    static std::string GetEntityArchetypeName(const EntityArchetype &entityArchetype);
    static void SetEntityArchetypeName(const EntityArchetype &entityArchetype, const std::string &name);
#pragma endregion
#pragma region EntityQuery Methods
    template <typename T = IDataComponent, typename... Ts>
    static void SetEntityQueryAllFilters(const EntityQuery &entityQuery, T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts>
    static void SetEntityQueryAnyFilters(const EntityQuery &entityQuery, T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts>
    static void SetEntityQueryNoneFilters(const EntityQuery &entityQuery, T arg, Ts... args);
    template <typename T = IDataComponent>
    static void GetComponentDataArray(const EntityQuery &entityQuery, std::vector<T> &container);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void GetComponentDataArray(
        const EntityQuery &entityQuery, std::vector<T1> &container, const std::function<bool(const T2 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    static void GetComponentDataArray(
        const EntityQuery &entityQuery,
        std::vector<T1> &container,
        const std::function<bool(const T2 &, const T3 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void GetComponentDataArray(const EntityQuery &entityQuery, const T1 &filter, std::vector<T2> &container);
    static void GetEntityArray(const EntityQuery &entityQuery, std::vector<Entity> &container);
    template <typename T1 = IDataComponent>
    static void GetEntityArray(
        const EntityQuery &entityQuery,
        std::vector<Entity> &container,
        const std::function<bool(const Entity &, const T1 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void GetEntityArray(
        const EntityQuery &entityQuery,
        std::vector<Entity> &container,
        const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc);
    template <typename T1 = IDataComponent>
    static void GetEntityArray(const EntityQuery &entityQuery, const T1 &filter, std::vector<Entity> &container);
    static size_t GetEntityAmount(EntityQuery entityQuery);
#pragma endregion
  public:
    static Entity GetEntity(const Handle &handle);

    static EntityArchetype GetDefaultEntityArchetype();

#pragma region Unsafe
    // Unsafe zone, allow directly manipulation of entity data, which may result in data corruption.
    /**
     * \brief Unsafe method, retrieve the internal storage of the entities.
     * \return A pointer to the internal storage for all arrays.
     */
    static std::vector<Entity> *UnsafeGetAllEntities();

    static std::vector<DataComponentStorage *> UnsafeGetDataComponentStorage(const EntityQuery &entityQuery);
    static void UnsafeForEachDataComponent(
        const Entity &entity, const std::function<void(const DataComponentType &type, void *data)> &func);
    static void UnsafeForEachEntityStorage(
        const std::function<void(int i, const std::string &name, const DataComponentStorage &storage)> &func);

    /**
     * \brief Unsafe method, directly retrieve the pointers and sizes of component data array.
     * \tparam T The type of data
     * \param entityQuery The query to filter the data for targeted entity type.
     * \return If the entity type contains the data, return a list of pointer and size pairs, which the pointer points
     * to the first data instance and the size indicates the amount of data instances.
     */
    template <typename T>
    static std::vector<std::pair<T *, size_t>> UnsafeGetDataComponentArray(const EntityQuery &entityQuery);
    template <typename T> static const std::vector<Entity> *UnsafeGetPrivateComponentOwnersList();
#pragma endregion
    static size_t GetArchetypeChunkSize();
    static EntityArchetypeInfo GetArchetypeInfo(const EntityArchetype &entityArchetype);
    static Entity GetEntity(const size_t &index);
    template <typename T> static const std::vector<Entity> GetPrivateComponentOwnersList();
    static void ForEachPrivateComponent(
        const Entity &entity, const std::function<void(PrivateComponentElement &data)> &func);
    static void GetAllEntities(std::vector<Entity> &target);
    static void Detach();
    static void Attach(const std::shared_ptr<Scene> &scene);
    static EntityArchetype CreateEntityArchetypeHelper(const EntityArchetypeInfo &info);

    template <typename T = IDataComponent, typename... Ts>
    static EntityArchetype CreateEntityArchetype(const std::string &name, T arg, Ts... args);
    static Entity CreateEntity(const std::string &name = "New Entity");
    static Entity CreateEntity(const EntityArchetype &archetype, const std::string &name = "New Entity", const Handle& handle = Handle());
    static std::vector<Entity> CreateEntities(
        const EntityArchetype &archetype, const size_t &amount, const std::string &name = "New Entity");
    static std::vector<Entity> CreateEntities(const size_t &amount, const std::string &name = "New Entity");
    static void DeleteEntity(const Entity &entity);
    static EntityQuery CreateEntityQuery();
#pragma region For Each
    template <typename T1 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    static void ForEach(
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
    static void ForEach(
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
    static void ForEach(
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
    static void ForEach(
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
    static void ForEach(
        ThreadPool &workers,
        const EntityQuery &entityQuery,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);
    // For implicit parallel task dispatching
    template <typename T1 = IDataComponent>
    static void ForEach(
        ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &)> &func, bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
        bool checkEnable = true);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    static void ForEach(
        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
        bool checkEnable = true);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    static void ForEach(
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
    static void ForEach(
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
    static void ForEach(
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
    static void ForEach(
        ThreadPool &workers,
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
        bool checkEnable = true);

    // For explicit parallel task dispatching
    template <typename T1 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &)> &func);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &)> &func);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func);
    template <
        typename T1 = IDataComponent,
        typename T2 = IDataComponent,
        typename T3 = IDataComponent,
        typename T4 = IDataComponent,
        typename T5 = IDataComponent,
        typename T6 = IDataComponent,
        typename T7 = IDataComponent>
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
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
    static std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> CreateParallelTask(
        const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func);

#pragma endregion
    static void ForAllEntities(const std::function<void(int i, Entity entity)> &func);
    static std::shared_ptr<Scene> GetCurrentScene();

    static void Init();

    template <typename T = ISystem>
    static std::shared_ptr<T> GetOrCreateSystem(std::shared_ptr<Scene> scene, const float &order);
    template <typename T = ISystem> static std::shared_ptr<T> GetOrCreateSystem(const float &order);
    template <typename T = ISystem> static std::shared_ptr<T> GetSystem(std::shared_ptr<Scene> scene);
    template <typename T = ISystem> static std::shared_ptr<T> GetSystem();
};

#pragma endregion

#pragma region Functions
template <typename T> std::shared_ptr<T> EntityManager::GetSystem(std::shared_ptr<Scene> scene)
{
    const auto search = scene->m_indexedSystems.find(typeid(T).hash_code());
    if (search != scene->m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
template <typename T> std::shared_ptr<T> EntityManager::GetSystem()
{
    auto scene = GetCurrentScene();
    const auto search = scene->m_indexedSystems.find(typeid(T).hash_code());
    if (search != scene->m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
template <typename T>
std::shared_ptr<T> EntityManager::GetOrCreateSystem(std::shared_ptr<Scene> scene, const float &rank)
{
    const auto search = scene->m_indexedSystems.find(typeid(T).hash_code());
    if (search != scene->m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    auto ptr = SerializationManager::ProduceSerializable<T>();
    auto system = std::dynamic_pointer_cast<ISystem>(ptr);
    system->m_handle = Handle();
    system->m_rank = rank;
    scene->m_systems.insert({rank, system});
    scene->m_indexedSystems[typeid(T).hash_code()] = system;
    scene->m_mappedSystems[system->m_handle] = system;
    system->m_started = false;
    system->OnCreate();
    scene->m_saved = false;
    return ptr;
}
template <typename T> std::shared_ptr<T> EntityManager::GetOrCreateSystem(const float &rank)
{
    auto scene = GetCurrentScene();
    return GetOrCreateSystem<T>(scene, rank);
}

#pragma region Collectors

template <typename T> bool EntityManager::CheckDataComponentTypes(T arg)
{
    return std::is_standard_layout<T>::value;
}

template <typename T, typename... Ts> bool EntityManager::CheckDataComponentTypes(T arg, Ts... args)
{
    return std::is_standard_layout<T>::value && CheckDataComponentTypes(args...);
}

template <typename T>
size_t EntityManager::CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg)
{
    const auto type = Typeof<T>();
    componentTypes->push_back(type);
    return type.m_size;
}

template <typename T, typename... Ts>
size_t EntityManager::CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg, Ts... args)
{
    auto offset = CollectDataComponentTypes(componentTypes, args...);
    DataComponentType type = Typeof<T>();
    componentTypes->push_back(type);
    return type.m_size + offset;
}

template <typename T, typename... Ts>
std::vector<DataComponentType> EntityManager::CollectDataComponentTypes(T arg, Ts... args)
{
    auto retVal = std::vector<DataComponentType>();
    retVal.push_back(Typeof<Transform>());
    retVal.push_back(Typeof<GlobalTransform>());
    retVal.push_back(Typeof<GlobalTransformUpdateFlag>());
    CollectDataComponentTypes(&retVal, arg, args...);
    std::sort(retVal.begin() + 3, retVal.end(), ComponentTypeComparator);
    size_t offset = 0;
    EraseDuplicates(retVal);
    for (auto &i : retVal)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    return retVal;
}
#pragma endregion
#pragma region ForEachStorage
template <typename T1>
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
                            return;
                        func(static_cast<int>(i), entity, address1[remainder]);
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        const auto chunkIndex = i / capacity;
                        const auto remainder = i % capacity;
                        auto *data = static_cast<char *>(chunkArray.m_chunks[chunkIndex].m_data);
                        T1 *address1 = reinterpret_cast<T1 *>(data + targetType1.m_offset * capacity);
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
                            return;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
                            return;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
                            return;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
void EntityManager::ForEachStorage(
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
    const auto &entities = &chunkArray.m_entities;
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
                        const auto entity = entities->at(i);
                        if (checkEnable && !GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_enabled)
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
#pragma region Others
template <typename T>
void EntityManager::GetDataComponentArrayStorage(const DataComponentStorage &storage, std::vector<T> &container)
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
            container.resize(container.size() + amount);
            const auto capacity = storage.m_chunkCapacity;
            const auto chunkAmount = amount / capacity;
            const auto remainAmount = amount % capacity;
            for (size_t i = 0; i < chunkAmount; i++)
            {
                memcpy(
                    &container.at(container.size() - remainAmount - capacity * (chunkAmount - i)),
                    reinterpret_cast<void *>(
                        static_cast<char *>(storage.m_chunkArray.m_chunks[i].m_data) + capacity * targetType.m_offset),
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

template <typename T, typename... Ts>
EntityArchetype EntityManager::CreateEntityArchetype(const std::string &name, T arg, Ts... args)
{
    EntityArchetype retVal = EntityArchetype();
    if (!CheckDataComponentTypes(arg, args...))
    {
        UNIENGINE_ERROR("CreateEntityArchetype failed: Standard Layout");
        return retVal;
    }
    auto &entityManager = GetInstance();
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;
    EntityArchetypeInfo info;
    info.m_name = name;
    info.m_dataComponentTypes = CollectDataComponentTypes(arg, args...);
    info.m_entitySize = info.m_dataComponentTypes.back().m_offset + info.m_dataComponentTypes.back().m_size;
    info.m_chunkCapacity = GetInstance().m_archetypeChunkSize / info.m_entitySize;
    retVal = CreateEntityArchetypeHelper(info);
    RefreshAllEntityArchetypeInfos();
    return retVal;
}
#pragma endregion
#pragma region GetSetHas
template <typename T> void EntityManager::AddDataComponent(const Entity &entity, const T &value)
{
    assert(entity.IsValid());
    const auto id = typeid(T).hash_code();
    auto &entityManager = GetInstance();
    auto &entityInfo = entityManager.m_entityMetaDataCollection->at(entity.m_index);
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;

#pragma region Check if componentdata already exists.If yes, go to SetComponentData
    auto &dataComponentStorage = entityManager.m_entityDataComponentStorage->at(entityInfo.m_dataComponentStorageIndex);
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
    entityManager.m_scene->m_saved = false;
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
    EraseDuplicates(newArchetypeInfo.m_dataComponentTypes);
    for (auto &i : newArchetypeInfo.m_dataComponentTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    newArchetypeInfo.m_entitySize =
        newArchetypeInfo.m_dataComponentTypes.back().m_offset + newArchetypeInfo.m_dataComponentTypes.back().m_size;
    newArchetypeInfo.m_chunkCapacity = GetInstance().m_archetypeChunkSize / newArchetypeInfo.m_entitySize;
    auto archetype = CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype.
    Entity newEntity = CreateEntity(archetype);
    // Transfer component data
    for (const auto &type : originalComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    newEntity.SetDataComponent(value);
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = GetInstance().m_entityMetaDataCollection->at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = entityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    entityManager.m_entityDataComponentStorage->at(entityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
    entityManager.m_entityDataComponentStorage->at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
}

template <typename T> void EntityManager::RemoveDataComponent(const Entity &entity)
{
    assert(entity.IsValid());
    const auto id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code() || id == typeid(GlobalTransform).hash_code() ||
        id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return;
    }
    auto &entityManager = GetInstance();
    auto &entityInfo = entityManager.m_entityMetaDataCollection->at(entity.m_index);
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;
#pragma region Check if componentdata already exists.If yes, go to SetComponentData
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    if (dataComponentStorage.m_dataComponentTypes.size() <= 3)
    {
        UNIENGINE_ERROR("Remove Component Data failed: Entity must have at least 1 data component besides 3 basic data "
                        "components!");
        return;
    }
    entityManager.m_scene->m_saved = false;
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
    newArchetypeInfo.m_chunkCapacity = GetInstance().m_archetypeChunkSize / newArchetypeInfo.m_entitySize;
    auto archetype = CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(archetype);
    // Transfer component data
    for (const auto &type : newArchetypeInfo.m_dataComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    T retVal = entity.GetDataComponent<T>();
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = GetInstance().m_entityMetaDataCollection->at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = entityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    entityManager.m_entityDataComponentStorage->at(entityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
    entityManager.m_entityDataComponentStorage->at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
    return;
}

template <typename T> void EntityManager::SetDataComponent(const Entity &entity, const T &value)
{
    assert(entity.IsValid());
    SetDataComponent(entity.m_index, typeid(T).hash_code(), sizeof(T), (IDataComponent *)&value);
}
template <typename T> void EntityManager::SetDataComponent(const size_t &index, const T &value)
{
    const size_t id = typeid(T).hash_code();
    assert(index < GetInstance().m_entityMetaDataCollection->size());
    SetDataComponent(index, id, sizeof(T), (IDataComponent *)&value);
}
template <typename T> T EntityManager::GetDataComponent(const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    EntityMetadata &entityInfo = entityManager.m_entityMetaDataCollection->at(entity.m_index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
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
template <typename T> bool EntityManager::HasDataComponent(const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    EntityMetadata &entityInfo = entityManager.m_entityMetaDataCollection->at(entity.m_index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
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
template <typename T> T EntityManager::GetDataComponent(const size_t &index)
{
    if (index > GetInstance().m_entityMetaDataCollection->size())
        return T();
    auto &entityManager = GetInstance();
    EntityMetadata &entityInfo = entityManager.m_entityMetaDataCollection->at(index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    const size_t chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const size_t chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    ComponentDataChunk &chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    ;
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
template <typename T> bool EntityManager::HasDataComponent(const size_t &index)
{
    if (index > GetInstance().m_entityMetaDataCollection->size())
        return false;
    auto &entityManager = GetInstance();
    EntityMetadata &entityInfo = entityManager.m_entityMetaDataCollection->at(index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];

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

template <typename T> std::weak_ptr<T> EntityManager::GetOrSetPrivateComponent(const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    auto typeName = SerializationManager::GetSerializableTypeName<T>();
    size_t i = 0;
    auto &elements = entityManager.m_entityMetaDataCollection->at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (typeName == element.m_privateComponentData->GetTypeName())
        {
            return std::static_pointer_cast<T>(element.m_privateComponentData);
        }
        i++;
    }
    entityManager.m_entityPrivateComponentStorage->SetPrivateComponent<T>(entity);
    auto ptr = SerializationManager::ProduceSerializable<T>();
    elements.emplace_back(typeid(T).hash_code(), ptr, entity);
    entityManager.m_scene->m_saved = false;
    return std::move(ptr);
}
template <typename T> void EntityManager::RemovePrivateComponent(const Entity &entity)
{
    assert(entity.IsValid());
    auto &elements = GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_privateComponentElements;
    for (auto i = 0; i < elements.size(); i++)
    {
        if (std::dynamic_pointer_cast<T>(elements[i].m_privateComponentData))
        {
            GetInstance().m_entityPrivateComponentStorage->RemovePrivateComponent<T>(entity);
            elements[i].m_privateComponentData->OnDestroy();
            elements.erase(elements.begin() + i);
            GetInstance().m_scene->m_saved = false;
            return;
        }
    }
}

template <typename T> bool EntityManager::HasPrivateComponent(const Entity &entity)
{
    assert(entity.IsValid());
    for (auto &element : GetInstance().m_entityMetaDataCollection->at(entity.m_index).m_privateComponentElements)
    {
        if (std::dynamic_pointer_cast<T>(element.m_privateComponentData))
        {
            return true;
        }
    }
    return false;
}

template <typename T, typename... Ts>
void EntityManager::SetEntityQueryAllFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_allDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
    RefreshEntityQueryInfo(entityQuery.m_index);
}

template <typename T, typename... Ts>
void EntityManager::SetEntityQueryAnyFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_anyDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
    RefreshEntityQueryInfo(entityQuery.m_index);
}

template <typename T, typename... Ts>
void EntityManager::SetEntityQueryNoneFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_noneDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
    RefreshEntityQueryInfo(entityQuery.m_index);
}
#pragma endregion
#pragma region For Each
template <typename T1>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void EntityManager::ForEach(
    ThreadPool &workers,
    const EntityQuery &entityQuery,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto entityManager = GetInstance();
    for (const auto i : entityManager.m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1>
void EntityManager::ForEach(
    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &)> &func, bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2>
void EntityManager::ForEach(
    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &, T2 &)> &func, bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3>
void EntityManager::ForEach(
    ThreadPool &workers, const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func, bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4>
void EntityManager::ForEach(
    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func,
    bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void EntityManager::ForEach(
    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func,
    bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void EntityManager::ForEach(
    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func,
    bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void EntityManager::ForEach(
    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func,
    bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void EntityManager::ForEach(
    ThreadPool &workers,
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func,
    bool checkEnable)
{
    auto &manager = GetInstance();
    auto *storages = manager.m_entityDataComponentStorage;
    for (auto i = storages->begin() + 1; i < storages->end(); ++i)
    {
        ForEachStorage(workers, *i, func, checkEnable);
    }
}

template <typename T1>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> EntityManager::CreateParallelTask(
    const std::function<void(int i, Entity entity, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &)> &func)
{
    std::packaged_task<void(ThreadPool &, const EntityQuery &, bool)> task(
        [func](ThreadPool &workers, const EntityQuery &entityQuery, bool checkEnable) {
            assert(entityQuery.IsValid());
            for (const auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
            {
                ForEachStorage(workers, *i, func, checkEnable);
            }
        });
    return task;
}
#pragma endregion
template <typename T>
void EntityManager::GetComponentDataArray(const EntityQuery &entityQuery, std::vector<T> &container)
{
    assert(entityQuery.IsValid());
    for (const auto *i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        GetDataComponentArrayStorage(*i, container);
    }
}

template <typename T1, typename T2>
void EntityManager::GetComponentDataArray(
    const EntityQuery &entityQuery, std::vector<T1> &container, const std::function<bool(const T2 &)> &filterFunc)
{
    assert(entityQuery.IsValid());
    std::vector<T2> componentDataList;
    std::vector<T1> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList);
    GetComponentDataArray(entityQuery, targetDataList);
    if (targetDataList.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList.size();
    std::vector<std::vector<T1>> collectedDataLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T1>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T1> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            JobManager::PrimaryWorkers()
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
void EntityManager::GetComponentDataArray(
    const EntityQuery &entityQuery,
    std::vector<T1> &container,
    const std::function<bool(const T2 &, const T3 &)> &filterFunc)
{
    assert(entityQuery.IsValid());
    std::vector<T3> componentDataList2;
    std::vector<T2> componentDataList1;
    std::vector<T1> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList2);
    GetComponentDataArray(entityQuery, componentDataList1);
    GetComponentDataArray(entityQuery, targetDataList);
    if (targetDataList.size() != componentDataList1.size() || componentDataList1.size() != componentDataList2.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList1.size();
    std::vector<std::vector<T1>> collectedDataLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T1>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T1> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            JobManager::PrimaryWorkers()
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
void EntityManager::GetComponentDataArray(const EntityQuery &entityQuery, const T1 &filter, std::vector<T2> &container)
{
    assert(entityQuery.IsValid());
    std::vector<T1> componentDataList;
    std::vector<T2> targetDataList;
    GetComponentDataArray(entityQuery, componentDataList);
    GetComponentDataArray(entityQuery, targetDataList);
    if (targetDataList.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = componentDataList.size();
    std::vector<std::vector<T2>> collectedDataLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedDataLists.push_back(std::vector<T2>());
    }
    for (int i = 0; i < collectedDataLists.size(); i++)
    {
        std::vector<T2> *collectedDataList = &collectedDataLists[i];
        futures.push_back(
            JobManager::PrimaryWorkers()
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
void EntityManager::GetEntityArray(
    const EntityQuery &entityQuery,
    std::vector<Entity> &container,
    const std::function<bool(const Entity &, const T1 &)> &filterFunc)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList;
    GetEntityArray(entityQuery, allEntities);
    GetComponentDataArray(entityQuery, componentDataList);
    if (allEntities.size() != componentDataList.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(
            JobManager::PrimaryWorkers()
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
void EntityManager::GetEntityArray(
    const EntityQuery &entityQuery,
    std::vector<Entity> &container,
    const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList1;
    std::vector<T2> componentDataList2;
    GetEntityArray(entityQuery, allEntities);
    GetComponentDataArray(entityQuery, componentDataList1);
    GetComponentDataArray(entityQuery, componentDataList2);
    if (allEntities.size() != componentDataList1.size() || componentDataList1.size() != componentDataList2.size())
        return;
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(JobManager::PrimaryWorkers()
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
void EntityManager::GetEntityArray(const EntityQuery &entityQuery, const T1 &filter, std::vector<Entity> &container)
{
    assert(entityQuery.IsValid());
    std::vector<Entity> allEntities;
    std::vector<T1> componentDataList;
    GetEntityArray(entityQuery, allEntities);
    GetComponentDataArray(entityQuery, componentDataList);
    std::vector<std::shared_future<void>> futures;
    size_t size = allEntities.size();
    std::vector<std::vector<Entity>> collectedEntityLists;
    const auto threadSize = JobManager::PrimaryWorkers().Size();
    for (int i = 0; i < threadSize; i++)
    {
        collectedEntityLists.push_back(std::vector<Entity>());
    }
    for (int i = 0; i < collectedEntityLists.size(); i++)
    {
        std::vector<Entity> *collectedEntityList = &collectedEntityLists[i];
        futures.push_back(
            JobManager::PrimaryWorkers()
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
std::vector<std::pair<T *, size_t>> EntityManager::UnsafeGetDataComponentArray(const EntityQuery &entityQuery)
{
    std::vector<std::pair<T *, size_t>> retVal;
    assert(entityQuery.IsValid());
    for (const auto &i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        auto targetType = Typeof<T>();
        const auto entityCount = i->m_entityAliveCount;
        auto found = false;
        for (const auto &type : i->m_dataComponentTypes)
        {
            if (type.m_typeId == targetType.m_typeId)
            {
                targetType = type;
                found = true;
            }
        }
        if (!found)
            continue;
        const auto capacity = i->m_chunkCapacity;
        const auto &chunkArray = i->m_chunkArray;
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

template <typename T> const std::vector<Entity> *EntityManager::UnsafeGetPrivateComponentOwnersList()
{
    return GetInstance().m_entityPrivateComponentStorage->UnsafeGetOwnersList<T>();
}


template <typename T> void Entity::SetDataComponent(const T &value) const
{
    EntityManager::SetDataComponent(*this, value);
}

template <typename T> T Entity::GetDataComponent() const
{
    return std::move(EntityManager::GetDataComponent<T>(*this));
}

template <typename T> bool Entity::HasDataComponent() const
{
    return EntityManager::HasDataComponent<T>(*this);
}

template <typename T> std::weak_ptr<T> Entity::GetOrSetPrivateComponent() const
{
    return std::move(EntityManager::GetOrSetPrivateComponent<T>(*this));
}

template <typename T> void Entity::RemovePrivateComponent() const
{
    EntityManager::RemovePrivateComponent<T>(*this);
}

template <typename T> bool Entity::HasPrivateComponent() const
{
    return EntityManager::HasPrivateComponent<T>(*this);
}

template <typename T> T ComponentDataChunk::GetData(const size_t &offset)
{
    return T(*reinterpret_cast<T *>(static_cast<char *>(m_data) + offset));
}

template <typename T> void ComponentDataChunk::SetData(const size_t &offset, const T &data)
{
    *reinterpret_cast<T *>(static_cast<char *>(m_data) + offset) = data;
}

template <typename T, typename... Ts> void EntityQuery::SetAllFilters(T arg, Ts... args)
{
    EntityManager::SetEntityQueryAllFilters(*this, arg, args...);
}

template <typename T, typename... Ts> void EntityQuery::SetAnyFilters(T arg, Ts... args)
{
    EntityManager::SetEntityQueryAnyFilters(*this, arg, args...);
}

template <typename T, typename... Ts> void EntityQuery::SetNoneFilters(T arg, Ts... args)
{
    EntityManager::SetEntityQueryNoneFilters(*this, arg, args...);
}

template <typename T1> void EntityQuery::ToComponentDataArray(std::vector<T1> &container)
{
    EntityManager::GetComponentDataArray<T1>(*this, container);
}

template <typename T1, typename T2>
void EntityQuery::ToComponentDataArray(std::vector<T1> &container, const std::function<bool(const T2 &)> &filterFunc)
{
    EntityManager::GetComponentDataArray(*this, container, filterFunc);
}

template <typename T1, typename T2, typename T3>
void EntityQuery::ToComponentDataArray(
    std::vector<T1> &container, const std::function<bool(const T2 &, const T3 &)> &filterFunc)
{
    EntityManager::GetComponentDataArray(*this, container, filterFunc);
}

template <typename T1, typename T2> void EntityQuery::ToComponentDataArray(const T1 &filter, std::vector<T2> &container)
{
    EntityManager::GetComponentDataArray(*this, filter, container);
}
template <typename T1> void EntityQuery::ToEntityArray(const T1 &filter, std::vector<Entity> &container)
{
    EntityManager::GetEntityArray(*this, filter, container);
}

template <typename T1>
void EntityQuery::ToEntityArray(
    std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &)> &filterFunc)
{
    EntityManager::GetEntityArray<T1>(*this, container, filterFunc);
}

template <typename T1, typename T2>
void EntityQuery::ToEntityArray(
    std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc)
{
    EntityManager::GetEntityArray<T1>(*this, container, filterFunc);
}
#pragma endregion

} // namespace UniEngine