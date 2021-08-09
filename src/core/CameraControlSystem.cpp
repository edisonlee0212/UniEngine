#include <Camera.hpp>
#include <CameraControlSystem.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void CameraControlSystem::OnCreate()
{
    auto mainCamera = RenderManager::GetMainCamera().lock();
    auto transform = mainCamera->GetOwner().GetDataComponent<Transform>();
    transform.SetRotation(Camera::ProcessMouseMovement(m_sceneCameraYawAngle, m_sceneCameraPitchAngle, false));
    mainCamera->GetOwner().SetDataComponent(transform);
    Enable();
}

void CameraControlSystem::LateUpdate()
{
    if(RenderManager::GetMainCamera().expired()) return;
    auto mainCamera = RenderManager::GetMainCamera().lock();
    if (EditorManager::MainCameraWindowFocused())
    {
#pragma region Scene Camera Controller
        auto transform = mainCamera->GetOwner().GetDataComponent<Transform>();
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
            mainCamera->GetOwner().SetDataComponent(transform);
        }
#pragma endregion
    }
}

void CameraControlSystem::SetVelocity(float velocity)
{
    m_velocity = velocity;
}

void CameraControlSystem::SetSensitivity(float sensitivity)
{
    m_sensitivity = sensitivity;
}
