#pragma once
#include "Engine/ECS/Entities.hpp"
#include <RigidBody.hpp>
#include <uniengine_export.h>
#include "PrivateComponentRef.hpp"
using namespace physx;
namespace UniEngine
{
enum class JointType
{
    Fixed = 0,
    /*
    Distance,
    Spherical,
    Revolute,
    Prismatic,
     */
    D6 = 1
};
enum class MotionAxis{
    X = 0,
    Y = 1,
    Z = 2,
    TwistX = 3,
    SwingY = 4,
    SwingZ = 5
};
enum class MotionType{
    Locked = 0,
    Limited = 1,
    Free = 2
};
enum class DriveType{
    X = 0,
    Y = 1,
    Z = 2,
    Swing = 3,
    Twist = 4,
    Slerp = 5
};
class UNIENGINE_API Joint : public IPrivateComponent
{
    JointType m_jointType = JointType::Fixed;
    friend class PhysicsLayer;
    PxJoint *m_joint;
    glm::vec3 m_localPosition1;
    glm::vec3 m_localPosition2;
    glm::quat m_localRotation1;
    glm::quat m_localRotation2;
    bool m_linked = false;
#pragma region Fixed
    void FixedGui();
#pragma endregion
    /*
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
     */
#pragma region D6
    PxD6Motion::Enum m_motionTypes[6] = { PxD6Motion::Enum::eLOCKED };
    PxD6JointDrive m_drives[6] = { PxD6JointDrive() };
    void D6Gui();
#pragma endregion
    bool TypeCheck(const JointType &type);

  public:
    PrivateComponentRef m_rigidBody1;
    PrivateComponentRef m_rigidBody2;
/*
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
 */
#pragma region D6
    void SetMotion(const MotionAxis& axis, const MotionType& type);
    void SetDistanceLimit(const float& toleranceLength, const float& toleranceSpeed, const float& extent, const float& contactDist = -1.0f);
    void SetDrive(const DriveType& type, const float &stiffness, const float &damping, const bool &isAcceleration = true);
#pragma endregion
    void SetType(const JointType &type);
    void Unlink();
    bool Linked();
    void OnCreate() override;

    void Link(const Entity& entity, bool reverse = false);
    void OnInspect() override;
    void OnDestroy() override;

    void Relink(const std::unordered_map<Handle, Handle> &map, const std::shared_ptr<Scene> &scene) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine