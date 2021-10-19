//
// Created by lllll on 10/10/2021.
//

#include "PrivateComponentRef.hpp"
#include <AssetManager.hpp>
#include <EntityManager.hpp>
using namespace UniEngine;
bool PrivateComponentRef::Update()
{
    if (m_entityHandle.GetValue() == 0 || m_scene.expired())
    {
        Clear();
        return false;
    }
    else if (!m_value.has_value() || m_value->expired())
    {
        auto scene = m_scene.lock();
        auto entity = EntityManager::GetEntity(scene, m_entityHandle);
        if (!entity.IsNull())
        {
            if (EntityManager::HasPrivateComponent(scene, entity, m_privateComponentTypeName))
            {
                m_value = EntityManager::GetPrivateComponent(scene, entity, m_privateComponentTypeName);
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
    m_scene.reset();
}
PrivateComponentRef &PrivateComponentRef::operator=(const PrivateComponentRef &other)
{
    m_entityHandle = other.m_entityHandle;
    m_privateComponentTypeName = other.m_privateComponentTypeName;
    m_scene = other.m_scene;
    return *this;
}
