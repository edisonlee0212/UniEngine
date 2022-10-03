#pragma once
#include "ISingleton.hpp"
#include "uniengine_export.h"
namespace UniEngine
{
class UNIENGINE_API Inputs : public ISingleton<Inputs>
{
    friend class Editor;
    friend class Application;
    bool m_enableInputMenu;
    friend class Application;
    friend class Windows;
    friend class Graphics;
    friend class Application;
    glm::vec2 m_mousePosition;
    bool m_mousePositionValid = false;
  public:
    static void PreUpdate();
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