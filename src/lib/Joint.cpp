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
        dynamic_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, m_maxDistanceEnabled);
        dynamic_cast<PxDistanceJoint *>(m_joint)->setMaxDistance(m_maxDistance);
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
        dynamic_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, m_minDistanceEnabled);
        dynamic_cast<PxDistanceJoint *>(m_joint)->setMinDistance(m_minDistance);
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
        dynamic_cast<PxDistanceJoint *>(m_joint)->setStiffness(m_stiffness);
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
        dynamic_cast<PxDistanceJoint *>(m_joint)->setDamping(m_damping);
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
    auto *joint = dynamic_cast<PxD6Joint *>(m_joint);
    if (ImGui::Checkbox("Lock X", &m_xLocked))
    {
        joint->setMotion(PxD6Axis::eX, m_xLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
    if (!m_xLocked)
    {
        if (ImGui::DragFloat("X Stiffness", &m_xDrive.stiffness))
        {
            joint->setDrive(PxD6Drive::eX, m_xDrive);
        }

        if (ImGui::DragFloat("X Damping", &m_xDrive.damping))
        {
            joint->setDrive(PxD6Drive::eX, m_xDrive);
        }
        if (ImGui::Checkbox("X is acceleration", &m_xAcceleration))
        {
            m_xDrive.flags =
                static_cast<PxD6JointDriveFlag::Enum>(m_xAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
            joint->setDrive(PxD6Drive::eX, m_xDrive);
        }
    }

    if (ImGui::Checkbox("Lock Y", &m_yLocked))
    {
        joint->setMotion(PxD6Axis::eY, m_yLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
    if (!m_yLocked)
    {
        if (ImGui::DragFloat("Y Stiffness", &m_yDrive.stiffness))
        {
            joint->setDrive(PxD6Drive::eY, m_yDrive);
        }

        if (ImGui::DragFloat("Y Damping", &m_yDrive.damping))
        {
            joint->setDrive(PxD6Drive::eY, m_yDrive);
        }
        if (ImGui::Checkbox("Y is acceleration", &m_yAcceleration))
        {
            m_yDrive.flags =
                static_cast<PxD6JointDriveFlag::Enum>(m_yAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
            joint->setDrive(PxD6Drive::eY, m_yDrive);
        }
    }
    if (ImGui::Checkbox("Lock Z", &m_zLocked))
    {
        joint->setMotion(PxD6Axis::eZ, m_zLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
    if (!m_zLocked)
    {
        if (ImGui::DragFloat("Z Stiffness", &m_zDrive.stiffness))
        {
            joint->setDrive(PxD6Drive::eZ, m_zDrive);
        }

        if (ImGui::DragFloat("Z Damping", &m_zDrive.damping))
        {
            joint->setDrive(PxD6Drive::eZ, m_zDrive);
        }
        if (ImGui::Checkbox("Z is acceleration", &m_zAcceleration))
        {
            m_zDrive.flags =
                static_cast<PxD6JointDriveFlag::Enum>(m_zAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
            joint->setDrive(PxD6Drive::eZ, m_zDrive);
        }
    }
}
void Joint::SetLockX(const bool &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    if (m_xLocked != value)
    {
        m_xLocked = value;
        dynamic_cast<PxD6Joint *>(m_joint)->setMotion(
            PxD6Axis::eX, m_xLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void Joint::SetLockZ(const bool &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    if (m_zLocked != value)
    {
        m_zLocked = value;
        dynamic_cast<PxD6Joint *>(m_joint)->setMotion(
            PxD6Axis::eZ, m_zLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void Joint::SetLockY(const bool &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    if (m_yLocked != value)
    {
        m_yLocked = value;
        dynamic_cast<PxD6Joint *>(m_joint)->setMotion(
            PxD6Axis::eY, m_yLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void Joint::SetDriveY(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;

    if (stiffness != m_yDrive.stiffness || damping != m_yDrive.damping ||
        isAcceleration != (m_yDrive.flags == PxD6JointDriveFlag::eACCELERATION))
    {
        m_yDrive.stiffness = stiffness;
        m_yDrive.damping = damping;
        m_yAcceleration = isAcceleration;
        m_yDrive.flags =
            static_cast<PxD6JointDriveFlag::Enum>(m_yAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        dynamic_cast<PxD6Joint *>(m_joint)->setDrive(PxD6Drive::eY, m_yDrive);
    }
}
void Joint::SetDriveX(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    if (stiffness != m_xDrive.stiffness || damping != m_xDrive.damping ||
        isAcceleration != (m_xDrive.flags == PxD6JointDriveFlag::eACCELERATION))
    {
        m_xDrive.stiffness = stiffness;
        m_xDrive.damping = damping;
        m_xAcceleration = isAcceleration;
        m_xDrive.flags =
            static_cast<PxD6JointDriveFlag::Enum>(m_xAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        dynamic_cast<PxD6Joint *>(m_joint)->setDrive(PxD6Drive::eX, m_xDrive);
    }
}
void Joint::SetDriveZ(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    if (stiffness != m_zDrive.stiffness || damping != m_zDrive.damping ||
        isAcceleration != (m_zDrive.flags == PxD6JointDriveFlag::eACCELERATION))
    {
        m_zDrive.stiffness = stiffness;
        m_zDrive.damping = damping;
        m_zAcceleration = isAcceleration;
        m_zDrive.flags =
            static_cast<PxD6JointDriveFlag::Enum>(m_zAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        dynamic_cast<PxD6Joint *>(m_joint)->setDrive(PxD6Drive::eZ, m_zDrive);
    }
}
#pragma endregion
void Joint::Unlink()
{
    if (m_joint)
        m_joint->release();
    m_joint = nullptr;
    m_linkedEntity = Entity();
}
bool Joint::Linked()
{
    return m_joint;
}
bool Joint::SafetyCheck()
{
    if (!m_linkedEntity.HasPrivateComponent<RigidBody>())
    {
        UNIENGINE_ERROR("Linked Entity doesn't contains RigidBody component!");
        return false;
    }
    if (m_linkedEntity.GetPrivateComponent<RigidBody>()->m_static &&
        GetOwner().GetPrivateComponent<RigidBody>()->m_static)
    {
        UNIENGINE_ERROR("At least one side of the joint is movable!");
        return false;
    }
    return true;
}
void Joint::Init()
{
    const auto owner = GetOwner();
    if (!owner.HasPrivateComponent<RigidBody>())
    {
        owner.SetPrivateComponent<RigidBody>();
    }
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
    auto storedEntity = m_linkedEntity;
    if (EditorManager::DragAndDrop(m_linkedEntity))
    {
        if (storedEntity != m_linkedEntity)
        {
            storedEntity = m_linkedEntity;
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

Joint::~Joint()
{
    Unlink();
}
void Joint::Link(const Entity &targetEntity)
{
    Unlink();
    m_linkedEntity = targetEntity;
    if (m_linkedEntity.IsValid() && !m_linkedEntity.HasPrivateComponent<RigidBody>())
    {
        m_linkedEntity = Entity();
    }
    if (m_linkedEntity.IsNull())
    {
        return;
    }
    if (SafetyCheck())
    {
        const auto owner = GetOwner();
        PxTransform localFrame1;
        auto ownerGT = owner.GetComponentData<GlobalTransform>();
        ownerGT.SetScale(glm::vec3(1.0f));
        auto linkerGT = m_linkedEntity.GetComponentData<GlobalTransform>();
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
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Distance:
            m_joint = PxDistanceJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Spherical: {
            m_joint = PxSphericalJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)),
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
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
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::Prismatic:
            m_joint = PxPrismaticJointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
            break;
        case JointType::D6:
            m_joint = PxD6JointCreate(
                *PhysicsManager::GetInstance().m_physics,
                m_linkedEntity.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position0.x, position0.y, position0.z),
                    PxQuat(rotation0.x, rotation0.y, rotation0.z, rotation0.w)),
                owner.GetPrivateComponent<RigidBody>()->m_rigidActor,
                PxTransform(
                    PxVec3(position1.x, position1.y, position1.z),
                    PxQuat(rotation1.x, rotation1.y, rotation1.z, rotation1.w)));
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
        Link(m_linkedEntity);
    }
}
