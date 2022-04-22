//
// Created by lllll on 10/10/2021.
//

#include "PrivateComponentRef.hpp"
#include "ProjectManager.hpp"
#include "Entities.hpp"
#include "Scene.hpp"
using namespace UniEngine;
bool PrivateComponentRef::Update()
{
    if (m_entityHandle.GetValue() == 0 || m_scene.expired())
    {
        Clear();
        return false;
    }
    if (m_value.expired())
    {
        auto scene = m_scene.lock();
        auto entity = scene->GetEntity(m_entityHandle);
        if (entity.GetIndex() != 0)
        {
            if (scene->HasPrivateComponent(entity, m_privateComponentTypeName))
            {
                m_value = scene->GetPrivateComponent(entity, m_privateComponentTypeName);
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
