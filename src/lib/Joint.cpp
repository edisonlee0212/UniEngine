#include <EditorManager.hpp>
#include <Joint.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;
#pragma region Fixed
void Joint::FixedGui()
{
}
#pragma endregion
#pragma region Distance
void Joint::DistanceGui()
{
    if (ImGui::DragFloat("Min", &m_minDistance, 0.1f, FLT_MIN, m_maxDistance))
    {
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Enabled", &m_minDistanceEnabled))
    {
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    if (ImGui::DragFloat("Max", &m_maxDistance, 0.1f, m_minDistance, FLT_MAX))
    {
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Enabled", &m_maxDistanceEnabled))
    {
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }

    if (ImGui::DragFloat("Stiffness", &m_stiffness))
    {
        SetStiffness(m_stiffness);
    }
    if (ImGui::DragFloat("Damping", &m_damping))
    {
        SetDamping(m_damping);
    }
}
void Joint::SetMax(const float &value, const bool &enabled)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_maxDistance != value || m_maxDistanceEnabled != enabled)
    {
        m_maxDistance = value;
        m_maxDistanceEnabled = enabled;
        static_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, m_maxDistanceEnabled);
        static_cast<PxDistanceJoint *>(m_joint)->setMaxDistance(m_maxDistance);
    }
}
void Joint::SetMin(const float &value, const bool &enabled)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_minDistance != value || m_maxDistanceEnabled != enabled)
    {
        m_minDistance = value;
        m_minDistanceEnabled = enabled;
        static_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, m_minDistanceEnabled);
        static_cast<PxDistanceJoint *>(m_joint)->setMinDistance(m_minDistance);
    }
}
void Joint::SetStiffness(const float &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_stiffness != value)
    {
        m_stiffness = value;
        static_cast<PxDistanceJoint *>(m_joint)->setStiffness(m_stiffness);
    }
}
void Joint::SetDamping(const float &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_damping != value)
    {
        m_damping = value;
        static_cast<PxDistanceJoint *>(m_joint)->setDamping(m_damping);
    }
}

#pragma endregion
#pragma region Spherical
void Joint::SphericalGui()
{
}
#pragma endregion
#pragma region Revolute
void Joint::RevoluteGui()
{
}
#pragma endregion
#pragma region Prismatic
void Joint::PrismaticGui()
{
}
#pragma endregion
#pragma region D6

void Joint::D6Gui()
{
    auto *joint = static_cast<PxD6Joint *>(m_joint);
}
#pragma endregion
void Joint::Unlink()
{
    if (m_joint)
    {
        m_joint->release();
        m_joint = nullptr;
    }
    m_linkedEntity.Reset();
}
bool Joint::Linked()
{
    return m_joint;
}
bool Joint::SafetyCheck()
{
    if (!m_linkedEntity.Get().HasPrivateComponent<RigidBody>())
    {
        UNIENGINE_ERROR("Linked Entity doesn't contains RigidBody component!");
        return false;
    }
    if (m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_static &&
    GetOwner().GetOrSetPrivateComponent<RigidBody>().lock()->m_static)
    {
        UNIENGINE_ERROR("At least one side of the joint is movable!");
        return false;
    }
    return true;
}
void Joint::OnCreate()
{
    const auto owner = GetOwner();
    if (!owner.HasPrivateComponent<RigidBody>())
    {
        owner.GetOrSetPrivateComponent<RigidBody>();
    }

    Link(m_linkedEntity.Get());
}
static const char *JointTypeNames[]{"Fixed", "Distance", "Spherical", "Revolute", "Prismatic", "D6"};
void Joint::OnGui()
{
    static int type = 0;
    type = (int)m_jointType;
    if (ImGui::Combo("Joint Type", &type, JointTypeNames, IM_ARRAYSIZE(JointTypeNames)))
    {
        SetType((JointType)type);
    }
    auto storedEntity = m_linkedEntity.Get();
    if (EditorManager::DragAndDrop(storedEntity))
    {
        if (storedEntity != m_linkedEntity.Get())
        {
            m_linkedEntity.Update(storedEntity);
            Link(storedEntity);
        }
    }
    if (m_joint)
    {
        switch (m_jointType)
        {
        case JointType::Fixed:
            FixedGui();
            break;
        case JointType::Distance:
            DistanceGui();
            break;
        case JointType::Spherical:
            SphericalGui();
            break;
        case JointType::Revolute:
            RevoluteGui();
            break;
        case JointType::Prismatic:
            PrismaticGui();
            break;
        case JointType::D6:
            D6Gui();
            break;
        }
    }
}

void Joint::OnDestroy()
{
    Unlink();
}

void Joint::Link(const Entity &targetEntity)
{
    Unlink();
    const auto owner = GetOwner();
    if (targetEntity.IsNull() || owner == targetEntity || (targetEntity.IsValid() && !targetEntity.HasPrivateComponent<RigidBody>()))
    {
        return;
    }
    m_linkedEntity.Update(targetEntity);
    if (SafetyCheck())
    {
        PxTransform localFrame1;
        auto ownerGT = owner.GetDataComponent<GlobalTransform>();
        ownerGT.SetScale(glm::vec3(1.0f));
        auto linkerGT = m_linkedEntity.Get().GetDataComponent<GlobalTransform>();
        linkerGT.SetScale(glm::vec3(1.0f));
        Transform transform;
        transform.m_value = glm::inverse(ownerGT.m_value) * linkerGT.m_value;
        const auto position0 = glm::vec3(0.0f);
        const glm::quat rotation0 = glm::vec3(0.0f);

        const auto position1 = transform.GetPosition();
        const auto rotation1 = transform.GetRotation();

        switch (m_jointType)
        {
        case JointType::Fixed:
            m_joint = PxFixedJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                    owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Distance:
            m_joint = PxDistanceJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                    owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Spherical: {
            m_joint = PxSphericalJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)),
                    m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)));
            // static_cast<PxSphericalJoint *>(m_joint)->setLimitCone(PxJointLimitCone(PxPi / 2, PxPi / 6, 0.01f));
            // static_cast<PxSphericalJoint *>(m_joint)->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED,
            // true);
        }
        break;
        case JointType::Revolute:
            m_joint = PxRevoluteJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                    owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Prismatic:
            m_joint = PxPrismaticJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                    owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::D6:
            m_joint = PxD6JointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.Get().GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                    owner.GetOrSetPrivateComponent<RigidBody>().lock()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            static_cast<PxD6Joint *>(m_joint)->setProjectionAngularTolerance(1.0f);
            static_cast<PxD6Joint *>(m_joint)->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
            break;
        }
    }
}

bool Joint::TypeCheck(const JointType &type)
{
    if (m_jointType != type)
    {
        UNIENGINE_ERROR("Wrong joint type!");
        return false;
    }
    return true;
}
void Joint::SetType(const JointType &type)
{
    if (type != m_jointType)
    {
        m_jointType = type;
        Link(m_linkedEntity.Get());
    }
}

void Joint::SetMotion(const MotionAxis &axis, const MotionType &type)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    m_motionTypes[static_cast<int>(axis)] = static_cast<PxD6Motion::Enum>(type);
    static_cast<PxD6Joint *>(m_joint)->setMotion(
        static_cast<PxD6Axis::Enum>(axis), static_cast<PxD6Motion::Enum>(type));

}
void Joint::SetDrive(const DriveType &type, const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    m_drives[static_cast<int>(type)].stiffness = stiffness;
    m_drives[static_cast<int>(type)].damping = damping;
    m_drives[static_cast<int>(type)].flags =
        static_cast<PxD6JointDriveFlag::Enum>(isAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
    static_cast<PxD6Joint *>(m_joint)->setDrive(static_cast<PxD6Drive::Enum>(type),  m_drives[static_cast<int>(type)]);
}
void Joint::SetDistanceLimit(const float& toleranceLength, const float& toleranceSpeed, const float &extent, const float &contactDist)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    auto scale = PxTolerancesScale();
    scale.length = toleranceLength;
    scale.speed = toleranceSpeed;
    static_cast<PxD6Joint *>(m_joint)->setDistanceLimit(PxJointLinearLimit(scale, extent, contactDist));
}
void Joint::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<Joint>(target);
}
