#pragma once
#include <IAsset.hpp>
#include <IDataComponent.hpp>
namespace UniEngine
{
#pragma region EntityManager
#pragma region Entity
struct UNIENGINE_API DataComponentType final
{
    std::string m_name;
    size_t m_typeId = 0;
    size_t m_size = 0;
    size_t m_offset = 0;
    DataComponentType() = default;
    DataComponentType(const std::string &name, const size_t &id, const size_t &size);
    bool operator==(const DataComponentType &other) const;
    bool operator!=(const DataComponentType &other) const;
};

struct UNIENGINE_API EntityArchetype final
{
  private:
    friend class EntityManager;
    friend class SerializationManager;
    size_t m_index = 0;

  public:
    size_t GetIndex();
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] std::string GetName() const;
    void SetName(const std::string &name) const;
};
class IPrivateComponent;
struct UNIENGINE_API Entity final
{
  private:
    friend class EntityManager;
    friend class Scene;
    friend class EntityMetadata;
    friend class SerializationManager;
    unsigned m_index = 0;
    unsigned m_version = 0;

  public:
    [[nodiscard]] Handle GetHandle() const;

    [[nodiscard]] unsigned GetIndex() const;
    [[nodiscard]] unsigned GetVersion() const;
    bool operator==(const Entity &other) const;
    bool operator!=(const Entity &other) const;
    size_t operator()(Entity const &key) const;
    [[nodiscard]] bool IsEnabled() const;
    void SetEnabled(const bool &value) const;
    void SetEnabledSingle(const bool &value) const;
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsValid() const;

    void SetParent(const Entity &parent, const bool &recalculateTransform = false) const;
    [[nodiscard]] Entity GetParent() const;
    [[nodiscard]] std::vector<Entity> GetChildren() const;
    [[nodiscard]] size_t GetChildrenAmount() const;
    [[nodiscard]] Entity GetRoot() const;
    void ForEachChild(const std::function<void(Entity child)> &func) const;
    void RemoveChild(const Entity &child) const;
    [[nodiscard]] std::vector<Entity> GetDescendants() const;
    void ForEachDescendant(const std::function<void(const Entity &entity)> &func, const bool &fromRoot = true) const;
    template <typename T = IDataComponent> void SetDataComponent(const T &value) const;
    template <typename T = IDataComponent> T GetDataComponent() const;
    template <typename T = IDataComponent> [[nodiscard]] bool HasDataComponent() const;
    template <typename T = IDataComponent> void RemoveDataComponent() const;

    template <typename T = IPrivateComponent> std::weak_ptr<T> GetOrSetPrivateComponent() const;
    template <typename T = IPrivateComponent> [[nodiscard]] bool HasPrivateComponent() const;
    template <typename T = IPrivateComponent> void RemovePrivateComponent() const;

    [[nodiscard]] std::string GetName() const;
    void SetName(const std::string &name) const;
};
#pragma region Storage

class UNIENGINE_API EntityRef : public ISerializable
{
    Entity m_value = Entity();
    Handle m_entityHandle = Handle(0);
    void Update();

  public:
    void Serialize(YAML::Emitter &out) override
    {
      out << YAML::Key << "m_entityHandle" << YAML::Value << m_entityHandle;
    }
    void Deserialize(const YAML::Node &in) override
    {
      m_entityHandle = Handle(in["m_entityHandle"].as<uint64_t>());
      Update();
    }
    EntityRef()
    {
        m_entityHandle = Handle(0);
        m_value = Entity();
    }
    template <typename T = IAsset> EntityRef(const Entity &other)
    {
        Set(other);
    }
    EntityRef &operator=(const Entity &other)
    {
        Set(other);
        return *this;
    }

    EntityRef &operator=(Entity &&other) noexcept
    {
        Set(other);
        return *this;
    }

    void Relink(const std::unordered_map<Handle, Handle> &map)
    {
        if(m_entityHandle.GetValue() == 0) return;
        auto search = map.find(m_entityHandle);
        if (search != map.end())
        {
            m_entityHandle = search->second;
            m_value = Entity();
        }
        else
            m_entityHandle = Handle(0);
    };
    [[nodiscard]] Entity Get()
    {
        Update();
        return m_value;
    }
    [[nodiscard]] Handle GetEntityHandle() const
    {
        return m_entityHandle;
    }
    void Set(const Entity &target);
    void Clear();

    void Save(const std::string& name, YAML::Emitter &out){
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string& name, const YAML::Node &in){
        Deserialize(in[name]);
    }
};
const size_t ARCHETYPE_CHUNK_SIZE = 16384;

struct UNIENGINE_API ComponentDataChunk
{
    void *m_data;
    template <typename T> T GetData(const size_t &offset);
    [[nodiscard]] IDataComponent *GetDataPointer(const size_t &offset) const;
    template <typename T> void SetData(const size_t &offset, const T &data);
    void SetData(const size_t &offset, const size_t &size, IDataComponent *data) const;
    void ClearData(const size_t &offset, const size_t &size) const;
};

struct UNIENGINE_API DataComponentChunkArray
{
    std::vector<Entity> m_entities;
    std::vector<ComponentDataChunk> m_chunks;
};

struct UNIENGINE_API EntityArchetypeInfo
{
    std::string m_name = "New Entity Archetype";
    size_t m_dataComponentStorageIndex = 0;
    size_t m_entitySize = 0;
    size_t m_chunkCapacity = 0;
    std::vector<DataComponentType> m_dataComponentTypes;
    template <typename T> bool HasType();
    bool HasType(const size_t &typeId);
};

struct UNIENGINE_API EntityQuery final
{
  private:
    friend class EntityManager;
    friend class SerializationManager;
    size_t m_index = 0;

  public:
    size_t GetIndex();
    bool operator==(const EntityQuery &other) const;
    bool operator!=(const EntityQuery &other) const;
    size_t operator()(const EntityQuery &key) const;
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsValid() const;
    template <typename T = IDataComponent, typename... Ts> void SetAllFilters(T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts> void SetAnyFilters(T arg, Ts... args);
    template <typename T = IDataComponent, typename... Ts> void SetNoneFilters(T arg, Ts... args);
    template <typename T1 = IDataComponent> void ToComponentDataArray(std::vector<T1> &container);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ToComponentDataArray(std::vector<T1> &container, const std::function<bool(const T2 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent, typename T3 = IDataComponent>
    void ToComponentDataArray(
        std::vector<T1> &container, const std::function<bool(const T2 &, const T3 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ToComponentDataArray(const T1 &filter, std::vector<T2> &container);
    void ToEntityArray(std::vector<Entity> &container) const;
    template <typename T1 = IDataComponent> void ToEntityArray(const T1 &filter, std::vector<Entity> &container);
    template <typename T1 = IDataComponent>
    void ToEntityArray(
        std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &)> &filterFunc);
    template <typename T1 = IDataComponent, typename T2 = IDataComponent>
    void ToEntityArray(
        std::vector<Entity> &container, const std::function<bool(const Entity &, const T1 &, const T2 &)> &filterFunc);
    [[nodiscard]] size_t GetEntityAmount() const;
};
struct UNIENGINE_API DataComponentStorage
{
    std::vector<DataComponentType> m_dataComponentTypes;
    size_t m_entitySize = 0;
    size_t m_chunkCapacity = 0;
    size_t m_entityCount = 0;
    size_t m_entityAliveCount = 0;
    template <typename T> bool HasType();
    bool HasType(const size_t &typeId);
    DataComponentChunkArray m_chunkArray;
    DataComponentStorage() = default;
    DataComponentStorage(const EntityArchetypeInfo &entityArchetypeInfo);
};

struct EntityQueryInfo
{
    std::vector<DataComponentType> m_allDataComponentTypes;
    std::vector<DataComponentType> m_anyDataComponentTypes;
    std::vector<DataComponentType> m_noneDataComponentTypes;
    std::vector<DataComponentStorage *> m_queriedStorage;
};
#pragma endregion
#pragma endregion
#pragma endregion
template <typename T> bool EntityArchetypeInfo::HasType()
{
    for (const auto &i : m_dataComponentTypes)
    {
        if (i.m_typeId == typeid(T).hash_code())
            return true;
    }
    return false;
}
template <typename T> bool DataComponentStorage::HasType()
{
    for (const auto &i : m_dataComponentTypes)
    {
        if (i.m_typeId == typeid(T).hash_code())
            return true;
    }
    return false;
}

} // namespace UniEngine