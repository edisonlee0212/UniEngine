#include <D6Joint.hpp>
#include <EditorManager.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;
void D6Joint::Init()
{
    const auto owner = GetOwner();
    if (!owner.HasPrivateComponent<RigidBody>())
    {
        owner.SetPrivateComponent<RigidBody>(std::make_unique<RigidBody>());
    }
}

void D6Joint::Link()
{
    if (m_linkedEntity.IsValid() && !m_linkedEntity.HasPrivateComponent<RigidBody>())
    {
        m_linkedEntity = Entity();
    }
    if(m_linkedEntity.IsNull()){
        Unlink();
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
    }
}
bool D6Joint::SafetyCheck()
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
bool D6Joint::Linked()
{
    return m_joint;
}
void D6Joint::Unlink()
{
    if (m_joint)
        m_joint->release();
    m_joint = nullptr;
}
D6Joint::~D6Joint()
{
    Unlink();
}
void D6Joint::OnGui()
{
    auto storedEntity = m_linkedEntity;
    if (EditorManager::DragAndDrop(m_linkedEntity))
    {
        if(storedEntity != m_linkedEntity) Link();
    }
    if (m_joint)
    {
        if (ImGui::Checkbox("Lock X", &m_xLocked))
        {
            m_joint->setMotion(PxD6Axis::eX, m_xLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
        }
        if (!m_xLocked)
        {
            if (ImGui::DragFloat("X Stiffness", &m_xDrive.stiffness))
            {
                m_joint->setDrive(PxD6Drive::eX, m_xDrive);
            }

            if (ImGui::DragFloat("X Damping", &m_xDrive.damping))
            {
                m_joint->setDrive(PxD6Drive::eX, m_xDrive);
            }
            if (ImGui::Checkbox("X is acceleration", &m_xAcceleration))
            {
                m_xDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
                    m_xAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
                m_joint->setDrive(PxD6Drive::eX, m_xDrive);
            }
        }

        if (ImGui::Checkbox("Lock Y", &m_yLocked))
        {
            m_joint->setMotion(PxD6Axis::eY, m_yLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
        }
        if (!m_yLocked)
        {
            if (ImGui::DragFloat("Y Stiffness", &m_yDrive.stiffness))
            {
                m_joint->setDrive(PxD6Drive::eY, m_yDrive);
            }

            if (ImGui::DragFloat("Y Damping", &m_yDrive.damping))
            {
                m_joint->setDrive(PxD6Drive::eY, m_yDrive);
            }
            if (ImGui::Checkbox("Y is acceleration", &m_yAcceleration))
            {
                m_yDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
                    m_yAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
                m_joint->setDrive(PxD6Drive::eY, m_yDrive);
            }
        }
        if (ImGui::Checkbox("Lock Z", &m_zLocked))
        {
            m_joint->setMotion(PxD6Axis::eZ, m_zLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
        }
        if (!m_zLocked)
        {
            if (ImGui::DragFloat("Z Stiffness", &m_zDrive.stiffness))
            {
                m_joint->setDrive(PxD6Drive::eZ, m_zDrive);
            }

            if (ImGui::DragFloat("Z Damping", &m_zDrive.damping))
            {
                m_joint->setDrive(PxD6Drive::eZ, m_zDrive);
            }
            if (ImGui::Checkbox("Z is acceleration", &m_zAcceleration))
            {
                m_zDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
                    m_zAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
                m_joint->setDrive(PxD6Drive::eZ, m_zDrive);
            }
        }
    }
}
void D6Joint::SetLockX(const bool &value)
{
    if(m_xLocked != value){
        m_xLocked = value;
        m_joint->setMotion(PxD6Axis::eX, m_xLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void D6Joint::SetLockY(const bool &value)
{
    if(m_yLocked != value){
        m_yLocked = value;
        m_joint->setMotion(PxD6Axis::eY, m_yLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void D6Joint::SetLockZ(const bool &value)
{
    if(m_zLocked != value){
        m_zLocked = value;
        m_joint->setMotion(PxD6Axis::eZ, m_zLocked ? PxD6Motion::eLOCKED : PxD6Motion::eFREE);
    }
}
void D6Joint::SetDriveX(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if(stiffness != m_xDrive.stiffness || damping != m_xDrive.damping || isAcceleration != (m_xDrive.flags == PxD6JointDriveFlag::eACCELERATION)){
        m_xDrive.stiffness = stiffness;
        m_xDrive.damping = damping;
        m_xAcceleration = isAcceleration;
        m_xDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
            m_xAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        m_joint->setDrive(PxD6Drive::eX, m_xDrive);
    }
}
void D6Joint::SetDriveY(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if(stiffness != m_yDrive.stiffness || damping != m_yDrive.damping || isAcceleration != (m_yDrive.flags == PxD6JointDriveFlag::eACCELERATION)){
        m_yDrive.stiffness = stiffness;
        m_yDrive.damping = damping;
        m_yAcceleration = isAcceleration;
        m_yDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
            m_yAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        m_joint->setDrive(PxD6Drive::eY, m_yDrive);
    }
}
void D6Joint::SetDriveZ(const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if(stiffness != m_zDrive.stiffness || damping != m_zDrive.damping || isAcceleration != (m_zDrive.flags == PxD6JointDriveFlag::eACCELERATION)){
        m_zDrive.stiffness = stiffness;
        m_zDrive.damping = damping;
        m_zAcceleration = isAcceleration;
        m_zDrive.flags = static_cast<PxD6JointDriveFlag::Enum>(
            m_zAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
        m_joint->setDrive(PxD6Drive::eZ, m_zDrive);
    }
}
