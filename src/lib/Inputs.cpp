#include "Editor.hpp"
#include "Engine/Core/Inputs.hpp"
#include "Engine/Core/Windows.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include "Application.hpp"
#include "EditorLayer.hpp"
using namespace UniEngine;

void Inputs::Init()
{
}

bool Inputs::GetKey(int key)
{
    bool retVal = false;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        const auto state = glfwGetKey(Windows::GetWindow(), key);
        retVal = state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    return retVal;
}

bool Inputs::GetMouse(int button)
{
    bool retVal = false;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        retVal = glfwGetMouseButton(Windows::GetWindow(), button) == GLFW_PRESS;
    }
    return retVal;
}
glm::vec2 Inputs::GetMouseAbsolutePosition()
{
    double x = FLT_MIN;
    double y = FLT_MIN;
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        glfwGetCursorPos(Windows::GetWindow(), &x, &y);
    }
    return glm::vec2(x, y);
}

bool Inputs::GetMousePositionInternal(ImGuiWindow *window, glm::vec2 &pos)
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
void Inputs::PreUpdate()
{
    auto mainCamera = Entities::GetCurrentScene()->m_mainCamera.Get<Camera>();
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
bool Inputs::GetMousePosition(glm::vec2 &pos)
{
    pos = GetInstance().m_mousePosition;
    return GetInstance().m_mousePositionValid;
}

bool Inputs::GetKeyInternal(int key, GLFWwindow *window)
{
    auto state = glfwGetKey(window, key);
    return state == GLFW_PRESS;
}

bool Inputs::GetMouseInternal(int button, GLFWwindow *window)
{
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

glm::vec2 Inputs::GetMouseAbsolutePositionInternal(GLFWwindow *window)
{
    double x = FLT_MIN;
    double y = FLT_MIN;
    glfwGetCursorPos(window, &x, &y);
    return glm::vec2(x, y);
}
