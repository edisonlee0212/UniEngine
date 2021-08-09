#include <Application.hpp>
#include <EditorManager.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
#include <RigidBody.hpp>
#include <Transform.hpp>
using namespace UniEngine;

void UniEngine::RigidBody::SetStatic(bool value)
{
    if (m_static == value)
        return;
    if (value)
    {
        auto linkedEntities = m_linkedEntities;
        for(auto& i : linkedEntities){
            i.GetOrSetPrivateComponent<Joint>().lock()->Unlink();
        }
        SetKinematic(false);
    }
    m_static = value;
    UpdateBody();
}

void UniEngine::RigidBody::SetShapeTransform(glm::mat4 value)
{
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
}

void UniEngine::RigidBody::OnDestroy()
{
    auto linkedEntities = m_linkedEntities;
    for(auto& i : linkedEntities){
        i.GetOrSetPrivateComponent<Joint>().lock()->Unlink();
    }

    while(!m_colliders.empty()) DetachCollider(0);
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
    auto linkedEntities = m_linkedEntities;
    for(auto& i : linkedEntities){
        i.GetOrSetPrivateComponent<Joint>().lock()->Unlink();
    }
    if(!m_static && !m_kinematic)
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
    if(!m_kinematic){
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
        for(const auto& collider : m_colliders)
        {
            switch (collider->m_shapeType)
            {
            case ShapeType::Sphere:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Sphere,
                        displayBoundColor,
                        ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(collider->m_shapeParam.x))),
                        1);
                break;
            case ShapeType::Box:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Cube,
                        displayBoundColor,
                        ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(collider->m_shapeParam))),
                        1);
                break;
            case ShapeType::Capsule:
                if (m_drawBounds)
                    RenderManager::DrawGizmoMesh(
                        DefaultResources::Primitives::Cylinder,
                        displayBoundColor,
                        ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(collider->m_shapeParam))),
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
void RigidBody::SetDensityAndMassCenter(const float &value, const glm::vec3 &center)
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

void RigidBody::SetKinematic(const bool &value)
{
    if (m_static)
    {
        UNIENGINE_ERROR("RigidBody is static!");
        return;
    }
    if (m_kinematic == value)
        return;
    m_kinematic = value;
    static_cast<PxRigidBody *>(m_rigidActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, m_kinematic);
    PhysicsManager::UploadTransform(GetOwner().GetDataComponent<GlobalTransform>(), GetOwner().GetOrSetPrivateComponent<RigidBody>().lock());
}
bool RigidBody::IsStatic()
{
    return m_static;
}
void RigidBody::SetLinearDamping(const float &value)
{
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_linearDamping = value;
    rigidBody->setLinearDamping(m_linearDamping);
}
void RigidBody::SetAngularDamping(const float &value)
{
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_angularDamping = value;
    rigidBody->setAngularDamping(m_angularDamping);
}
void RigidBody::SetSolverIterations(const unsigned int &position, const unsigned int &velocity)
{
    PxRigidDynamic *rigidBody = static_cast<PxRigidDynamic *>(m_rigidActor);
    m_minPositionIterations = position;
    m_minVelocityIterations = velocity;
    rigidBody->setSolverIterationCounts(m_minPositionIterations, m_minVelocityIterations);
}
void RigidBody::SetEnableGravity(const bool &value)
{
    m_gravity = value;
    m_rigidActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}
void RigidBody::AttachCollider(std::shared_ptr<Collider> &collider)
{
    if(collider->m_attached){
        UNIENGINE_ERROR("Collider already attached to a RigidBody!");
        return;
    }
    m_colliders.push_back(collider);
    m_rigidActor->attachShape(*collider->m_shape);
}
void RigidBody::DetachCollider(const size_t &index)
{
    m_rigidActor->detachShape(*m_colliders[index]->m_shape);
    m_colliders[index]->m_attached = false;
    m_colliders.erase(m_colliders.begin() + index);
}
void RigidBody::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_shapeTransform" << YAML::Value << m_shapeTransform;
    out << YAML::Key << "m_drawBounds" << YAML::Value << m_drawBounds;
    out << YAML::Key << "m_static" << YAML::Value << m_static;
    out << YAML::Key << "m_density" << YAML::Value << m_density;
    out << YAML::Key << "m_massCenter" << YAML::Value << m_massCenter;
    out << YAML::Key << "m_currentRegistered" << YAML::Value << m_currentRegistered;
    out << YAML::Key << "m_linearVelocity" << YAML::Value << m_linearVelocity;
    out << YAML::Key << "m_angularVelocity" << YAML::Value << m_angularVelocity;
    out << YAML::Key << "m_kinematic" << YAML::Value << m_kinematic;
    out << YAML::Key << "m_linearDamping" << YAML::Value << m_linearDamping;
    out << YAML::Key << "m_angularDamping" << YAML::Value << m_angularDamping;
    out << YAML::Key << "m_minPositionIterations" << YAML::Value << m_minPositionIterations;
    out << YAML::Key << "m_minVelocityIterations" << YAML::Value << m_minVelocityIterations;
    out << YAML::Key << "m_gravity" << YAML::Value << m_gravity;
}
void RigidBody::Deserialize(const YAML::Node &in)
{
    m_shapeTransform = in["m_shapeTransform"].as<glm::mat4>();
}
