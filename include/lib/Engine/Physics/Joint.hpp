#pragma once
#include <RigidBody.hpp>
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
enum class JointType{
    Fixed,
    Distance,
    Spherical,
    Revolute,
    Prismatic,
    D6
};

class UNIENGINE_API Joint : public PrivateComponentBase
{
    JointType m_jointType = JointType::Spherical;
    friend class PhysicsManager;
    PxJoint* m_joint;


#pragma region Fixed
    void FixedGui();
#pragma endregion
#pragma region Distance
    float m_maxDistance = FLT_MIN;
    float m_minDistance = FLT_MAX;
    bool m_maxDistanceEnabled = false;
    bool m_minDistanceEnabled = false;
    float m_stiffness = 0;
    float m_damping = 0;
    void SetMax(const float& value, const bool& enabled);
    void SetMin(const float& value, const bool& enabled);
    void SetStiffness(const float& value);
    void SetDamping(const float& value);
    void DistanceGui();
#pragma endregion
#pragma region Spherical
    void SphericalGui();
#pragma endregion
#pragma region Revolute
    void RevoluteGui();
#pragma endregion
#pragma region Prismatic
    void PrismaticGui();
#pragma endregion
#pragma region D6
    bool m_xLocked = true;
    bool m_yLocked = true;
    bool m_zLocked = true;
    PxD6JointDrive m_xDrive;
    bool m_xAcceleration = true;
    PxD6JointDrive m_yDrive;
    bool m_yAcceleration = true;
    PxD6JointDrive m_zDrive;
    bool m_zAcceleration = true;
    void D6Gui();
#pragma endregion

    Entity m_linkedEntity;
    bool SafetyCheck();
    bool TypeCheck(const JointType& type);
  public:
#pragma region Fixed
#pragma endregion
#pragma region Distance
#pragma endregion
#pragma region Spherical
#pragma endregion
#pragma region Revolute
#pragma endregion
#pragma region Prismatic
#pragma endregion
#pragma region D6
    void SetLockX(const bool& value);
    void SetLockY(const bool& value);
    void SetLockZ(const bool& value);
    void SetDriveX(const float& stiffness, const float& damping, const bool& isAcceleration = true);
    void SetDriveY(const float& stiffness, const float& damping, const bool& isAcceleration = true);
    void SetDriveZ(const float& stiffness, const float& damping, const bool& isAcceleration = true);
#pragma endregion
    void SetType(const JointType& type);
    void Unlink();
    bool Linked();
    void Init() override;
    void Link(const Entity& targetEntity);
    void OnGui() override;
    ~Joint() override;
};
}