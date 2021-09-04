#include <Application.hpp>
#include <ConsoleManager.hpp>
using namespace UniEngine;

void ConsoleManager::Log(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Log;
    cm.m_time = Application::Time().CurrentTime();
    instance.m_consoleMessages.push_back(cm);
}

void ConsoleManager::Error(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Error;
    cm.m_time = Application::Time().CurrentTime();
    instance.m_consoleMessages.push_back(cm);
}

void ConsoleManager::Warning(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Warning;
    cm.m_time = Application::Time().CurrentTime();
    instance.m_consoleMessages.push_back(cm);
}

std::vector<ConsoleMessage> &ConsoleManager::GetConsoleMessages()
{
    return GetInstance().m_consoleMessages;
}

void ConsoleManager::OnInspect()
{
    auto& consoleManager = GetInstance();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Console", &consoleManager.m_enableConsole);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Console");
    ImGui::Checkbox("Log", &consoleManager.m_enableConsoleLogs);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &consoleManager.m_enableConsoleWarnings);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &consoleManager.m_enableConsoleErrors);
    ImGui::SameLine();
    if(ImGui::Button("Clear all")){
        consoleManager.m_consoleMessages.clear();
    }
    int i = 0;
    for (auto msg = consoleManager.m_consoleMessages.rbegin(); msg != consoleManager.m_consoleMessages.rend(); ++msg)
    {
        if (i > 999)
            break;
        i++;
        switch (msg->m_type)
        {
        case ConsoleMessageType::Log:
            if (consoleManager.m_enableConsoleLogs)
            {
                ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 1, 1), msg->m_value.c_str());
                ImGui::Separator();
            }
            break;
        case ConsoleMessageType::Warning:
            if (consoleManager.m_enableConsoleWarnings)
            {
                ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 1, 0, 1), msg->m_value.c_str());
                ImGui::Separator();
            }
            break;
        case ConsoleMessageType::Error:
            if (consoleManager.m_enableConsoleErrors)
            {
                ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1, 0, 0, 1), msg->m_value.c_str());
                ImGui::Separator();
            }
            break;
        }
    }
    ImGui::End();
}
