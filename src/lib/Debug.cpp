#include <Application.hpp>
#include <Debug.hpp>
using namespace UniEngine;

Debug &Debug::GetInstance()
{
    static Debug instance;
    return instance;
}

void Debug::Log(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Log;
    cm.m_time = Application::EngineTime();
    instance.m_consoleMessages.push_back(cm);
}

void Debug::Error(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Error;
    cm.m_time = Application::EngineTime();
    instance.m_consoleMessages.push_back(cm);
}

void Debug::Warning(const std::string &msg)
{
    auto &instance = GetInstance();
    std::lock_guard lock(instance.m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Warning;
    cm.m_time = Application::EngineTime();
    instance.m_consoleMessages.push_back(cm);
}

std::vector<ConsoleMessage> &Debug::GetConsoleMessages()
{
    return GetInstance().m_consoleMessages;
}
