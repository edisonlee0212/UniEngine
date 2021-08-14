//
// Created by lllll on 8/13/2021.
//
#include <EntityManager.hpp>
#include "IPrivateComponent.hpp"
using namespace UniEngine;
PrivateComponentElement::PrivateComponentElement(size_t id, const std::shared_ptr<IPrivateComponent> &data, const Entity &owner)
{
    m_typeId = id;
    m_privateComponentData = data;
    m_privateComponentData->m_owner = owner;
    m_privateComponentData->OnCreate();
}

void PrivateComponentElement::ResetOwner(const Entity &newOwner) const
{
    m_privateComponentData->m_owner = newOwner;
}


bool IPrivateComponent::IsEnabled() const
{
    return m_enabled;
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

bool PrivateComponentRef::Update()
{
    if (m_entityHandle.GetValue() == 0)
    {
        m_value.reset();
        return false;
    }
    else if(!m_value.has_value() || m_value->expired())
    {
        auto entity = EntityManager::GetEntity(m_entityHandle);
        if (!entity.IsNull())
        {
            if (EntityManager::HasPrivateComponent(entity, m_privateComponentTypeName))
            {
                m_value = EntityManager::GetPrivateComponent(entity, m_privateComponentTypeName);
                return true;
            }
        }
        Clear();
        return false;
    }
    return true;
}
void PrivateComponentRef::Clear()
{
    m_value.reset();
    m_entityHandle = Handle(0);
}