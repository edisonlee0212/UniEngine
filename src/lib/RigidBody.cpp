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
    if (meshRenderer)
    {
        auto bound = meshRenderer->m_mesh->GetBound();
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
    }
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
}

void UniEngine::RigidBody::SetShapeType(ShapeType type)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    if (m_shapeType == type)
        return;
    m_shapeType = type;
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetShapeParam(glm::vec3 value)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    if (m_shapeParam == value)
        return;
    m_shapeParam = value;
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetStatic(bool value)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    m_isStatic = value;
    m_shapeUpdated = false;
    UpdateBody();
}

void UniEngine::RigidBody::SetTransform(glm::mat4 value)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::SetDensity(float value)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    if (m_density == value)
        return;
    m_density = value;
    if (m_density < 0.0001f)
        m_density = 0.0001f;
    m_shapeUpdated = false;
}

UniEngine::RigidBody::~RigidBody()
{
    if (m_rigidActor)
    {
        m_rigidActor->release();
    }
    if (m_material && m_material != PhysicsManager::GetInstance().m_defaultMaterial)
    {
        m_material->release();
    }
    if (m_shape)
    {
        m_shape->release();
    }
}

void UniEngine::RigidBody::SetMaterial(PxMaterial *value)
{
    if (value && m_material != value)
    {
        if (m_material && m_material != PhysicsManager::GetInstance().m_defaultMaterial)
        {
            m_material->release();
        }
        m_material = value;
        m_shapeUpdated = false;
    }
}

void UniEngine::RigidBody::UpdateBody()
{
    if (m_rigidActor)
        m_rigidActor->release();
    PxTransform localTm(PxVec3(0, 0, 0));
    if (m_isStatic)
        m_rigidActor = PhysicsManager::GetInstance().m_physics->createRigidStatic(PxTransform(localTm));
    else
        m_rigidActor = PhysicsManager::GetInstance().m_physics->createRigidDynamic(PxTransform(localTm));
    m_currentRegistered = false;
    m_shapeUpdated = false;
}

void UniEngine::RigidBody::Init()
{
    m_material = PhysicsManager::GetInstance().m_defaultMaterial;
    m_rigidActor = PhysicsManager::GetInstance().m_physics->createRigidDynamic(PxTransform(PxVec3(0, 0, 0)));
    PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidActor), m_density);
    SetEnabled(false);
}

static const char *RigidBodyShapeShape[]{"Sphere", "Box", "Capsule"};
void UniEngine::RigidBody::OnGui()
{
    ImGui::Checkbox("Draw bounds", &m_drawBounds);
    static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
    if (m_drawBounds)
        ImGui::ColorEdit4("Color:##SkinnedMeshRenderer", (float *)(void *)&displayBoundColor);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!m_isStatic)
    {
        PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidActor);
        if (Application::IsPlaying())
        {
            m_linearVelocity = rigidBody->getLinearVelocity();
            m_angularVelocity = rigidBody->getAngularVelocity();
        }
        if (ImGui::DragFloat3("Angular V", &m_angularVelocity.x, 0.01f))
        {
            rigidBody->setAngularVelocity(m_angularVelocity);
        }
        if (ImGui::DragFloat3("Linear V", &m_linearVelocity.x, 0.01f))
        {
            rigidBody->setLinearVelocity(m_linearVelocity);
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
        bool savedVal = m_isStatic;
        ImGui::Checkbox("Static", &m_isStatic);
        if (m_isStatic != savedVal)
        {
            statusChanged = true;
            staticChanged = true;
        }
        ImGui::Combo(
            "Shape", reinterpret_cast<int *>(&m_shapeType), RigidBodyShapeShape, IM_ARRAYSIZE(RigidBodyShapeShape));
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
                    EditorManager::GetSceneCamera().get(),
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
                    EditorManager::GetSceneCamera().get(),
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
                    EditorManager::GetSceneCamera().get(),
                    displayBoundColor,
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam))),
                    1);
            break;
        }
        if (ImGui::DragFloat("Density", &m_density, 0.1f, 0.001f))
            statusChanged = true;
        if (statusChanged)
        {
            m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
            m_shapeUpdated = false;
        }
        if (staticChanged)
        {
            UpdateBody();
        }
    }
}
