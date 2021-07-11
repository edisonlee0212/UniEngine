#include <EditorManager.hpp>
#include <FixedJoint.hpp>
#include <PhysicsManager.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;
void UniEngine::FixedJoint::Unlink()
{
    if (m_joint)
        m_joint->release();
    m_joint = nullptr;
}
bool FixedJoint::Linked()
{
    return m_joint;
}
bool FixedJoint::SafetyCheck()
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
void FixedJoint::Init()
{
    const auto owner = GetOwner();
    if (!owner.HasPrivateComponent<RigidBody>())
    {
        owner.SetPrivateComponent<RigidBody>(std::make_unique<RigidBody>());
    }
}
void FixedJoint::Link()
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
    }
}
void FixedJoint::OnGui()
{
    auto storedEntity = m_linkedEntity;
    if (EditorManager::DragAndDrop(m_linkedEntity))
    {
        if(storedEntity != m_linkedEntity) Link();
    }
    if(m_linkedEntity.IsValid()){
        if(!m_linkedEntity.HasPrivateComponent<RigidBody>()) m_linkedEntity = Entity();
    }
}
FixedJoint::~FixedJoint()
{
    Unlink();
}
