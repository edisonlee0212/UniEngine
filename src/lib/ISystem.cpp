#include "ISystem.hpp"
#include "Entities.hpp"
#include "Application.hpp"
#include "Scene.hpp"
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
std::shared_ptr<Scene> ISystem::GetScene() const
{
    return m_scene.lock();
}
bool ISystem::Enabled() const
{
    return m_enabled;
}

float ISystem::GetRank()
{
    return m_rank;
}

bool SystemRef::Update()
{
    if (m_systemHandle.GetValue() == 0)
    {
        m_value.reset();
        return false;
    }
    else if(!m_value.has_value() || m_value->expired())
    {
        auto currentScene = Application::GetActiveScene();
        auto system = currentScene->m_mappedSystems.find(m_systemHandle);
        if(system != currentScene->m_mappedSystems.end()){
            m_value = system->second;
            return true;
        }
        Clear();
        return false;
    }
    return true;
}