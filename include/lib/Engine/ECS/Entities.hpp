#pragma once
#include "Console.hpp"
#include "Entity.hpp"
#include "EntityMetadata.hpp"
#include "IPrivateComponent.hpp"
#include "ISerializable.hpp"
#include "ISingleton.hpp"
#include "ISystem.hpp"
#include "Jobs.hpp"
#include "Serialization.hpp"
#include "Transform.hpp"

namespace UniEngine
{
template <typename T> DataComponentType Typeof()
{
    DataComponentType type;
    type.m_name = Serialization::GetDataComponentTypeName<T>();
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

class UNIENGINE_API Entities final : ISingleton<Entities>
{
    friend class PhysicsSystem;
    friend class EditorLayer;
    friend class PrefabHolder;
    friend class PrivateComponentStorage;
    friend class TransformLayer;
    friend class Editor;
    friend class Scene;
    friend class Serialization;
    friend struct EntityArchetype;
    friend struct EntityQuery;
    friend struct Entity;
    friend struct Application;
    friend struct PrivateComponentRef;
    friend class Prefab;
    size_t m_archetypeChunkSize = ARCHETYPE_CHUNK_SIZE;
    EntityArchetype m_basicArchetype = EntityArchetype();

    std::vector<EntityArchetypeInfo> m_entityArchetypeInfos;
    std::vector<EntityQueryInfo> m_entityQueryInfos;

#pragma region Helpers
    static EntityArchetype CreateEntityArchetypeHelper(const EntityArchetypeInfo &info);

    template <typename T = IDataComponent> static bool CheckDataComponentTypes(T arg);
    template <typename T = IDataComponent, typename... Ts> static bool CheckDataComponentTypes(T arg, Ts... args);
    template <typename T = IDataComponent>
    static size_t CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg);
    template <typename T = IDataComponent, typename... Ts>
    static size_t CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts>
    static std::vector<DataComponentType> CollectDataComponentTypes(T arg, Ts... args);

  public:

    static EntityArchetype CreateEntityArchetype(const std::string &name, const std::vector<DataComponentType> &types);

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

    static EntityArchetype GetDefaultEntityArchetype();

    static size_t GetArchetypeChunkSize();
    static EntityArchetypeInfo GetArchetypeInfo(const EntityArchetype &entityArchetype);

    template <typename T = IDataComponent, typename... Ts>
    static EntityArchetype CreateEntityArchetype(const std::string &name, T arg, Ts... args);
    static EntityQuery CreateEntityQuery();

    static void Init();
};




#pragma endregion

#pragma region Functions

template <typename T, typename... Ts>
void Entities::SetEntityQueryAllFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_allDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
}

template <typename T, typename... Ts>
void Entities::SetEntityQueryAnyFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_anyDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
}

template <typename T, typename... Ts>
void Entities::SetEntityQueryNoneFilters(const EntityQuery &entityQuery, T arg, Ts... args)
{
    assert(entityQuery.IsValid());
    GetInstance().m_entityQueryInfos[entityQuery.m_index].m_noneDataComponentTypes =
        CollectDataComponentTypes(arg, args...);
}
#pragma region Collectors

template <typename T> bool Entities::CheckDataComponentTypes(T arg)
{
    return std::is_standard_layout<T>::value;
}

template <typename T, typename... Ts> bool Entities::CheckDataComponentTypes(T arg, Ts... args)
{
    return std::is_standard_layout<T>::value && CheckDataComponentTypes(args...);
}

template <typename T>
size_t Entities::CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg)
{
    const auto type = Typeof<T>();
    componentTypes->push_back(type);
    return type.m_size;
}

template <typename T, typename... Ts>
size_t Entities::CollectDataComponentTypes(std::vector<DataComponentType> *componentTypes, T arg, Ts... args)
{
    auto offset = CollectDataComponentTypes(componentTypes, args...);
    DataComponentType type = Typeof<T>();
    componentTypes->push_back(type);
    return type.m_size + offset;
}

template <typename T, typename... Ts>
std::vector<DataComponentType> Entities::CollectDataComponentTypes(T arg, Ts... args)
{
    auto retVal = std::vector<DataComponentType>();
    retVal.push_back(Typeof<Transform>());
    retVal.push_back(Typeof<GlobalTransform>());
    retVal.push_back(Typeof<GlobalTransformUpdateFlag>());
    CollectDataComponentTypes(&retVal, arg, args...);
    std::sort(retVal.begin() + 3, retVal.end(), ComponentTypeComparator);
    size_t offset = 0;

    std::vector<DataComponentType> copy;
    copy.insert(copy.begin(), retVal.begin(), retVal.end());
    retVal.clear();
    for (const auto &i : copy)
    {
        bool found = false;
        for (const auto j : retVal)
        {
            if (i == j)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        retVal.push_back(i);
    }
    for (auto &i : retVal)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    return retVal;
}
#pragma endregion

#pragma region Others


template <typename T, typename... Ts>
EntityArchetype Entities::CreateEntityArchetype(const std::string &name, T arg, Ts... args)
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
    return retVal;
}
#pragma endregion


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
    Entities::SetEntityQueryAllFilters(*this, arg, args...);
}

template <typename T, typename... Ts> void EntityQuery::SetAnyFilters(T arg, Ts... args)
{
    Entities::SetEntityQueryAnyFilters(*this, arg, args...);
}

template <typename T, typename... Ts> void EntityQuery::SetNoneFilters(T arg, Ts... args)
{
    Entities::SetEntityQueryNoneFilters(*this, arg, args...);
}

#pragma endregion

} // namespace UniEngine
