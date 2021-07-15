#pragma once
#include <Core/OpenGLUtils.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API WindowManager : public ISingleton<WindowManager>
{
  public:
    static void LateUpdate();
    static void Init(std::string name, bool fullScreen = false);
    static GLFWwindow *GetWindow();
    static GLFWmonitor *PrimaryMonitor();
    static void PreUpdate();
    static void DrawTexture(OpenGLUtils::GLTexture2D *texture);
    static void ResizeCallback(GLFWwindow *, int, int);
    static void SetMonitorCallback(GLFWmonitor *monitor, int event);

  private:
    std::vector<GLFWmonitor *> m_monitors;
    GLFWmonitor *m_primaryMonitor = nullptr;
    GLFWwindow *m_window = nullptr;
    unsigned m_windowWidth = 1;
    unsigned m_windowHeight = 1;
};

} // namespace UniEngine
