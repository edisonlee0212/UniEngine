#pragma once
#include <Application.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class UNIENGINE_API PlayerController : public IPrivateComponent
{
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    float m_lastX = 0, m_lastY = 0, m_lastScrollY = 0;
    bool m_startMouse = false;
    float m_sceneCameraYawAngle = -89;
    float m_sceneCameraPitchAngle = 0;
  public:
    void OnCreate() override;
    void LateUpdate() override;
    void SetVelocity(float velocity);
    void SetSensitivity(float sensitivity);

    void Clone(const std::shared_ptr<IPrivateComponent>& target) override;
};
} // namespace UniEngine
