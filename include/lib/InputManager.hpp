#pragma once
#include <ISingleton.hpp>
#include <uniengine_export.h>
#include <Gui.hpp>
namespace UniEngine
{
class UNIENGINE_API InputManager : public ISingleton<InputManager>
{
    friend class EditorManager;
    friend class Application;
    static void LateUpdate();
    bool m_enableInputMenu;
    friend class Application;
    friend class WindowManager;
    friend class RenderManager;
    friend class Application;

  public:
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