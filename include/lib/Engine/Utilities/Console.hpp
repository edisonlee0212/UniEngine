#pragma once
#include "uniengine_export.h"
#include "ISingleton.hpp"
#include "ConsoleLayer.hpp"
namespace UniEngine
{

class Console : public ISingleton<Console>
{
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
};

} // namespace UniEngine

/**
 * \brief A thread-safe message log macro.
 * \param msg The log message
 */
#define UNIENGINE_LOG(msg)                                                                                             \
    {                                                                                                                  \
        UniEngine::Console::Log(msg);                                                                                    \
        std::cout << "(UniEngine)Log: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;       \
    }

/**
 * \brief A thread-safe error log macro.
 * \param msg The error message
 */
#define UNIENGINE_ERROR(msg)                                                                                           \
    {                                                                                                                  \
        UniEngine::Console::Error(msg);                                                                                  \
        std::cerr << "(UniEngine)Error: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;     \
    }
/**
 * \brief A thread-safe warning log macro.
 * \param msg The warning message
 */
#define UNIENGINE_WARNING(msg)                                                                                         \
    {                                                                                                                  \
        UniEngine::Console::Warning(msg);                                                                                \
        std::cout << "(UniEngine)Warning: " << msg << " (" << __FILE__ << ": line " << __LINE__ << ")" << std::endl;   \
    }