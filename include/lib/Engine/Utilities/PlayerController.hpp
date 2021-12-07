#pragma once
#include <Application.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class UNIENGINE_API PlayerController : public IPrivateComponent
{
    float m_lastX = 0, m_lastY = 0, m_lastScrollY = 0;
    bool m_startMouse = false;
    float m_sceneCameraYawAngle = -89;
    float m_sceneCameraPitchAngle = 0;
  public:
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    void OnCreate() override;
    void LateUpdate() override;
    void OnInspect() override;

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
};
} // namespace UniEngine
