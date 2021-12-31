#include <Application.hpp>
#include <ProfilerLayer.hpp>
using namespace UniEngine;
CPUUsageEvent::CPUUsageEvent(CPUUsageEvent *parent, const std::string &name)
{
    m_parent = parent;
    m_name = name;
    m_timeStart = Application::Time().CurrentTime();
}

void CPUUsageEvent::OnInspect(const float &parentTotalTime) const
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

void CPUTimeProfiler::PreUpdate()
{
    if (m_currentEventPointer != &m_rootEvent)
    {
        UNIENGINE_ERROR("Event not properly registered!");
    }
    m_rootEvent = CPUUsageEvent(nullptr, "Main Loop");
    m_currentEventPointer = &m_rootEvent;
}

void CPUTimeProfiler::StartEvent(const std::string &name)
{
    m_currentEventPointer->m_children.emplace_back(m_currentEventPointer, name);
    m_currentEventPointer = &m_currentEventPointer->m_children.back();
}

void CPUTimeProfiler::EndEvent(const std::string &name)
{
    if (name != m_currentEventPointer->m_name)
    {
        UNIENGINE_ERROR("Event not properly ended!");
    }
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
    m_currentEventPointer = m_currentEventPointer->m_parent;
}

void CPUTimeProfiler::LateUpdate()
{
    m_currentEventPointer->m_timeEnd = Application::Time().CurrentTime();
}

void CPUTimeProfiler::OnInspect()
{
    auto time = m_rootEvent.m_timeEnd - m_rootEvent.m_timeStart;
    if(time < 0.0f) ImGui::Text("No frame recorded!");
    else m_rootEvent.OnInspect(m_rootEvent.m_timeEnd - m_rootEvent.m_timeStart);
}

void ProfilerLayer::PreUpdate()
{
    m_record = Application::IsPlaying();
    if (!m_record)
        return;
    for (auto &i : m_profilers)
        i.second->PreUpdate();
}

void ProfilerLayer::OnInspect()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Profiler", &m_gui);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_gui)
    {
        ImGui::Begin("Profiler");
        if (m_record)
        {
            ImGui::Text("Pause Game to view results");
        }
        else
        {
            for (auto &i : m_profilers)
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
void ProfilerLayer::OnCreate()
{
    GetOrCreateProfiler<CPUTimeProfiler>("CPU Time");
}
void ProfilerLayer::StartEvent(const std::string &name)
{
    auto profilerLayer = Application::GetLayer<ProfilerLayer>();
    if(profilerLayer)
    {
        if (!profilerLayer->m_record)
            return;
        for (auto &i : profilerLayer->m_profilers)
            i.second->StartEvent(name);
    }
}
void ProfilerLayer::EndEvent(const std::string &name)
{
    auto profilerLayer = Application::GetLayer<ProfilerLayer>();
    if(profilerLayer)
    {
        if (!profilerLayer->m_record)
            return;
        for (auto &i : profilerLayer->m_profilers)
            i.second->EndEvent(name);
    }
}
void ProfilerLayer::LateUpdate()
{
    if (!m_record)
        return;
    for (auto &i : m_profilers)
        i.second->LateUpdate();
}
