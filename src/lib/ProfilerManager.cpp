#include <Application.hpp>
#include <ProfilerManager.hpp>

UniEngine::CPUUsageEvent::CPUUsageEvent(CPUUsageEvent *parent, const std::string &name)
{
    m_parent = parent;
    m_name = name;
    m_timeStart = Application::Time().CurrentTime();
}

void UniEngine::CPUUsageEvent::OnGui(const float &parentTotalTime) const
{
    const float time = m_timeEnd - m_timeStart;
    if (ImGui::TreeNode(m_name.c_str()))
    {
        ImGui::SameLine();
        ImGui::Text("Time: %.4f ms (%.3f\%)", time, time / parentTotalTime * 100.0f);
        for (auto &i : m_children)
            i.OnGui(time);
        ImGui::TreePop();
    }
}

void UniEngine::EngineProfiler::PreUpdate()
{
    if (m_currentEventPointer != &m_rootEvent)
    {
        UNIENGINE_ERROR("Event not properly registered!");
    }
    m_rootEvent = CPUUsageEvent(nullptr, "EngineLoop");
    m_currentEventPointer = &m_rootEvent;
}

void UniEngine::EngineProfiler::StartEvent(const std::string &name)
{
    m_currentEventPointer->m_children.emplace_back(m_currentEventPointer, name);
    m_currentEventPointer = &m_currentEventPointer->m_children.back();
}

void UniEngine::EngineProfiler::EndEvent(const std::string &name)
{
    if (name != m_currentEventPointer->m_name)
    {
        UNIENGINE_ERROR("Event not properly ended!");
    }
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
    m_currentEventPointer = m_currentEventPointer->m_parent;
}

void UniEngine::EngineProfiler::LateUpdate()
{
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
}

void UniEngine::EngineProfiler::OnGui()
{
    m_rootEvent.OnGui(m_rootEvent.m_timeEnd - m_rootEvent.m_timeStart);
}

UniEngine::EngineProfiler &UniEngine::ProfilerManager::GetEngineProfiler()
{
    return GetInstance().m_engineProfiler;
}

void UniEngine::ProfilerManager::PreUpdate()
{
    auto &profilerManager = GetInstance();
    profilerManager.m_engineProfiler.PreUpdate();
}

void UniEngine::ProfilerManager::LateUpdate()
{
    auto &profilerManager = GetInstance();
    profilerManager.m_engineProfiler.LateUpdate();
}
void UniEngine::ProfilerManager::OnGui()
{
    auto &profilerManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Profiler Manager", &profilerManager.m_gui);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (profilerManager.m_gui)
    {
        ImGui::Begin("Profiler Manager");
        if (ImGui::CollapsingHeader("Engine Profiler"))
        {
            profilerManager.m_engineProfiler.OnGui();
        }
        ImGui::End();
    }
}
