#include "Entity.hpp"
#include "Entities.hpp"
#include "ISerializable.hpp"
#include "EntityMetadata.hpp"
#include "IPrivateComponent.hpp"
#include "ProjectManager.hpp"
#include "Application.hpp"
#include "Scene.hpp"
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

unsigned Entity::GetIndex() const
{
    return m_index;
}
unsigned Entity::GetVersion() const
{
    return m_version;
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
    m_data = static_cast<void *>(calloc(1, Entities::GetArchetypeChunkSize()));
    memcpy(m_data, source.m_data, Entities::GetArchetypeChunkSize());
    return *this;
}

bool EntityArchetype::IsNull() const
{
    return m_index == 0;
}

bool EntityArchetype::IsValid() const
{
    return m_index != 0 && Entities::GetInstance().m_entityArchetypeInfos.size() > m_index;
}

std::string EntityArchetype::GetName() const
{
    return Entities::GetEntityArchetypeName(*this);
}
void EntityArchetype::SetName(const std::string &name) const
{
    Entities::SetEntityArchetypeName(*this, name);
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
    return m_index != 0 && Entities::GetInstance().m_entityQueryInfos.size() > m_index;
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
    if(target.GetIndex() == 0)
    {
        Clear();
    }
    else {
        auto scene = Application::GetActiveScene();
        m_entityHandle = scene->GetEntityHandle(target);
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
    auto scene = Application::GetActiveScene();
    if(m_entityHandle.GetValue() == 0){
        Clear();
        return;
    }else if(m_value.GetIndex() == 0){
        if(!scene) Clear();
        else
        {
            m_value = scene->GetEntity(m_entityHandle);
            if (m_value.GetIndex() == 0)
            {
                Clear();
            }
        }
    }
    if(! scene->IsEntityValid(m_value)){
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
