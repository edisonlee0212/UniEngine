#include <SystemBase.hpp>
using namespace UniEngine;

void SystemBase::OnStartRunning()
{
}

void SystemBase::OnStopRunning()
{
}

SystemBase::SystemBase()
{
    m_enabled = false;
}

void SystemBase::Enable()
{
    if (!m_enabled)
    {
        m_enabled = true;
        OnStartRunning();
    }
}

void SystemBase::Disable()
{
    if (m_enabled)
    {
        m_enabled = false;
        OnStopRunning();
    }
}

bool SystemBase::Enabled() const
{
    return m_enabled;
}

void SystemBase::OnCreate()
{
}

void SystemBase::OnDestroy()
{
}

void SystemBase::PreUpdate()
{
}

void SystemBase::Update()
{
}

void SystemBase::FixedUpdate()
{
}

void SystemBase::LateUpdate()
{
}
