#include <DistanceJoint.hpp>
#include <EditorManager.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>m
using namespace UniEngine;
void DistanceJoint::SetMax(const float &value, const bool& enabled)
{
    if(!m_joint) return;
    if(m_maxDistance != value || m_maxDistanceEnabled != enabled){
        m_maxDistance = value;
        m_maxDistanceEnabled = enabled;
        m_joint->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, m_maxDistanceEnabled);
        m_joint->setMaxDistance(m_maxDistance);
    }
}

void DistanceJoint::SetMin(const float &value, const bool &enabled)
{
    if(!m_joint) return;
    if(m_minDistance != value || m_maxDistanceEnabled != enabled){
        m_minDistance = value;
        m_minDistanceEnabled = enabled;
        m_joint->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, m_minDistanceEnabled);
        m_joint->setMinDistance(m_minDistance);
    }
}

void DistanceJoint::SetStiffness(const float &value)
{
    if(!m_joint) return;
    if(m_stiffness != value){
        m_stiffness = value;
        m_joint->setStiffness(m_stiffness);
    }
}


void DistanceJoint::Unlink()
{
    if (m_joint)
        m_joint->release();
    m_joint = nullptr;
}
bool DistanceJoint::Linked()
{
    return m_joint;
}
bool DistanceJoint::SafetyCheck()
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
void DistanceJoint::Init()
{
    const auto owner = GetOwner();
    if (!owner.HasPrivateComponent<RigidBody>())
    {
        owner.SetPrivateComponent<RigidBody>(std::make_unique<RigidBody>());
    }
}
void DistanceJoint::Link()
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
        Unlink();
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
        m_joint->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, true);
        SetMax(0, true);
        SetMin(0, true);
        SetStiffness(10);
        SetDamping(1);
    }
}
void DistanceJoint::OnGui()
{
    auto storedEntity = m_linkedEntity;
    if (EditorManager::DragAndDrop(m_linkedEntity))
    {
        if(storedEntity != m_linkedEntity) Link();
    }
    if(ImGui::DragFloat("Min", &m_minDistance, 0.1f, FLT_MIN, m_maxDistance)){
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Enabled", &m_minDistanceEnabled)){
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    if(ImGui::DragFloat("Max", &m_maxDistance, 0.1f, m_minDistance, FLT_MAX)){
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }
    ImGui::SameLine();
    if(ImGui::Checkbox("Enabled", &m_maxDistanceEnabled)){
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }

    if(ImGui::DragFloat("Stiffness", &m_stiffness)){
        SetStiffness(m_stiffness);
    }
    if(ImGui::DragFloat("Damping", &m_damping)){
        SetDamping(m_damping);
    }
}
DistanceJoint::~DistanceJoint()
{
    Unlink();
}
void DistanceJoint::SetDamping(const float &value)
{
    if(!m_joint) return;
    if(m_damping != value){
        m_damping = value;
        m_joint->setDamping(m_damping);
    }
}
