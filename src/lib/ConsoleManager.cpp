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
