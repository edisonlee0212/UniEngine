#pragma once
#include "uniengine_export.h"
#include <ILayer.hpp>
namespace UniEngine
{
enum class ConsoleMessageType
{
    Log,
    Warning,
    Error
};
struct ConsoleMessage
{
    ConsoleMessageType m_type = ConsoleMessageType::Log;
    std::string m_value;
    double m_time = 0;
};
class ConsoleLayer : public ILayer
{
    friend class Console;
    std::vector<ConsoleMessage> m_consoleMessages;
    std::mutex m_consoleMessageMutex;

    
    bool m_enableConsoleLogs = true;
    bool m_enableConsoleErrors = true;
    bool m_enableConsoleWarnings = true;

  public:
	bool m_showConsoleWindow = true;
    std::vector<ConsoleMessage> &GetConsoleMessages();
    void OnInspect() override;
};
} // namespace UniEngine