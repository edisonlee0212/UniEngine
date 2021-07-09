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
    if (!m_linkedEntity.HasPrivateComponent<RigidBody>())
    {
        m_linkedEntity = Entity();
        return;
    }
    if (SafetyCheck())
    {
        const auto owner = GetOwner();
        PxTransform localFrame1;
        auto ownerGT = owner.GetComponentData<GlobalTransform>();
        auto linkerGT = m_linkedEntity.GetComponentData<GlobalTransform>();
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
}
D6Joint::~D6Joint()
{
    Unlink();
}
void D6Joint::OnGui()
{
    if (EditorManager::DragAndDrop(m_linkedEntity))
    {
        Link();
    }
}
