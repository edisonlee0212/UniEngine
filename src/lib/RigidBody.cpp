#include "Editor.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include <Application.hpp>
#include <PhysicsLayer.hpp>
#include <RigidBody.hpp>
#include <Transform.hpp>
using namespace UniEngine;

void RigidBody::SetStatic(bool value)
{
    if (m_static == value)
        return;
    if (value)
    {
        SetKinematic(false);
    }
    m_static = value;
    RecreateBody();
}

void RigidBody::SetShapeTransform(const glm::mat4 &value)
{
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
}

void RigidBody::OnDestroy()
{
    while (!m_colliders.empty())
        DetachCollider(0);
    if (m_rigidActor)
    {
        m_rigidActor->release();
        m_rigidActor = nullptr;
    }
}

void RigidBody::RecreateBody()
{
	const auto physicsLayer = Application::GetLayer<PhysicsLayer>();
    if (!physicsLayer)
        return;
    if (m_rigidActor)
        m_rigidActor->release();
    auto scene = GetScene();
    auto owner = GetOwner();
    GlobalTransform globalTransform;
    if (owner.GetIndex() != 0)
    {
        globalTransform = scene->GetDataComponent<GlobalTransform>(owner);
        globalTransform.m_value = globalTransform.m_value * m_shapeTransform;
        globalTransform.SetScale(glm::vec3(1.0f));
    }
    if (m_static)
        m_rigidActor =
            physicsLayer->m_physics->createRigidStatic(PxTransform(*static_cast<PxMat44*>(static_cast<void*>(&globalTransform.m_value))));
    else
        m_rigidActor =
            physicsLayer->m_physics->createRigidDynamic(PxTransform(*static_cast<PxMat44*>(static_cast<void*>(&globalTransform.m_value))));

    if (!m_static)
    {
	    const auto rigidDynamic = static_cast<PxRigidDynamic *>(m_rigidActor);
        rigidDynamic->setSolverIterationCounts(m_minPositionIterations, m_minVelocityIterations);
        PxRigidBodyExt::updateMassAndInertia(*rigidDynamic, m_density, &m_massCenter);
        rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_kinematic);
        if (!m_kinematic)
        {
            rigidDynamic->setLinearDamping(m_linearDamping);
            rigidDynamic->setAngularDamping(m_angularDamping);
            rigidDynamic->setLinearVelocity(m_linearVelocity);
            rigidDynamic->setAngularVelocity(m_angularVelocity);
            m_rigidActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !m_gravity);
        }
    }
    m_currentRegistered = false;
}

void RigidBody::OnCreate()
{
    RecreateBody();
}

void RigidBody::OnInspect()
{
    if (ImGui::TreeNodeEx("Colliders"))
    {
        int index = 0;
        for (auto &i : m_colliders)
        {
            Editor::DragAndDropButton<Collider>(i, ("Collider " + std::to_string(index++)));
        }
        ImGui::TreePop();
    }

    ImGui::Checkbox("Draw bounds", &m_drawBounds);
    static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
    if (m_drawBounds)
        ImGui::ColorEdit4("Color:##SkinnedMeshRenderer", (float *)(void *)&displayBoundColor);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    glm::ivec2 iterations = glm::ivec2(m_minPositionIterations, m_minVelocityIterations);
    if (ImGui::DragInt2("Solver iterations (P/V)", &iterations.x, 1, 0, 128))
    {
        SetSolverIterations(iterations.x, iterations.y);
    }
    if (!m_static)
    {
        const auto rigidDynamic = static_cast<PxRigidDynamic*>(m_rigidActor);
        if (ImGui::Checkbox("Kinematic", &m_kinematic))
        {
            const bool newVal = m_kinematic;
            m_kinematic = !m_kinematic;
            SetKinematic(newVal);
        }
        if (ImGui::DragFloat("Density", &m_density, 0.1f, 0.001f))
        {
            m_density = glm::max(0.001f, m_density);
            PxRigidBodyExt::updateMassAndInertia(
                *rigidDynamic, m_density, &m_massCenter);
        }
        if (ImGui::DragFloat3("Center", &m_massCenter.x, 0.1f, 0.001f))
        {
            PxRigidBodyExt::updateMassAndInertia(
                *rigidDynamic, m_density, &m_massCenter);
        }
        if (!m_kinematic)
        {
            if (Application::IsPlaying())
            {
                m_linearVelocity = rigidDynamic->getLinearVelocity();
                m_angularVelocity = rigidDynamic->getAngularVelocity();
            }
            if (ImGui::DragFloat3("Linear Velocity", &m_linearVelocity.x, 0.01f))
            {
                rigidDynamic->setLinearVelocity(m_linearVelocity);
            }
            if (ImGui::DragFloat("Linear Damping", &m_linearDamping, 0.01f))
            {
                rigidDynamic->setLinearDamping(m_linearDamping);
            }
            if (ImGui::DragFloat3("Angular Velocity", &m_angularVelocity.x, 0.01f))
            {
                rigidDynamic->setAngularVelocity(m_angularVelocity);
            }
            if (ImGui::DragFloat("Angular Damping", &m_angularDamping, 0.01f))
            {
                rigidDynamic->setAngularDamping(m_angularDamping);
            }

            static auto applyValue = glm::vec3(0.0f);
            ImGui::DragFloat3("Value", &applyValue.x, 0.01f);
            if (ImGui::Button("Apply force"))
            {
                AddForce(applyValue);
            }
            if (ImGui::Button("Apply torque"))
            {
                AddForce(applyValue);
            }
        }
    }
    bool staticChanged = false;
    bool savedVal = m_static;
    if (!m_kinematic)
    {
        ImGui::Checkbox("Static", &m_static);
        if (m_static != savedVal)
        {
            staticChanged = true;
        }
    }
    {
        glm::vec3 scale;
        glm::vec3 trans;
        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m_shapeTransform, scale, rotation, trans, skew, perspective);
        skew = glm::degrees(glm::eulerAngles(rotation));
        bool shapeTransChanged = false;
        if (ImGui::DragFloat3("Center Position", &trans.x, 0.01f))
            shapeTransChanged = true;
        if (ImGui::DragFloat3("Rotation", &skew.x, 0.01f))
            shapeTransChanged = true;
        if (shapeTransChanged)
        {
            auto newValue =
                glm::translate(trans) * glm::mat4_cast(glm::quat(glm::radians(skew))) * glm::scale(glm::vec3(1.0f));
            SetShapeTransform(newValue);
        }
        auto scene = GetScene();
        auto ltw = scene->GetDataComponent<GlobalTransform>(GetOwner());
        ltw.SetScale(glm::vec3(1.0f));
        for (auto &collider : m_colliders)
        {
            switch (collider.Get<Collider>()->m_shapeType)
            {
            case ShapeType::Sphere:
                if (m_drawBounds)
                    Gizmos::DrawGizmoMesh(
                        DefaultResources::Primitives::Sphere,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam.x))),
                        1);
                break;
            case ShapeType::Box:
                if (m_drawBounds)
                    Gizmos::DrawGizmoMesh(
                        DefaultResources::Primitives::Cube,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam))),
                        1);
                break;
            case ShapeType::Capsule:
                if (m_drawBounds)
                    Gizmos::DrawGizmoMesh(
                        DefaultResources::Primitives::Cylinder,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam))),
                        1);
                break;
            }
        }

        if (staticChanged)
        {
            RecreateBody();
        }
    }
}
void RigidBody::SetDensityAndMassCenter(float value, const glm::vec3 &center)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    m_density = value;
    m_massCenter = PxVec3(center.x, center.y, center.z);
    PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidActor), m_density, &m_massCenter);
}

void RigidBody::SetAngularVelocity(const glm::vec3 &velocity)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    const auto rigidDynamic = static_cast<PxRigidDynamic*>(m_rigidActor);
    m_angularVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    rigidDynamic->setAngularVelocity(m_angularVelocity);
}
void RigidBody::SetLinearVelocity(const glm::vec3 &velocity)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    const auto rigidDynamic = static_cast<PxRigidDynamic*>(m_rigidActor);
    m_linearVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    rigidDynamic->setLinearVelocity(m_linearVelocity);
}

bool RigidBody::IsKinematic() const
{
    return m_kinematic;
}

void RigidBody::SetKinematic(bool value)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    m_kinematic = value;
    static_cast<PxRigidBody *>(m_rigidActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_kinematic);
}
bool RigidBody::IsStatic()
{
    return m_static;
}
void RigidBody::SetLinearDamping(float value)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_linearDamping = value;
    rigidBody->setLinearDamping(m_linearDamping);
}
void RigidBody::SetAngularDamping(float value)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_angularDamping = value;
    rigidBody->setAngularDamping(m_angularDamping);
}
void RigidBody::SetSolverIterations(unsigned position, unsigned velocity)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_minPositionIterations = position;
    m_minVelocityIterations = velocity;
    rigidBody->setSolverIterationCounts(m_minPositionIterations, m_minVelocityIterations);
}
void RigidBody::SetEnableGravity(bool value)
{
    m_gravity = value;
    m_rigidActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !value);
}
void RigidBody::AttachCollider(std::shared_ptr<Collider> &collider)
{
    collider->m_attachCount++;
    m_colliders.emplace_back(collider);
    if (m_rigidActor)
        m_rigidActor->attachShape(*collider->m_shape);
}
void RigidBody::DetachCollider(size_t index)
{
    if (m_rigidActor)
        m_rigidActor->detachShape(*m_colliders[index].Get<Collider>()->m_shape);
    m_colliders[index].Get<Collider>()->m_attachCount--;
    m_colliders.erase(m_colliders.begin() + index);
}
void RigidBody::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_shapeTransform" << YAML::Value << m_shapeTransform;
    out << YAML::Key << "m_drawBounds" << YAML::Value << m_drawBounds;
    out << YAML::Key << "m_static" << YAML::Value << m_static;
    out << YAML::Key << "m_density" << YAML::Value << m_density;
    out << YAML::Key << "m_massCenter" << YAML::Value << m_massCenter;
    out << YAML::Key << "m_linearVelocity" << YAML::Value << m_linearVelocity;
    out << YAML::Key << "m_angularVelocity" << YAML::Value << m_angularVelocity;
    out << YAML::Key << "m_kinematic" << YAML::Value << m_kinematic;
    out << YAML::Key << "m_linearDamping" << YAML::Value << m_linearDamping;
    out << YAML::Key << "m_angularDamping" << YAML::Value << m_angularDamping;
    out << YAML::Key << "m_minPositionIterations" << YAML::Value << m_minPositionIterations;
    out << YAML::Key << "m_minVelocityIterations" << YAML::Value << m_minVelocityIterations;
    out << YAML::Key << "m_gravity" << YAML::Value << m_gravity;

    if (!m_colliders.empty())
    {
        out << YAML::Key << "m_colliders" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < m_colliders.size(); i++)
        {
            out << YAML::BeginMap;
            m_colliders[i].Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}
void RigidBody::Deserialize(const YAML::Node &in)
{
    m_shapeTransform = in["m_shapeTransform"].as<glm::mat4>();
    m_drawBounds = in["m_drawBounds"].as<bool>();
    m_static = in["m_static"].as<bool>();
    m_density = in["m_density"].as<float>();
    m_massCenter = in["m_massCenter"].as<PxVec3>();
    m_linearVelocity = in["m_linearVelocity"].as<PxVec3>();
    m_angularVelocity = in["m_angularVelocity"].as<PxVec3>();
    m_kinematic = in["m_kinematic"].as<bool>();
    m_linearDamping = in["m_linearDamping"].as<float>();
    m_angularDamping = in["m_angularDamping"].as<float>();
    m_minPositionIterations = in["m_minPositionIterations"].as<unsigned>();
    m_minVelocityIterations = in["m_minVelocityIterations"].as<unsigned>();
    m_gravity = in["m_gravity"].as<bool>();
    RecreateBody();
    auto inColliders = in["m_colliders"];
    if (inColliders)
    {
        for (const auto &i : inColliders)
        {
            AssetRef ref;
            ref.Deserialize(i);
            auto collider = ref.Get<Collider>();
            AttachCollider(collider);
        }
    }
}
void RigidBody::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
    auto ptr = std::static_pointer_cast<RigidBody>(target);
    m_colliders.clear();
    m_shapeTransform = ptr->m_shapeTransform;
    m_drawBounds = ptr->m_drawBounds;
    m_density = ptr->m_density;
    m_massCenter = ptr->m_massCenter;
    m_static = ptr->m_static;
    m_density = ptr->m_density;
    m_massCenter = ptr->m_massCenter;
    m_linearVelocity = ptr->m_linearVelocity;
    m_angularVelocity = ptr->m_angularVelocity;
    m_kinematic = ptr->m_kinematic;
    m_linearDamping = ptr->m_linearDamping;
    m_angularDamping = ptr->m_angularDamping;
    m_minPositionIterations = ptr->m_minPositionIterations;
    m_minVelocityIterations = ptr->m_minVelocityIterations;
    m_gravity = ptr->m_gravity;
    RecreateBody();
    for (auto &i : ptr->m_colliders)
    {
        auto collider = i.Get<Collider>();
        AttachCollider(collider);
    }
}
void RigidBody::CollectAssetRef(std::vector<AssetRef> &list)
{
    for (const auto &i : m_colliders)
        list.push_back(i);
}
bool RigidBody::Registered() const
{
    return m_currentRegistered;
}
void RigidBody::AddForce(const glm::vec3 &force)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    if (m_kinematic)
    {
        UNIENGINE_ERROR("RigidBody is kinematic!");
        return;
    }
    auto *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
    auto pxForce = PxVec3(force.x, force.y, force.z);
    rigidBody->addForce(pxForce, PxForceMode::eFORCE);
}
void RigidBody::AddTorque(const glm::vec3 &torque)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    if (m_kinematic)
    {
        UNIENGINE_ERROR("RigidBody is kinematic!");
        return;
    }
    auto *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
    auto pxTorque = PxVec3(torque.x, torque.y, torque.z);
    rigidBody->addTorque(pxTorque, PxForceMode::eFORCE);
}
