//
// Created by lllll on 8/13/2021.
//
#include "IPrivateComponent.hpp"
#include <AssetManager.hpp>
#include <EntityManager.hpp>
using namespace UniEngine;
PrivateComponentElement::PrivateComponentElement(
    size_t id, const std::shared_ptr<IPrivateComponent> &data, const Entity &owner)
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

bool IPrivateComponent::Started()
{
    return m_started;
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
