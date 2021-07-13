#pragma once
#include <Serializable.hpp>
namespace UniEngine
{
#pragma region EntityManager
#pragma region Entity
struct UNIENGINE_API ComponentDataType final
{
    std::string m_name;
    size_t m_typeId = 0;
    size_t m_size = 0;
    size_t m_offset = 0;
    ComponentDataType() = default;
    ComponentDataType(const std::string &name, const size_t &id, const size_t &size);
    bool operator==(const ComponentDataType &other) const;
    bool operator!=(const ComponentDataType &other) const;
};
struct UNIENGINE_API ComponentDataBase
{
};

struct UNIENGINE_API EntityArchetype final
{
    size_t m_index;
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] std::string GetName() const;
};

class PrivateComponentBase;
struct UNIENGINE_API Entity final
{
    unsigned m_index = 0;
    unsigned m_version = 0;
    Entity();
    bool operator==(const Entity &other) const;
    bool operator!=(const Entity &other) const;
    size_t operator()(Entity const &key) const;
    [[nodiscard]] bool IsEnabled() const;
    void SetStatic(const bool &value) const;
    void SetEnabled(const bool &value) const;
    void SetEnabledSingle(const bool &value) const;
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsStatic() const;
    [[nodiscard]] bool IsDeleted() const;
    [[nodiscard]] bool IsValid() const;

    void SetParent(const Entity &parent, const bool &recalculateTransform = true) const;
    [[nodiscard]] Entity GetParent() const;
    [[nodiscard]] std::vector<Entity> GetChildren() const;
    [[nodiscard]] size_t GetChildrenAmount() const;
    [[nodiscard]] Entity GetRoot() const;
    void ForEachChild(const std::function<void(Entity child)> &func) const;
    void RemoveChild(const Entity &child) const;
    [[nodiscard]] EntityArchetype GetEntityArchetype() const;

    template <typename T = ComponentDataBase> void SetComponentData(const T &value) const;
    template <typename T = ComponentDataBase> T GetComponentData() const;
    template <typename T = ComponentDataBase> [[nodiscard]] bool HasComponentData() const;
    template <typename T = ComponentDataBase> void RemoveComponentData() const;

    template <typename T = PrivateComponentBase> std::unique_ptr<T> &SetPrivateComponent() const;
    template <typename T = PrivateComponentBase> std::unique_ptr<T> &GetPrivateComponent() const;
    template <typename T = PrivateComponentBase> [[nodiscard]] bool HasPrivateComponent() const;
    template <typename T = PrivateComponentBase> void RemovePrivateComponent() const;

    [[nodiscard]] std::string GetName() const;
    void SetName(const std::string &name) const;
};
#pragma region Storage

class UNIENGINE_API PrivateComponentBase : public Serializable
{
    friend class EntityManager;
    friend class EditorManager;
    friend struct PrivateComponentElement;
    friend class SerializationManager;
    bool m_enabled = true;
    Entity m_owner = Entity();
  public:
    [[nodiscard]] Entity GetOwner() const;
    void SetEnabled(const bool &value);
    [[nodiscard]] bool IsEnabled() const;
    virtual void Init();
    virtual void OnEnable();
    virtual void OnDisable();
    virtual void OnEntityEnable();
    virtual void OnEntityDisable();
    virtual void OnGui();
};

template <typename T> ComponentDataType Typeof()
{
    ComponentDataType type;
    type.m_name = std::string(typeid(T).name());
    type.m_size = sizeof(T);
    type.m_offset = 0;
    type.m_typeId = typeid(T).hash_code();
    return type;
}

const size_t ARCHETYPE_CHUNK_SIZE = 16384;

struct ComponentDataChunk
{
    void *m_data;
    template <typename T> T GetData(const size_t &offset);
    [[nodiscard]] ComponentDataBase *GetDataPointer(const size_t &offset) const;
    template <typename T> void SetData(const size_t &offset, const T &data);
    void SetData(const size_t &offset, const size_t &size, ComponentDataBase *data) const;
    void ClearData(const size_t &offset, const size_t &size) const;
};

struct ComponentDataChunkArray
{
    std::vector<Entity> Entities;
    std::vector<ComponentDataChunk> Chunks;
};



struct PrivateComponentElement
{
    std::string m_name;
    size_t m_typeId;
    std::unique_ptr<PrivateComponentBase> m_privateComponentData;
    UNIENGINE_API PrivateComponentElement(
        const std::string &name, const size_t &id, std::unique_ptr<PrivateComponentBase> data, const Entity &owner);
    UNIENGINE_API void ResetOwner(const Entity &newOwner) const;
};

struct EntityInfo
{
    friend class PrivateComponentStorage;
    std::string m_name;
    unsigned m_version = 1;
    bool m_static = false;
    bool m_enabled = true;
    Entity m_parent = Entity();
    std::vector<PrivateComponentElement> m_privateComponentElements;
    std::vector<Entity> m_children;
    size_t m_archetypeInfoIndex;
    size_t m_chunkArrayIndex;
};

struct UNIENGINE_API EntityArchetypeInfo
{
    std::string m_name;
    size_t m_index = 0;
    std::vector<ComponentDataType> m_componentTypes;
    size_t m_entitySize = 0;
    size_t m_chunkCapacity = 0;
    size_t m_entityCount = 0;
    size_t m_entityAliveCount = 0;
    template <typename T> bool HasType();
    bool HasType(const size_t &typeId);
};

struct UNIENGINE_API EntityQuery final
{
    EntityQuery();
    size_t m_index = 0;
    bool operator==(const EntityQuery &other) const;
    bool operator!=(const EntityQuery &other) const;
    size_t operator()(const EntityQuery &key) const;
    [[nodiscard]] bool IsNull() const;
    template <typename T = ComponentDataBase, typename... Ts> void SetAllFilters(T arg, Ts... args);
    template <typename T = ComponentDataBase, typename... Ts> void SetAnyFilters(T arg, Ts... args);
    template <typename T = ComponentDataBase, typename... Ts> void SetNoneFilters(T arg, Ts... args);
    template <typename T1 = ComponentDataBase> void ToComponentDataArray(std::vector<T1> &container);
    template <typename T1 = ComponentDataBase, typename T2 = ComponentDataBase>
    void ToComponentDataArray(std::vector<T1> &container, const std::function<bool(const T2 &)> &filterFunc);
    template <typename T1 = ComponentDataBase, typename T2 = ComponentDataBase, typename T3 = ComponentDataBase>
    void ToComponentDataArray(
        std::vector<T1> &container, const std::function<bool(const T2 &, const T3 &)> &filterFunc);
    template <typename T1 = ComponentDataBase, typename T2 = ComponentDataBase>
    void ToComponentDataArray(const T1 &filter, std::vector<T2> &container);
    void ToEntityArray(std::vector<Entity> &container) const;
    template <typename T1 = ComponentDataBase> void ToEntityArray(const T1 &filter, std::vector<Entity> &container);
    template <typename T1 = ComponentDataBase>
    void ToEntityArray(
        std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &)> &filterFunc);
    template <typename T1 = ComponentDataBase, typename T2 = ComponentDataBase>
    void ToEntityArray(
        std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc);
    [[nodiscard]] size_t GetEntityAmount() const;
};
struct EntityComponentDataStorage
{
    EntityArchetypeInfo *m_archetypeInfo = nullptr;
    ComponentDataChunkArray *m_chunkArray = nullptr;
    UNIENGINE_API EntityComponentDataStorage();
    UNIENGINE_API EntityComponentDataStorage(EntityArchetypeInfo *info, ComponentDataChunkArray *array);
};
struct EntityQueryInfo
{
    std::vector<ComponentDataType> m_allComponentTypes;
    std::vector<ComponentDataType> m_anyComponentTypes;
    std::vector<ComponentDataType> m_noneComponentTypes;
    std::vector<EntityComponentDataStorage> m_queriedStorage;
};
#pragma endregion
#pragma endregion
#pragma endregion
template <typename T> bool EntityArchetypeInfo::HasType()
{
    for (const auto &i : m_componentTypes)
    {
        if (i.m_typeId == typeid(T).hash_code())
            return true;
    }
    return false;
}
} // namespace UniEngine