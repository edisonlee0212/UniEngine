//
// Created by lllll on 11/1/2021.
//

#include "ConsoleLayer.hpp"
using namespace UniEngine;
void ConsoleLayer::OnInspect()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Console", &m_showConsoleWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if (m_showConsoleWindow) {
        if (ImGui::Begin("Console")) {
            ImGui::Checkbox("Log", &m_enableConsoleLogs);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &m_enableConsoleWarnings);
            ImGui::SameLine();
            ImGui::Checkbox("Error", &m_enableConsoleErrors);
            ImGui::SameLine();
            if (ImGui::Button("Clear all")) {
                m_consoleMessages.clear();
            }
            int i = 0;
            for (auto msg = m_consoleMessages.rbegin(); msg != m_consoleMessages.rend(); ++msg)
            {
                if (i > 999)
                    break;
                i++;
                switch (msg->m_type)
                {
                case ConsoleMessageType::Log:
                    if (m_enableConsoleLogs)
                    {
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1, 1, 1, 1), msg->m_value.c_str());
                        ImGui::Separator();
                    }
                    break;
                case ConsoleMessageType::Warning:
                    if (m_enableConsoleWarnings)
                    {
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), msg->m_value.c_str());
                        ImGui::Separator();
                    }
                    break;
                case ConsoleMessageType::Error:
                    if (m_enableConsoleErrors)
                    {
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "%.2f: ", msg->m_time);
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), msg->m_value.c_str());
                        ImGui::Separator();
                    }
                    break;
                }
            }
        }
        ImGui::End();
    }
}
std::vector<ConsoleMessage> &UniEngine::ConsoleLayer::GetConsoleMessages()
{
    return m_consoleMessages;
}
