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
    return GetOwner()->m_sceneDataStorage.m_entityInfos.at(m_index).m_enabled;
}

void Entity::SetEnabled(const bool &value) const
{
    EntityManager::SetEnable(GetOwner(), *this, value);
}

void Entity::SetEnabledSingle(const bool &value) const
{
    EntityManager::SetEnableSingle(GetOwner(), *this, value);
}

bool Entity::IsNull() const
{
    return m_index == 0;
}

bool Entity::IsValid() const
{
    auto& storage = GetOwner()->m_sceneDataStorage.m_entities;
    return m_index != 0 && m_version != 0 && m_index < storage.size() && storage.at(m_index).m_version == m_version;
}

void Entity::SetParent(const Entity &parent, const bool &recalculateTransform) const
{
    EntityManager::SetParent(GetOwner(), *this, parent, recalculateTransform);
}

std::string Entity::GetName() const
{
    return EntityManager::GetEntityName(GetOwner(), *this);
}

void Entity::SetName(const std::string &name) const
{
    return EntityManager::SetEntityName(GetOwner(), *this, name);
}

Entity Entity::GetParent() const
{
    return EntityManager::GetParent(GetOwner(), *this);
}
template <typename T> void Entity::RemoveDataComponent() const
{
    EntityManager::RemoveDataComponent(GetOwner(), *this);
}

Entity Entity::GetRoot() const
{
    return EntityManager::GetRoot(GetOwner(), *this);
}
size_t Entity::GetChildrenAmount() const
{
    return EntityManager::GetChildrenAmount(GetOwner(), *this);
}
std::vector<Entity> Entity::GetChildren() const
{
    return std::move(EntityManager::GetChildren(GetOwner(), *this));
}
Entity Entity::GetChild(int index) const
{
    return std::move(EntityManager::GetChild(GetOwner(), *this, index));
}
void Entity::ForEachChild(const std::function<void(Entity)> &func) const
{
    EntityManager::ForEachChild(GetOwner(), *this, func);
}

void Entity::RemoveChild(const Entity &child) const
{
    EntityManager::RemoveChild(GetOwner(), child, *this);
}
std::vector<Entity> Entity::GetDescendants() const
{
    return std::move(EntityManager::GetDescendants(GetOwner(), *this));
}
void Entity::ForEachDescendant(const std::function<void(const Entity &)> &func, const bool &fromRoot) const
{
    EntityManager::ForEachDescendant(GetOwner(), *this, func, fromRoot);
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
    auto& storage = GetOwner()->m_sceneDataStorage.m_entityInfos;
    return storage.at(m_index).GetHandle();
}
Handle Entity::GetSceneHandle() const
{
    return m_sceneHandle;
}
std::shared_ptr<Scene> Entity::GetOwner() const
{
    auto currentScene = EntityManager::GetCurrentScene();
    if(currentScene->GetHandle().GetValue() == m_sceneHandle.GetValue()) return currentScene;
    return AssetManager::Get<Scene>(m_sceneHandle);
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
    if(target.IsNull()) m_entityHandle = Handle(0);
    else m_entityHandle = target.GetHandle();
    m_value = target;
}
void EntityRef::Clear()
{
    m_value = Entity();
    m_entityHandle = Handle(0);
    m_sceneHandle = Handle(0);
}
void EntityRef::Update()
{
    if(m_entityHandle.GetValue() == 0){
        Clear();
        return;
    }else if(m_value.IsNull()){
        auto scene = EntityManager::GetCurrentScene();
        if(scene->GetHandle().GetValue() != m_sceneHandle.GetValue()){
            scene = AssetManager::Get<Scene>(m_sceneHandle);
        }
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
