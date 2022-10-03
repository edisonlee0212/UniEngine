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


void Inputs::PreUpdate()
{
    auto mainCamera = Application::GetActiveScene()->m_mainCamera.Get<Camera>();
    if (mainCamera && Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

        ImGui::Begin("Camera");
        {
            if (ImGui::BeginChild("MainCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::IsWindowFocused())
                {
                    ImVec2 viewPortSize = ImGui::GetWindowSize();
                    auto mousePosition = glm::vec2(FLT_MAX, FLT_MIN);
                    auto mp = ImGui::GetMousePos();
                    auto wp = ImGui::GetWindowPos();
                    mousePosition = glm::vec2(mp.x - wp.x, mp.y - wp.y - 20);
                    GetInstance().m_mousePositionValid = !(mousePosition.x < 0 || mousePosition.y < 0 || mousePosition.x > viewPortSize.x ||
                    mousePosition.y > viewPortSize.y);
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
