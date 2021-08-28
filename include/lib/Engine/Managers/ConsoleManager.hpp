#pragma once
#include <uniengine_export.h>
#include <ISingleton.hpp>
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
class ConsoleManager : public ISingleton<ConsoleManager>
{
    std::vector<ConsoleMessage> m_consoleMessages;
    std::mutex m_consoleMessageMutex;

    bool m_enableConsole = true;
    bool m_enableConsoleLogs = false;
    bool m_enableConsoleErrors = true;
    bool m_enableConsoleWarnings = false;


  public:
    /**
     * Push log to the console
     * @param msg The message to print.
     */
    UNIENGINE_API static void Log(const std::string &msg);
    /**
     * Push error to the console
     * @param msg The message to print.
     */
    UNIENGINE_API static void Error(const std::string &msg);
    /**
     * Push warning to the console
     * @param msg The message to print.
     */
    UNIENGINE_API static void Warning(const std::string &msg);
    /**
     * Retrieve current stored messages.
     * @return The list of message stored.
     */
    UNIENGINE_API static std::vector<ConsoleMessage> &GetConsoleMessages();

    static void OnGui();
};

} // namespace UniEngine

/**
 * \brief A thread-safe message log macro.
 * \param msg The log message
 */
#define UNIENGINE_LOG(msg)                                                                                             \
    {                                                                                                                  \
        UniEngine::ConsoleManager::Log(msg);                                                                                    \
        std::cout << "(UniEngine)Log: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;       \
    }

/**
 * \brief A thread-safe error log macro.
 * \param msg The error message
 */
#define UNIENGINE_ERROR(msg)                                                                                           \
    {                                                                                                                  \
        UniEngine::ConsoleManager::Error(msg);                                                                                  \
        std::cerr << "(UniEngine)Error: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;     \
    }
/**
 * \brief A thread-safe warning log macro.
 * \param msg The warning message
 */
#define UNIENGINE_WARNING(msg)                                                                                         \
    {                                                                                                                  \
        UniEngine::ConsoleManager::Warning(msg);                                                                                \
        std::cout << "(UniEngine)Warning: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;   \
    }