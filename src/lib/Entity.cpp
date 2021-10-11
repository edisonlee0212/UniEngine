#include <Entity.hpp>
#include <EntityManager.hpp>
#include <ISerializable.hpp>
#include <EntityMetadata.hpp>
#include <IPrivateComponent.hpp>
#include <AssetManager.hpp>
using namespace UniEngine;

DataComponentType::DataComponentType(const std::string &name, const size_t &id, const size_t &size)
{
    m_name = name;
    m_typeId = id;
    m_size = size;
    m_offset = 0;
}

bool DataComponentType::operator==(const DataComponentType &other) const
{
    return (other.m_typeId == m_typeId) && (other.m_size == m_size);
}

bool DataComponentType::operator!=(const DataComponentType &other) const
{
    return (other.m_typeId != m_typeId) || (other.m_size != m_size);
}

bool Entity::operator==(const Entity &other) const
{
    return (other.m_index == m_index) && (other.m_version == m_version);
}

bool Entity::operator!=(const Entity &other) const
{
    return (other.m_index != m_index) || (other.m_version != m_version);
}

size_t Entity::operator()(Entity const &key) const
{
    return static_cast<size_t>(m_index);
}

bool Entity::IsEnabled() const
{
    assert(IsValid());
    return EntityManager::GetCurrentScene()->m_sceneDataStorage.m_entityInfos.at(m_index).m_enabled;
}

void Entity::SetEnabled(const bool &value) const
{
    EntityManager::SetEnable(EntityManager::GetCurrentScene(), *this, value);
}

void Entity::SetEnabledSingle(const bool &value) const
{
    EntityManager::SetEnableSingle(EntityManager::GetCurrentScene(), *this, value);
}

bool Entity::IsNull() const
{
    return m_index == 0;
}

bool Entity::IsValid() const
{
    auto& storage = EntityManager::GetCurrentScene()->m_sceneDataStorage.m_entities;
    return m_index != 0 && m_version != 0 && m_index < storage.size() && storage.at(m_index).m_version == m_version;
}

void Entity::SetParent(const Entity &parent, const bool &recalculateTransform) const
{
    EntityManager::SetParent(EntityManager::GetCurrentScene(), *this, parent, recalculateTransform);
}

std::string Entity::GetName() const
{
    return EntityManager::GetEntityName(EntityManager::GetCurrentScene(), *this);
}

void Entity::SetName(const std::string &name) const
{
    return EntityManager::SetEntityName(EntityManager::GetCurrentScene(), *this, name);
}

Entity Entity::GetParent() const
{
    return EntityManager::GetParent(EntityManager::GetCurrentScene(), *this);
}
template <typename T> void Entity::RemoveDataComponent() const
{
    EntityManager::RemoveDataComponent(EntityManager::GetCurrentScene(), *this);
}

Entity Entity::GetRoot() const
{
    return EntityManager::GetRoot(EntityManager::GetCurrentScene(), *this);
}
size_t Entity::GetChildrenAmount() const
{
    return EntityManager::GetChildrenAmount(EntityManager::GetCurrentScene(), *this);
}
std::vector<Entity> Entity::GetChildren() const
{
    return std::move(EntityManager::GetChildren(EntityManager::GetCurrentScene(), *this));
}
Entity Entity::GetChild(int index) const
{
    return std::move(EntityManager::GetChild(EntityManager::GetCurrentScene(), *this, index));
}
void Entity::ForEachChild(const std::function<void(const std::shared_ptr<Scene> &scene, Entity child)> &func) const
{
    EntityManager::ForEachChild(EntityManager::GetCurrentScene(), *this, func);
}

void Entity::RemoveChild(const Entity &child) const
{
    EntityManager::RemoveChild(EntityManager::GetCurrentScene(), child, *this);
}
std::vector<Entity> Entity::GetDescendants() const
{
    return std::move(EntityManager::GetDescendants(EntityManager::GetCurrentScene(), *this));
}
void Entity::ForEachDescendant(const std::function<void(const std::shared_ptr<Scene> &, const Entity &)> &func, const bool &fromRoot) const
{
    EntityManager::ForEachDescendant(EntityManager::GetCurrentScene(), *this, func, fromRoot);
}
unsigned Entity::GetIndex() const
{
    return m_index;
}
unsigned Entity::GetVersion() const
{
    return m_version;
}
Handle Entity::GetHandle() const
{
    auto& storage = EntityManager::GetCurrentScene()->m_sceneDataStorage.m_entityInfos;
    return storage.at(m_index).GetHandle();
}

IDataComponent *ComponentDataChunk::GetDataPointer(const size_t &offset) const
{
    return reinterpret_cast<IDataComponent *>(static_cast<char *>(m_data) + offset);
}

void ComponentDataChunk::SetData(const size_t &offset, const size_t &size, IDataComponent *data) const
{
    memcpy(static_cast<void *>(static_cast<char *>(m_data) + offset), data, size);
}

void ComponentDataChunk::ClearData(const size_t &offset, const size_t &size) const
{
    memset(static_cast<void *>(static_cast<char *>(m_data) + offset), 0, size);
}

ComponentDataChunk &ComponentDataChunk::operator=(const ComponentDataChunk &source){
    m_data = static_cast<void *>(calloc(1, EntityManager::GetArchetypeChunkSize()));
    memcpy(m_data, source.m_data, EntityManager::GetArchetypeChunkSize());
    return *this;
}

bool EntityArchetype::IsNull() const
{
    return m_index == 0;
}

bool EntityArchetype::IsValid() const
{
    return m_index != 0 && EntityManager::GetInstance().m_entityArchetypeInfos.size() > m_index;
}

std::string EntityArchetype::GetName() const
{
    return EntityManager::GetEntityArchetypeName(*this);
}
void EntityArchetype::SetName(const std::string &name) const
{
    EntityManager::SetEntityArchetypeName(*this, name);
}
size_t EntityArchetype::GetIndex()
{
    return m_index;
}



bool EntityArchetypeInfo::HasType(const size_t &typeId)
{
    for (const auto &type : m_dataComponentTypes)
    {
        if (typeId == type.m_typeId)
            return true;
    }
    return false;
}

bool EntityQuery::operator==(const EntityQuery &other) const
{
    return other.m_index == m_index;
}

bool EntityQuery::operator!=(const EntityQuery &other) const
{
    return other.m_index != m_index;
}

size_t EntityQuery::operator()(const EntityQuery &key) const
{
    return m_index;
}

bool EntityQuery::IsNull() const
{
    return m_index == 0;
}
size_t EntityQuery::GetIndex()
{
    return m_index;
}
bool EntityQuery::IsValid() const
{
    return m_index != 0 && EntityManager::GetInstance().m_entityQueryInfos.size() > m_index;
}

DataComponentStorage::DataComponentStorage(const EntityArchetypeInfo &entityArchetypeInfo)
{
    m_dataComponentTypes = entityArchetypeInfo.m_dataComponentTypes;
    m_entitySize = entityArchetypeInfo.m_entitySize;
    m_chunkCapacity = entityArchetypeInfo.m_chunkCapacity;
}

DataComponentStorage &DataComponentStorage::operator=(const DataComponentStorage &source)
{
    m_dataComponentTypes = source.m_dataComponentTypes;
    m_entitySize = source.m_entitySize;
    m_chunkCapacity = source.m_chunkCapacity;
    m_entityCount = source.m_entityCount;
    m_entityAliveCount = source.m_entityAliveCount;
    m_chunkArray = source.m_chunkArray;
    return *this;
}

bool DataComponentStorage::HasType(const size_t &typeId)
{
    for (const auto &type : m_dataComponentTypes)
    {
        if (typeId == type.m_typeId)
            return true;
    }
    return false;
}


void EntityRef::Set(const Entity &target)
{
    if(target.IsNull())
    {
        Clear();
    }
    else {
        m_entityHandle = target.GetHandle();
        m_value = target;
    }
}
void EntityRef::Clear()
{
    m_value = Entity();
    m_entityHandle = Handle(0);
}
void EntityRef::Update()
{
    if(m_entityHandle.GetValue() == 0){
        Clear();
        return;
    }else if(m_value.IsNull()){
        auto scene = EntityManager::GetCurrentScene();
        if(!scene) Clear();
        else
        {
            m_value = EntityManager::GetEntity(scene, m_entityHandle);
            if (m_value.IsNull())
            {
                Clear();
            }
        }
    }
    if(!m_value.IsValid()){
        Clear();
    }
}

DataComponentChunkArray &DataComponentChunkArray::operator=(const DataComponentChunkArray &source)
{
    m_entities = source.m_entities;
    m_chunks.resize(source.m_chunks.size());
    for(int i = 0; i < m_chunks.size(); i++) m_chunks[i] = source.m_chunks[i];
    return *this;
}
