#include <Entity.hpp>
#include <EntityManager.hpp>

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

void Entity::SetParent(const Entity &parent, const bool &recalculateTransform) const
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

Entity Entity::GetParent() const
{
    return EntityManager::GetParent(*this);
}
template <typename T> void Entity::RemoveDataComponent() const
{
    EntityManager::RemoveDataComponent(*this);
}
EntityArchetype Entity::GetEntityArchetype() const
{
    return EntityManager::GetEntityArchetype(*this);
}

Entity Entity::GetRoot() const
{
    return EntityManager::GetRoot(*this);
}
size_t Entity::GetChildrenAmount() const
{
    return EntityManager::GetChildrenAmount(*this);
}
std::vector<Entity> Entity::GetChildren() const
{
    return std::move(EntityManager::GetChildren(*this));
}

void Entity::ForEachChild(const std::function<void(Entity)> &func) const
{
    EntityManager::ForEachChild(*this, func);
}

void Entity::RemoveChild(const Entity &child) const
{
    EntityManager::RemoveChild(*this, child);
}
std::vector<Entity> Entity::GetDescendants() const
{
    return std::move(EntityManager::GetDescendants(*this));
}
void Entity::ForEachDescendant(const std::function<void(const Entity &)> &func, const bool &fromRoot) const
{
    EntityManager::ForEachDescendant(*this, func, fromRoot);
}
unsigned Entity::GetIndex() const
{
    return m_index;
}
unsigned Entity::GetVersion() const
{
    return m_version;
}

Entity IPrivateComponent::GetOwner() const
{
    return m_owner;
}

void IPrivateComponent::SetEnabled(const bool &value)
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

bool IPrivateComponent::IsEnabled() const
{
    return m_enabled;
}

void IPrivateComponent::OnCreate()
{
}

void IPrivateComponent::OnEnable()
{
}

void IPrivateComponent::OnDisable()
{
}

void IPrivateComponent::OnEntityEnable()
{
}

void IPrivateComponent::OnEntityDisable()
{
}

void IPrivateComponent::OnGui()
{
}
void IPrivateComponent::OnDestroy()
{
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
    return EntityManager::IsEntityArchetypeValid(*this);
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

PrivateComponentElement::PrivateComponentElement(
    const std::string &name, const size_t &id, IPrivateComponent * data, const Entity &owner)
{
    m_name = name;
    m_typeId = id;
    m_privateComponentData = data;
    m_privateComponentData->m_owner = owner;
    m_privateComponentData->OnCreate();
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

DataComponentStorage::DataComponentStorage() = default;
