#pragma once
#include <EntityManager.hpp>
#include <RigidBody.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
enum class JointType
{
    Fixed,
    Distance,
    Spherical,
    Revolute,
    Prismatic,
    D6
};
enum class MotionAxis{
    X,
    Y,
    Z,
    TwistX,
    SwingY,
    SwingZ
};
enum class MotionType{
    Locked,
    Limited,
    Free
};
enum class DriveType{
    X,
    Y,
    Z,
    Swing,
    Twist,
    Slerp
};
class UNIENGINE_API Joint : public IPrivateComponent
{
    JointType m_jointType = JointType::Spherical;
    friend class PhysicsManager;
    PxJoint *m_joint;

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
    void SetMax(const float &value, const bool &enabled);
    void SetMin(const float &value, const bool &enabled);
    void SetStiffness(const float &value);
    void SetDamping(const float &value);
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
    PxD6Motion::Enum m_motionTypes[6] = { PxD6Motion::Enum::eLOCKED };
    PxD6JointDrive m_drives[5] = { PxD6JointDrive() };
    void D6Gui();
#pragma endregion

    Entity m_linkedEntity;
    bool SafetyCheck();
    bool TypeCheck(const JointType &type);

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
    void SetMotion(const MotionAxis& axis, const MotionType& type);
    void SetDistanceLimit(const float& toleranceLength, const float& toleranceSpeed, const float& extent, const float& contactDist = -1.0f);
    void SetDrive(const DriveType& type, const float &stiffness, const float &damping, const bool &isAcceleration = true);
#pragma endregion
    void SetType(const JointType &type);
    void Unlink();
    bool Linked();
    void OnCreate() override;
    void Link(const Entity &targetEntity);
    void OnGui() override;
    void OnDestroy() override;
};
} // namespace UniEngine