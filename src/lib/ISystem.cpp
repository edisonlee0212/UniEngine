#include <ISystem.hpp>
using namespace UniEngine;

void ISystem::OnEnable()
{
}

void ISystem::OnDisable()
{
}

void ISystem::OnGui()
{
}

ISystem::ISystem()
{
    m_enabled = false;
}

void ISystem::Enable()
{
    if (!m_enabled)
    {
        m_enabled = true;
        OnEnable();
    }
}

void ISystem::Disable()
{
    if (m_enabled)
    {
        m_enabled = false;
        OnDisable();
    }
}

bool ISystem::Enabled() const
{
    return m_enabled;
}

void ISystem::OnCreate()
{
}

void ISystem::OnDestroy()
{
}

void ISystem::PreUpdate()
{
}

void ISystem::Update()
{
}

void ISystem::FixedUpdate()
{
}

void ISystem::LateUpdate()
{
}

float ISystem::GetRank()
{
    return m_rank;
}