#pragma once
#include <uniengine_export.h>

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
class Debug
{
    std::vector<ConsoleMessage> m_consoleMessages;
    std::mutex m_consoleMessageMutex;

  protected:
    Debug() = default;

  public:
    UNIENGINE_API static Debug &GetInstance();
    UNIENGINE_API static void Log(const std::string &msg);
    UNIENGINE_API static void Error(const std::string &msg);
    UNIENGINE_API static void Warning(const std::string &msg);
    UNIENGINE_API static std::vector<ConsoleMessage> &GetConsoleMessages();
};

} // namespace UniEngine

/**
 * \brief A thread-safe message log macro.
 * \param msg The log message
 */
#define UNIENGINE_LOG(msg)                                                                                             \
    {                                                                                                                  \
        UniEngine::Debug::Log(msg);                                                                                    \
        std::cout << "(UniEngine)Log: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;                     \
    }

/**
 * \brief A thread-safe error log macro.
 * \param msg The error message
 */
#define UNIENGINE_ERROR(msg)                                                                                           \
    {                                                                                                                  \
        UniEngine::Debug::Error(msg);                                                                                  \
        std::cerr << "(UniEngine)Error: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;                     \
    }
/**
 * \brief A thread-safe warning log macro.
 * \param msg The warning message
 */
#define UNIENGINE_WARNING(msg)                                                                                         \
    {                                                                                                                  \
        UniEngine::Debug::Warning(msg);                                                                                \
        std::cout << "(UniEngine)Warning: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;                     \
    }