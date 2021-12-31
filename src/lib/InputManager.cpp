#include <EditorManager.hpp>
#include <InputManager.hpp>
#include <WindowManager.hpp>
#include "Engine/Utilities/Graphics.hpp"
#include "Application.hpp"
#include "EditorLayer.hpp"
using namespace UniEngine;

void InputManager::Init()
{
}

bool InputManager::GetKey(int key)
{
    bool retVal = false;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        const auto state = glfwGetKey(WindowManager::GetWindow(), key);
        retVal = state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    return retVal;
}

bool InputManager::GetMouse(int button)
{
    bool retVal = false;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        retVal = glfwGetMouseButton(WindowManager::GetWindow(), button) == GLFW_PRESS;
    }
    return retVal;
}
glm::vec2 InputManager::GetMouseAbsolutePosition()
{
    double x = FLT_MIN;
    double y = FLT_MIN;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        glfwGetCursorPos(WindowManager::GetWindow(), &x, &y);
    }
    return glm::vec2(x, y);
}

bool InputManager::GetMousePositionInternal(ImGuiWindow *window, glm::vec2 &pos)
{
    ImGuiIO &io = ImGui::GetIO();
    const auto viewPortSize = window->Size;
    const auto overlayPos = window->Pos;
    const ImVec2 windowPos = ImVec2(overlayPos.x + viewPortSize.x, overlayPos.y);
    if (ImGui::IsMousePosValid())
    {
        pos.x = io.MousePos.x - windowPos.x;
        pos.y = io.MousePos.y - windowPos.y - 20; // In editormanager, a 20 offset is preserved for menu bar.
        return true;
    }
    return false;
}
void InputManager::PreUpdate()
{
    auto mainCamera = EntityManager::GetCurrentScene()->m_mainCamera.Get<Camera>();
    if (mainCamera && Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

        ImGui::Begin("Camera");
        {
            if (ImGui::BeginChild("MainCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::IsWindowFocused())
                {
                    GetInstance().m_mousePositionValid = GetMousePositionInternal(ImGui::GetCurrentWindowRead(), GetInstance().m_mousePosition);
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }else{
        GetInstance().m_mousePositionValid = false;
    };
}
bool InputManager::GetMousePosition(glm::vec2 &pos)
{
    pos = GetInstance().m_mousePosition;
    return GetInstance().m_mousePositionValid;
}

bool InputManager::GetKeyInternal(int key, GLFWwindow *window)
{
    auto state = glfwGetKey(window, key);
    return state == GLFW_PRESS;
}

bool InputManager::GetMouseInternal(int button, GLFWwindow *window)
{
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

glm::vec2 InputManager::GetMouseAbsolutePositionInternal(GLFWwindow *window)
{
    double x = FLT_MIN;
    double y = FLT_MIN;
    glfwGetCursorPos(window, &x, &y);
    return glm::vec2(x, y);
}
