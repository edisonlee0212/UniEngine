#include <Application.hpp>
#include "Engine/Utilities/Console.hpp"
using namespace UniEngine;

void Console::Log(const std::string &msg)
{
    auto consoleLayer = Application::GetLayer<ConsoleLayer>();
    if(!consoleLayer) return;
    std::lock_guard lock(consoleLayer->m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Log;
    cm.m_time = Application::Time().CurrentTime();
    consoleLayer->m_consoleMessages.push_back(cm);
}

void Console::Error(const std::string &msg)
{
    auto consoleLayer = Application::GetLayer<ConsoleLayer>();
    if(!consoleLayer) return;
    std::lock_guard lock(consoleLayer->m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Error;
    cm.m_time = Application::Time().CurrentTime();
    consoleLayer->m_consoleMessages.push_back(cm);
}

void Console::Warning(const std::string &msg)
{
    auto consoleLayer = Application::GetLayer<ConsoleLayer>();
    if(!consoleLayer) return;
    std::lock_guard lock(consoleLayer->m_consoleMessageMutex);
    ConsoleMessage cm;
    cm.m_value = msg;
    cm.m_type = ConsoleMessageType::Warning;
    cm.m_time = Application::Time().CurrentTime();
    consoleLayer->m_consoleMessages.push_back(cm);
}

