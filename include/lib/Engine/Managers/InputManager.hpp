#pragma once
#include <Gui.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API InputManager : public ISingleton<InputManager>
{
    friend class EditorManager;
    friend class Application;
    static void OnInspect();
    bool m_enableInputMenu;
    friend class Application;
    friend class WindowManager;
    friend class RenderManager;
    friend class Application;
    glm::vec2 m_mousePosition;
    bool m_mousePositionValid = false;
  public:
    static void PreUpdate();
    static bool GetMousePositionInternal(ImGuiWindow *window, glm::vec2 &pos);
    static bool GetKeyInternal(int key, GLFWwindow *window);
    static bool GetMouseInternal(int button, GLFWwindow *window);
    static glm::vec2 GetMouseAbsolutePositionInternal(GLFWwindow *window);
    static void Init();
    static bool GetKey(int key);
    static bool GetMouse(int button);
    static glm::vec2 GetMouseAbsolutePosition();
    static bool GetMousePosition(glm::vec2 &pos);
};

} // namespace UniEngine