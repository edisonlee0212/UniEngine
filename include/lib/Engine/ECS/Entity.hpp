#pragma once
#include "IHandle.hpp"
#include "ISerializable.hpp"
#include <IDataComponent.hpp>
namespace UniEngine
{
#pragma region EntityManager
#pragma region Entity
class Scene;
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
    friend class Entities;
    friend class Serialization;
    friend class Scene;
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
    friend class Entities;
    friend class Scene;
    friend class EntityMetadata;
    friend class Serialization;
    unsigned m_index = 0;
    unsigned m_version = 0;
  public:
    [[nodiscard]] unsigned GetIndex() const;
    [[nodiscard]] unsigned GetVersion() const;
    bool operator==(const Entity &other) const;
    bool operator!=(const Entity &other) const;
    size_t operator()(Entity const &key) const;
};
#pragma region Storage

class UNIENGINE_API EntityRef final
{
    Entity m_value = Entity();
    Handle m_entityHandle = Handle(0);
    void Update();

  public:
    void Serialize(YAML::Emitter &out) const
    {
        out << YAML::Key << "m_entityHandle" << YAML::Value << m_entityHandle;
    }
    void Deserialize(const YAML::Node &in)
    {
        m_entityHandle = Handle(in["m_entityHandle"].as<uint64_t>());
    }
    EntityRef()
    {
        m_entityHandle = Handle(0);
        m_value = Entity();
    }
    EntityRef(const Entity &other)
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
        if (m_entityHandle.GetValue() == 0)
            return;
        auto search = map.find(m_entityHandle);
        if (search != map.end())
        {
            m_entityHandle = search->second;
            m_value = Entity();
        }
        else
        {
            Clear();
        }
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

    void Save(const std::string &name, YAML::Emitter &out)
    {
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string &name, const YAML::Node &in)
    {
        Deserialize(in[name]);
    }
};

UNIENGINE_API inline void SaveList(const std::string& name, const std::vector<EntityRef>& target, YAML::Emitter &out){
    if(target.empty()) return;
    out << YAML::Key << name << YAML::Value << YAML::BeginSeq;
    for (auto &i: target) {
        out << YAML::BeginMap;
        i.Serialize(out);
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}
UNIENGINE_API inline void LoadList(const std::string& name, std::vector<EntityRef>& target, const YAML::Node &in){
    if(in[name]){
        target.clear();
        for(const auto& i : in[name]){
            EntityRef instance;
            instance.Deserialize(i);
            target.push_back(instance);
        }
    }
}

const size_t ARCHETYPE_CHUNK_SIZE = 16384;

struct UNIENGINE_API ComponentDataChunk
{
    void *m_data;
    template <typename T> T GetData(const size_t &offset);
    [[nodiscard]] IDataComponent *GetDataPointer(const size_t &offset) const;
    template <typename T> void SetData(const size_t &offset, const T &data);
    void SetData(const size_t &offset, const size_t &size, IDataComponent *data) const;
    void ClearData(const size_t &offset, const size_t &size) const;

    ComponentDataChunk &operator=(const ComponentDataChunk &source);
};

struct UNIENGINE_API DataComponentChunkArray
{
    std::vector<Entity> m_entities;
    std::vector<ComponentDataChunk> m_chunks;
    DataComponentChunkArray &operator=(const DataComponentChunkArray &source);
};

struct UNIENGINE_API EntityArchetypeInfo
{
    std::string m_name = "New Entity Archetype";
    size_t m_entitySize = 0;
    size_t m_chunkCapacity = 0;
    std::vector<DataComponentType> m_dataComponentTypes;
    template <typename T> bool HasType();
    bool HasType(const size_t &typeId);
};

struct UNIENGINE_API EntityQuery final
{
  private:
    friend class Entities;
    friend class Scene;
    friend class Serialization;
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
    DataComponentStorage &operator=(const DataComponentStorage &source);
};

struct EntityQueryInfo
{
    size_t m_index = 0;
    std::vector<DataComponentType> m_allDataComponentTypes;
    std::vector<DataComponentType> m_anyDataComponentTypes;
    std::vector<DataComponentType> m_noneDataComponentTypes;
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