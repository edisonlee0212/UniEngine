//
// Created by lllll on 8/9/2021.
//
#include <EditorManager.hpp>
#include <PlayerController.hpp>
#include "EditorLayer.hpp"
void UniEngine::PlayerController::OnCreate()
{
    m_startMouse = false;
}
void UniEngine::PlayerController::LateUpdate()
{
    if (Application::GetLayer<EditorLayer>()->MainCameraWindowFocused())
    {
#pragma region Scene Camera Controller
        auto transform = GetOwner().GetDataComponent<Transform>();
        const auto rotation = transform.GetRotation();
        auto position = transform.GetPosition();
        const auto front = rotation * glm::vec3(0, 0, -1);
        const auto right = rotation * glm::vec3(1, 0, 0);
        auto moved = false;
        if (InputManager::GetKey(GLFW_KEY_W))
        {
            position += front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
            moved = true;
        }
        if (InputManager::GetKey(GLFW_KEY_S))
        {
            position -= front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
            moved = true;
        }
        if (InputManager::GetKey(GLFW_KEY_A))
        {
            position -= right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
            moved = true;
        }
        if (InputManager::GetKey(GLFW_KEY_D))
        {
            position += right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
            moved = true;
        }
        if (InputManager::GetKey(GLFW_KEY_LEFT_SHIFT))
        {
            position.y += m_velocity * static_cast<float>(Application::Time().DeltaTime());
            moved = true;
        }
        if (InputManager::GetKey(GLFW_KEY_LEFT_CONTROL))
        {
            position.y -= m_velocity * static_cast<float>(Application::Time().DeltaTime());
            moved = true;
        }
        if (moved)
        {
            transform.SetPosition(position);
        }
        glm::vec2 mousePosition;
        bool valid = InputManager::GetMousePosition(mousePosition);
        float xOffset = 0;
        float yOffset = 0;
        if (valid)
        {
            if (!m_startMouse)
            {
                m_lastX = mousePosition.x;
                m_lastY = mousePosition.y;
                m_startMouse = true;
            }
            xOffset = mousePosition.x - m_lastX;
            yOffset = -mousePosition.y + m_lastY;
            m_lastX = mousePosition.x;
            m_lastY = mousePosition.y;
        }
        if (InputManager::GetMouse(GLFW_MOUSE_BUTTON_RIGHT))
        {
            if (xOffset != 0 || yOffset != 0)
            {
                moved = true;
                m_sceneCameraYawAngle += xOffset * m_sensitivity;
                m_sceneCameraPitchAngle += yOffset * m_sensitivity;

                if (m_sceneCameraPitchAngle > 89.0f)
                    m_sceneCameraPitchAngle = 89.0f;
                if (m_sceneCameraPitchAngle < -89.0f)
                    m_sceneCameraPitchAngle = -89.0f;

                transform.SetRotation(
                        Camera::ProcessMouseMovement(m_sceneCameraYawAngle, m_sceneCameraPitchAngle, false));
            }
        }
        if (moved)
        {
            GetOwner().SetDataComponent(transform);
        }
#pragma endregion
    }
}

void UniEngine::PlayerController::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
}
void UniEngine::PlayerController::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_velocity" << YAML::Value << m_velocity;
    out << YAML::Key << "m_sensitivity" << YAML::Value << m_sensitivity;
    out << YAML::Key << "m_sceneCameraYawAngle" << YAML::Value << m_sceneCameraYawAngle;
    out << YAML::Key << "m_sceneCameraPitchAngle" << YAML::Value << m_sceneCameraPitchAngle;
}
void UniEngine::PlayerController::Deserialize(const YAML::Node &in)
{
    if(in["m_velocity"]) m_velocity = in["m_velocity"].as<float>();
    if(in["m_sensitivity"]) m_sensitivity = in["m_sensitivity"].as<float>();
    if(in["m_sceneCameraYawAngle"]) m_sceneCameraYawAngle = in["m_sceneCameraYawAngle"].as<float>();
    if(in["m_sceneCameraPitchAngle"]) m_sceneCameraPitchAngle = in["m_sceneCameraPitchAngle"].as<float>();
}
void UniEngine::PlayerController::OnInspect()
{
    ImGui::DragFloat("Velocity", &m_velocity, 0.01f);
    ImGui::DragFloat("Mouse sensitivity", &m_sensitivity, 0.01f);
    ImGui::DragFloat("Yaw angle", &m_sceneCameraYawAngle, 0.01f);
    ImGui::DragFloat("Pitch angle", &m_sceneCameraPitchAngle, 0.01f);
}