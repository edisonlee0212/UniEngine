#pragma once
#include <RigidBody.hpp>
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API DistanceJoint : public PrivateComponentBase
{
    friend class PhysicsManager;
    float m_maxDistance = 0;
    float m_minDistance = 0;
    bool m_maxDistanceEnabled = false;
    bool m_minDistanceEnabled = false;

    float m_stiffness = 0;
    float m_damping = 0;

    PxDistanceJoint* m_joint;
  public:
    void SetMax(const float& value, const bool& enabled);
    void SetMin(const float& value, const bool& enabled);
    void SetStiffness(const float& value);
    void SetDamping(const float& value);
    Entity m_linkedEntity;
    void Unlink();
    bool Linked();
    bool SafetyCheck();
    void Init() override;
    void Link();
    void OnGui() override;
    ~DistanceJoint();
};
}