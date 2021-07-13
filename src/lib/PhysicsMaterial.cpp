#include "PhysicsMaterial.hpp"
#include <PhysicsManager.hpp>
void UniEngine::PhysicsMaterial::OnCreate()
{
    m_name = "New physics model";
    m_value =
        PhysicsManager::GetInstance().m_physics->createMaterial(m_staticFriction, m_dynamicFriction, m_restitution);
}
UniEngine::PhysicsMaterial::~PhysicsMaterial()
{
    if (m_value)
        m_value->release();
}
void UniEngine::PhysicsMaterial::SetDynamicFriction(const float &value)
{
    if (m_dynamicFriction != value)
    {
        m_dynamicFriction = value;
        m_value->setDynamicFriction(m_dynamicFriction);
    }
}
void UniEngine::PhysicsMaterial::SetStaticFriction(const float &value)
{
    if (m_staticFriction != value)
    {
        m_staticFriction = value;
        m_value->setStaticFriction(m_staticFriction);
    }
}
void UniEngine::PhysicsMaterial::SetRestitution(const float &value)
{
    if (m_restitution != value)
    {
        m_restitution = value;
        m_value->setRestitution(m_restitution);
    }
}
void UniEngine::PhysicsMaterial::OnGui()
{
    if (ImGui::DragFloat("Dynamic Friction", &m_dynamicFriction))
    {
        SetDynamicFriction(m_dynamicFriction);
    }
    if (ImGui::DragFloat("Static Friction", &m_staticFriction))
    {
        SetStaticFriction(m_staticFriction);
    }
    if (ImGui::DragFloat("Restitution", &m_restitution))
    {
        SetRestitution(m_restitution);
    }
}
