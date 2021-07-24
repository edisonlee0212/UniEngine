#include <ISystem.hpp>
using namespace UniEngine;

void ISystem::OnStartRunning()
{
}

void ISystem::OnStopRunning()
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
        OnStartRunning();
    }
}

void ISystem::Disable()
{
    if (m_enabled)
    {
        m_enabled = false;
        OnStopRunning();
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
std::shared_ptr<Scene> ISystem::GetOwner()
{
    return m_scene;
}
float ISystem::GetRank()
{
    return m_rank;
}