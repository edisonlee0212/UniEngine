#pragma once
#include <Application.hpp>
namespace UniEngine
{
class CameraControlSystem : public SystemBase
{
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    float m_lastX = 0, m_lastY = 0, m_lastScrollY = 0;
    bool m_startMouse = false;
    float m_sceneCameraYawAngle = -90;
    float m_sceneCameraPitchAngle = 0;

  public:
    void LateUpdate() override;
    void SetVelocity(float velocity);
    void SetSensitivity(float sensitivity);
};
} // namespace UniEngine
