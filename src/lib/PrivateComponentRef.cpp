//
// Created by lllll on 10/10/2021.
//

#include "PrivateComponentRef.hpp"
#include <AssetManager.hpp>
#include <EntityManager.hpp>
using namespace UniEngine;
bool PrivateComponentRef::Update()
{
    if (m_entityHandle.GetValue() == 0 && m_sceneHandle.GetValue() == 0)
    {
        m_value.reset();
        return false;
    }
    else if (!m_value.has_value() || m_value->expired())
    {
        auto scene = EntityManager::GetCurrentScene();
        if (scene->GetHandle().GetValue() != m_sceneHandle.GetValue())
        {
            scene = AssetManager::Get<Scene>(m_sceneHandle);
        }
        if (scene)
        {
            auto entity = EntityManager::GetEntity(scene, m_entityHandle);
            if (!entity.IsNull())
            {
                if (EntityManager::HasPrivateComponent(entity, m_privateComponentTypeName))
                {
                    m_value = EntityManager::GetPrivateComponent(entity, m_privateComponentTypeName);
                    return true;
                }
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
    m_sceneHandle = Handle(0);
}