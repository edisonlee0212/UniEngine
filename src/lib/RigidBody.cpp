#include <Application.hpp>
#include <EditorManager.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
#include <RigidBody.hpp>
#include <Transform.hpp>
using namespace UniEngine;

void UniEngine::RigidBody::SetStatic(bool value)
{
    if (value)
    {
        SetKinematic(false);
    }
    m_static = value;
    UpdateBody();
}

void UniEngine::RigidBody::SetShapeTransform(const glm::mat4 &value)
{
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
}

void UniEngine::RigidBody::OnDestroy()
{
    while (!m_colliders.empty())
        DetachCollider(0);
    if (m_rigidActor)
    {
        m_rigidActor->release();
    }
}

void UniEngine::RigidBody::UpdateBody()
{
    if (m_rigidActor)
        m_rigidActor->release();
    PxTransform localTm(PxVec3(0, 0, 0));
    if (m_static)
        m_rigidActor = PhysicsManager::GetInstance().m_physics->createRigidStatic(PxTransform(localTm));
    else
        m_rigidActor = PhysicsManager::GetInstance().m_physics->createRigidDynamic(PxTransform(localTm));
    m_currentRegistered = false;
    if (!m_static && !m_kinematic)
    {
        PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
        rigidBody->setLinearDamping(m_linearDamping);
        rigidBody->setAngularVelocity(m_angularVelocity);
    }
}

void UniEngine::RigidBody::OnCreate()
{
    UpdateBody();
    PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidActor), m_density, &m_massCenter);
}

void UniEngine::RigidBody::OnGui()
{
    if (ImGui::TreeNodeEx("Colliders"))
    {
        int index = 0;
        for (auto &i : m_colliders)
        {
            EditorManager::DragAndDropButton<Collider>(i, ("Collider " + std::to_string(index++)));
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
        if (ImGui::Checkbox("Kinematic", &m_kinematic))
        {
            const bool newVal = m_kinematic;
            m_kinematic = !m_kinematic;
            SetKinematic(newVal);
        }
        if (!m_kinematic)
        {
            PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
            if (Application::IsPlaying())
            {
                m_linearVelocity = rigidBody->getLinearVelocity();
                m_angularVelocity = rigidBody->getAngularVelocity();
            }
            if (ImGui::DragFloat3("Linear Velocity", &m_linearVelocity.x, 0.01f))
            {
                rigidBody->setLinearVelocity(m_linearVelocity);
            }
            if (ImGui::DragFloat("Linear Damping", &m_linearDamping, 0.01f))
            {
                rigidBody->setLinearDamping(m_linearDamping);
            }
            if (ImGui::DragFloat3("Angular Velocity", &m_angularVelocity.x, 0.01f))
            {
                rigidBody->setAngularVelocity(m_angularVelocity);
            }
            if (ImGui::DragFloat("Angular Damping", &m_angularDamping, 0.01f))
            {
                rigidBody->setAngularDamping(m_angularDamping);
            }
        }
    }
    bool statusChanged = false;
    bool staticChanged = false;
    bool savedVal = m_static;
    if (!m_kinematic)
    {
        ImGui::Checkbox("Static", &m_static);
        if (m_static != savedVal)
        {
            statusChanged = true;
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
            m_shapeTransform =
                glm::translate(trans) * glm::mat4_cast(glm::quat(glm::radians(skew))) * glm::scale(glm::vec3(1.0f));

        auto ltw = GetOwner().GetDataComponent<GlobalTransform>();
        ltw.SetScale(glm::vec3(1.0f));
        for (auto &collider : m_colliders)
        {
            switch (collider.Get<Collider>()->m_shapeType)
            {
            case ShapeType::Sphere:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Sphere,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam.x))),
                        1);
                break;
            case ShapeType::Box:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Cube,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam))),
                        1);
                break;
            case ShapeType::Capsule:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Cylinder,
                        displayBoundColor,
                        ltw.m_value *
                            (m_shapeTransform * glm::scale(glm::vec3(collider.Get<Collider>()->m_shapeParam))),
                        1);
                break;
            }
        }
        if (ImGui::DragFloat("Density", &m_density, 0.1f, 0.001f))
            statusChanged = true;
        if (ImGui::DragFloat3("Center", &m_massCenter.x, 0.1f, 0.001f))
            statusChanged = true;
        if (statusChanged)
        {
            m_density = glm::max(0.001f, m_density);
        }
        if (staticChanged)
        {
            UpdateBody();
        }
    }
}
void RigidBody::SetDensityAndMassCenter(float value, const glm::vec3 &center)
{
    m_density = value;
    m_massCenter = PxVec3(center.x, center.y, center.z);
    if (!m_static)
        PxRigidBodyExt::updateMassAndInertia(
            *reinterpret_cast<PxRigidDynamic *>(m_rigidActor), m_density, &m_massCenter);
}

void RigidBody::SetAngularVelocity(const glm::vec3 &velocity)
{
    PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
    m_angularVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    rigidBody->setAngularVelocity(m_angularVelocity);
}
void RigidBody::SetLinearVelocity(const glm::vec3 &velocity)
{
    PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
    m_linearVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    rigidBody->setLinearVelocity(m_linearVelocity);
}

bool RigidBody::IsKinematic()
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
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_linearDamping = value;
    rigidBody->setLinearDamping(m_linearDamping);
}
void RigidBody::SetAngularDamping(float value)
{
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_angularDamping = value;
    rigidBody->setAngularDamping(m_angularDamping);
}
void RigidBody::SetSolverIterations(unsigned position, unsigned velocity)
{
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
    if (collider->m_attached)
    {
        UNIENGINE_ERROR("Collider already attached to a RigidBody!");
        return;
    }
    m_colliders.emplace_back(collider);
    if (m_rigidActor)
        m_rigidActor->attachShape(*collider->m_shape);
}
void RigidBody::DetachCollider(size_t index)
{
    if (m_rigidActor)
        m_rigidActor->detachShape(*m_colliders[index].Get<Collider>()->m_shape);
    m_colliders[index].Get<Collider>()->m_attached = false;
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
    auto isStatic = in["m_static"].as<bool>();
    m_density = in["m_density"].as<float>();
    m_massCenter = in["m_massCenter"].as<PxVec3>();
    m_linearVelocity = in["m_linearVelocity"].as<PxVec3>();
    m_angularVelocity = in["m_angularVelocity"].as<PxVec3>();
    auto kinematic = in["m_kinematic"].as<bool>();
    m_linearDamping = in["m_linearDamping"].as<float>();
    m_angularDamping = in["m_angularDamping"].as<float>();
    m_minPositionIterations = in["m_minPositionIterations"].as<unsigned>();
    m_minVelocityIterations = in["m_minVelocityIterations"].as<unsigned>();
    m_gravity = in["m_gravity"].as<bool>();

    SetShapeTransform(m_shapeTransform);
    SetSolverIterations(m_minPositionIterations, m_minVelocityIterations);
    SetAngularDamping(m_angularDamping);
    SetLinearDamping(m_linearDamping);
    SetEnableGravity(m_gravity);
    if (kinematic)
        SetKinematic(kinematic);
    if (isStatic)
        SetStatic(isStatic);
    if (!kinematic && !isStatic)
    {
        SetLinearVelocity(glm::vec3(m_linearVelocity.x, m_linearVelocity.y, m_linearVelocity.z));
        SetAngularVelocity(glm::vec3(m_angularVelocity.x, m_angularVelocity.y, m_angularVelocity.z));
    }
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
void RigidBody::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    auto ptr = std::static_pointer_cast<RigidBody>(target);
    *this = *ptr;
    m_rigidActor = nullptr;
    m_currentRegistered = false;
}
void RigidBody::CollectAssetRef(std::vector<AssetRef> &list)
{
    for (const auto &i : m_colliders)
        list.push_back(i);
}
