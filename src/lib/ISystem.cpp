#include <ISystem.hpp>
using namespace UniEngine;

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

float ISystem::GetRank()
{
    return m_rank;
}