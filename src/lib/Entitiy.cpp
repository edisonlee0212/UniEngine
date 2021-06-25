#include <EntityManager.hpp>
using namespace UniEngine;

ComponentDataType::ComponentDataType(const std::string &name, const size_t &id, const size_t &size)
{
    m_name = name;
    m_typeId = id;
    m_size = size;
    m_offset = 0;
}

bool ComponentDataType::operator==(const ComponentDataType &other) const
{
    return (other.m_typeId == m_typeId) && (other.m_size == m_size);
}

bool ComponentDataType::operator!=(const ComponentDataType &other) const
{
    return (other.m_typeId != m_typeId) || (other.m_size != m_size);
}

Entity::Entity()
{
    m_index = 0;
    m_version = 0;
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
    return EntityManager::IsEntityEnabled(*this);
}

void Entity::SetStatic(const bool &value) const
{
    EntityManager::SetStatic(*this, value);
}

void Entity::SetEnabled(const bool &value) const
{
    EntityManager::SetEnable(*this, value);
}

void Entity::SetEnabledSingle(const bool &value) const
{
    EntityManager::SetEnableSingle(*this, value);
}

bool Entity::IsNull() const
{
    return m_index == 0;
}

bool Entity::IsStatic() const
{
    return EntityManager::IsEntityStatic(*this);
}

bool Entity::IsDeleted() const
{
    return EntityManager::IsEntityDeleted(m_index);
}

bool Entity::IsValid() const
{
    if (!IsNull() && EntityManager::IsEntityValid(*this))
        return true;
    return false;
}

void Entity::SetParent(const Entity &parent, const bool& recalculateTransform) const
{
    EntityManager::SetParent(*this, parent, recalculateTransform);
}

std::string Entity::GetName() const
{
    return EntityManager::GetEntityName(*this);
}

void Entity::SetName(const std::string &name) const
{
    return EntityManager::SetEntityName(*this, name);
}

Entity PrivateComponentBase::GetOwner() const
{
    return m_owner;
}

void PrivateComponentBase::SetEnabled(const bool &value)
{
    if (m_enabled != value)
    {
        if (value)
        {
            OnEnable();
        }
        else
        {
            OnDisable();
        }
        m_enabled = value;
    }
}

bool PrivateComponentBase::IsEnabled() const
{
    return m_enabled;
}

void PrivateComponentBase::Init()
{
}

void PrivateComponentBase::OnEnable()
{
}

void PrivateComponentBase::OnDisable()
{
}

void PrivateComponentBase::OnEntityEnable()
{
}

void PrivateComponentBase::OnEntityDisable()
{
}

void PrivateComponentBase::OnGui()
{
}

ComponentDataBase *ComponentDataChunk::GetDataPointer(const size_t &offset) const
{
    return reinterpret_cast<ComponentDataBase *>(static_cast<char *>(m_data) + offset);
}

void ComponentDataChunk::SetData(const size_t &offset, const size_t &size, ComponentDataBase *data) const
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
    return EntityManager::IsEntityArchetypeValid(*this);
}

std::string EntityArchetype::GetName() const
{
    return EntityManager::GetEntityArchetypeName(*this);
}

PrivateComponentElement::PrivateComponentElement(
    const std::string &name, const size_t &id, std::unique_ptr<PrivateComponentBase> data, const Entity &owner)
{
    m_name = name;
    m_typeId = id;
    m_privateComponentData = std::move(data);
    m_privateComponentData->m_owner = owner;
    m_privateComponentData->Init();
}

void PrivateComponentElement::ResetOwner(const Entity &newOwner) const
{
    m_privateComponentData->m_owner = newOwner;
}

bool EntityArchetypeInfo::HasType(const size_t &typeId)
{
    for (const auto &type : m_componentTypes)
    {
        if (typeId == type.m_typeId)
            return true;
    }
    return false;
}

EntityQuery::EntityQuery()
{
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

EntityComponentDataStorage::EntityComponentDataStorage()
{
}
