#include <Application.hpp>
#include <EditorManager.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
#include <RigidBody.hpp>
#include <Transform.hpp>
using namespace UniEngine;

void UniEngine::RigidBody::ApplyMeshBound()
{
    if (!GetOwner().IsValid() || !GetOwner().HasPrivateComponent<MeshRenderer>())
        return;
    auto &meshRenderer = GetOwner().GetPrivateComponent<MeshRenderer>();
    auto bound = meshRenderer.m_mesh->GetBound();
    glm::vec3 scale = GetOwner().GetComponentData<GlobalTransform>().GetScale();
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        scale = bound.Size() * scale;
        m_shapeParam = glm::vec3((scale.x + scale.y + scale.z) / 3.0f, 1.0f, 1.0f);
        break;
    case ShapeType::Box:
        m_shapeParam = bound.Size() * scale;
        break;
    case ShapeType::Capsule:
        m_shapeParam = bound.Size() * scale;
        break;
    }
    m_density = glm::max(0.001f, m_density);
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetShapeType(ShapeType type)
{
    if (m_shapeType == type)
        return;
    m_shapeType = type;
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetShapeParam(glm::vec3 value)
{
    if (m_shapeParam == value)
        return;
    m_shapeParam = value;
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetStatic(bool value)
{
    if (m_static == value)
        return;
    if (value)
    {
        auto linkedEntities = m_linkedEntities;
        for(auto& i : linkedEntities){
            i.GetPrivateComponent<Joint>().Unlink();
        }
        SetKinematic(false);
    }
    m_static = value;
    m_shapeUpdated = false;

    UpdateBody();
}

void UniEngine::RigidBody::SetShapeTransform(glm::mat4 value)
{
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::OnDestroy()
{
    auto linkedEntities = m_linkedEntities;
    for(auto& i : linkedEntities){
        i.GetPrivateComponent<Joint>().Unlink();
    }

    if (m_rigidActor)
    {
        m_rigidActor->release();
    }
    if (m_shape)
    {
        m_shape->release();
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
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::OnCreate()
{
    m_material = PhysicsManager::GetInstance().m_defaultMaterial;
    UpdateBody();
    PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidActor), m_density, &m_massCenter);
    SetEnabled(false);
}

static const char *RigidBodyShape[]{"Sphere", "Box", "Capsule"};
void UniEngine::RigidBody::OnGui()
{
    if (ImGui::TreeNode("Material"))
    {
        m_material->OnGui();
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
    if (Application::IsPlaying())
    {
        ImGui::Text("Pause Engine to edit shape.");
    }
    else
    {
        bool statusChanged = false;
        bool staticChanged = false;
        bool savedVal = m_static;
        ImGui::Checkbox("Static", &m_static);
        if (m_static != savedVal)
        {
            statusChanged = true;
            staticChanged = true;
        }
        ImGui::Combo(
            "Shape", reinterpret_cast<int *>(&m_shapeType), RigidBodyShape, IM_ARRAYSIZE(RigidBodyShape));
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

        auto ltw = GetOwner().GetComponentData<GlobalTransform>();
        ltw.SetScale(glm::vec3(1.0f));

        switch (m_shapeType)
        {
        case ShapeType::Sphere:
            if (ImGui::DragFloat("Radius", &m_shapeParam.x, 0.01f, 0.0001f))
                statusChanged = true;
            if (m_drawBounds)
                RenderManager::DrawGizmoMesh(
                    DefaultResources::Primitives::Sphere.get(),
                    EditorManager::GetSceneCamera(),
                    displayBoundColor,
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam.x))),
                    1);
            break;
        case ShapeType::Box:
            if (GetOwner().HasPrivateComponent<MeshRenderer>())
            {
                if (ImGui::Button("Apply mesh bound"))
                {
                    statusChanged = true;
                    ApplyMeshBound();
                }
            }
            if (ImGui::DragFloat3("XYZ Size", &m_shapeParam.x, 0.01f, 0.0f))
                statusChanged = true;
            if (m_drawBounds)
                RenderManager::DrawGizmoMesh(
                    DefaultResources::Primitives::Cube.get(),
                    EditorManager::GetSceneCamera(),
                    displayBoundColor,
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam))),
                    1);
            break;
        case ShapeType::Capsule:
            if (ImGui::DragFloat2("R/HalfH", &m_shapeParam.x, 0.01f, 0.0001f))
                statusChanged = true;
            if (m_drawBounds)
                RenderManager::DrawGizmoMesh(
                    DefaultResources::Primitives::Cylinder.get(),
                    EditorManager::GetSceneCamera(),
                    displayBoundColor,
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam))),
                    1);
            break;
        }
        if (ImGui::DragFloat("Density", &m_density, 0.1f, 0.001f))
            statusChanged = true;
        if (ImGui::DragFloat3("Center", &m_massCenter.x, 0.1f, 0.001f))
            ;
        if (statusChanged)
        {
            m_density = glm::max(0.001f, m_density);
            m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
            m_shapeUpdated = false;
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
void RigidBody::SetMaterial(const std::shared_ptr<PhysicsMaterial> &value)
{
    if (value && m_material != value)
    {
        m_material = value;
        m_shapeUpdated = false;
    }
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
