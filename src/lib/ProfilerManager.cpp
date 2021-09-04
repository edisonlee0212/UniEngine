#include <Application.hpp>
#include <ProfilerManager.hpp>

UniEngine::CPUUsageEvent::CPUUsageEvent(CPUUsageEvent *parent, const std::string &name)
{
    m_parent = parent;
    m_name = name;
    m_timeStart = Application::Time().CurrentTime();
}

void UniEngine::CPUUsageEvent::OnInspect(const float &parentTotalTime) const
{
    const float time = m_timeEnd - m_timeStart;
    if (ImGui::TreeNode((m_name).c_str()))
    {
        ImGui::SameLine();
        ImGui::Text(": %.4f ms (%.3f%%)", time, time / parentTotalTime * 100.0f);
        for (auto &i : m_children)
            i.OnInspect(parentTotalTime);
        ImGui::TreePop();
    }
}

void UniEngine::CPUTimeProfiler::PreUpdate()
{
    if (m_currentEventPointer != &m_rootEvent)
    {
        UNIENGINE_ERROR("Event not properly registered!");
    }
    m_rootEvent = CPUUsageEvent(nullptr, "Main Loop");
    m_currentEventPointer = &m_rootEvent;
}

void UniEngine::CPUTimeProfiler::StartEvent(const std::string &name)
{
    m_currentEventPointer->m_children.emplace_back(m_currentEventPointer, name);
    m_currentEventPointer = &m_currentEventPointer->m_children.back();
}

void UniEngine::CPUTimeProfiler::EndEvent(const std::string &name)
{
    if (name != m_currentEventPointer->m_name)
    {
        UNIENGINE_ERROR("Event not properly ended!");
    }
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
    m_currentEventPointer = m_currentEventPointer->m_parent;
}

void UniEngine::CPUTimeProfiler::LateUpdate()
{
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
}

void UniEngine::CPUTimeProfiler::OnInspect()
{
    auto time = m_rootEvent.m_timeEnd - m_rootEvent.m_timeStart;
    if(time < 0.0f) ImGui::Text("No frame recorded!");
    else m_rootEvent.OnInspect(m_rootEvent.m_timeEnd - m_rootEvent.m_timeStart);
}

void UniEngine::ProfilerManager::PreUpdate()
{
    auto &profilerManager = GetInstance();
    profilerManager.m_record = Application::IsPlaying();
    if (!profilerManager.m_record)
        return;
    for (auto &i : profilerManager.m_profilers)
        i.second->PreUpdate();
}

void UniEngine::ProfilerManager::OnInspect()
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
        if (profilerManager.m_record)
        {
            ImGui::Text("Pause Game to view results");
        }
        else
        {
            for (auto &i : profilerManager.m_profilers)
            {
                if (ImGui::CollapsingHeader(i.second->m_name.c_str()))
                {
                    i.second->OnInspect();
                }
            }
        }
        ImGui::End();
    }
}
void UniEngine::ProfilerManager::StartEvent(const std::string &name)
{
    auto &profilerManager = GetInstance();
    if (!profilerManager.m_record)
        return;
    for (auto &i : profilerManager.m_profilers)
        i.second->StartEvent(name);
}
void UniEngine::ProfilerManager::EndEvent(const std::string &name)
{
    auto &profilerManager = GetInstance();
    if (!profilerManager.m_record)
        return;
    for (auto &i : profilerManager.m_profilers)
        i.second->EndEvent(name);
}
void UniEngine::ProfilerManager::LateUpdate()
{
    auto &profilerManager = GetInstance();
    if (!profilerManager.m_record)
        return;
    for (auto &i : profilerManager.m_profilers)
        i.second->LateUpdate();
}
