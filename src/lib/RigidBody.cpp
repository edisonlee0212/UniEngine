#include <RigidBody.hpp>

#include "PhysicsManager.hpp"
#include <RenderManager.hpp>
#include <Transform.hpp>
#include <EditorManager.hpp>
#include <Application.hpp>
void UniEngine::RigidBody::RegisterCheck()
{
    if (m_currentRegistered == false && GetOwner().IsValid() && GetOwner().IsEnabled() && IsEnabled())
    {
        m_currentRegistered = true;
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
    }
    else if (m_currentRegistered == true && (!GetOwner().IsValid() || !GetOwner().IsEnabled() || !IsEnabled()))
    {
        m_currentRegistered = false;
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    }
}

UniEngine::RigidBody::RigidBody()
{
    m_material = PhysicsSimulationManager::GetInstance().m_defaultMaterial;
    m_shapeTransform =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    m_massCenter = PxVec3(0.0f);
    m_drawBounds = false;
    m_isStatic = false;
    PxTransform localTm(PxVec3(0, 0, 0));
    m_rigidBody = PhysicsSimulationManager::GetInstance().m_physics->createRigidDynamic(PxTransform(localTm));
    m_shapeParam = glm::vec3(1.0f);
    m_shapeType = ShapeType::Box;

    UpdateShape();
    m_density = 10.0f;
    PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    m_currentRegistered = false;
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
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    UpdateShape();
    if (!m_isStatic)
        PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
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
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    UpdateShape();
    if (!m_isStatic)
        PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
}

void UniEngine::RigidBody::SetStatic(bool value)
{
    if (Application::IsPlaying())
    {
        Debug::Log("Failed! Pause game to reset!");
        return;
    }
    if (m_isStatic == value)
        return;
    m_isStatic = value;
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    UpdateBody();
    UpdateShape();
    if (!m_isStatic)
        PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
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
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    UpdateShape();
    if (!m_isStatic)
        PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
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
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
    if (!m_isStatic)
        PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
    if (m_currentRegistered)
        PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
}

UniEngine::RigidBody::~RigidBody()
{
    if (m_rigidBody)
    {
        m_rigidBody->release();
    }
    if (m_material && m_material != PhysicsSimulationManager::GetInstance().m_defaultMaterial)
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
        if (m_material && m_material != PhysicsSimulationManager::GetInstance().m_defaultMaterial)
        {
            m_material->release();
        }
        m_material = value;
        if (m_currentRegistered)
            PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
        UpdateShape();
        if (!m_isStatic)
            PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
        if (m_currentRegistered)
            PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
    }
}

void UniEngine::RigidBody::UpdateShape()
{
    if (m_shape != nullptr)
        m_shape->release();
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        m_shape = PhysicsSimulationManager::GetInstance().m_physics->createShape(
            PxSphereGeometry(m_shapeParam.x), *m_material);
        break;
    case ShapeType::Box:
        m_shape = PhysicsSimulationManager::GetInstance().m_physics->createShape(
            PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z), *m_material);
        break;
    case ShapeType::Capsule:
        m_shape = PhysicsSimulationManager::GetInstance().m_physics->createShape(
            PxCapsuleGeometry(m_shapeParam.x, m_shapeParam.y), *m_material);
        break;
    }
    m_rigidBody->attachShape(*m_shape);
}

void UniEngine::RigidBody::UpdateBody()
{
    if (m_rigidBody)
        m_rigidBody->release();
    PxTransform localTm(PxVec3(0, 0, 0));
    if (m_isStatic)
        m_rigidBody = PhysicsSimulationManager::GetInstance().m_physics->createRigidStatic(PxTransform(localTm));
    else
        m_rigidBody = PhysicsSimulationManager::GetInstance().m_physics->createRigidDynamic(PxTransform(localTm));
}

void UniEngine::RigidBody::Init()
{
    SetEnabled(false);
}

void UniEngine::RigidBody::OnEntityDisable()
{
    RegisterCheck();
}
void UniEngine::RigidBody::OnEntityEnable()
{
    RegisterCheck();
}
void UniEngine::RigidBody::OnDisable()
{
    RegisterCheck();
}

void UniEngine::RigidBody::OnEnable()
{
    RegisterCheck();
}
static const char *RigidBodyShapeShape[]{"Sphere", "Box", "Capsule"};
void UniEngine::RigidBody::OnGui()
{
    ImGui::Checkbox("Draw bounds", &m_drawBounds);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!m_isStatic)
    {
        PxRigidBody *rigidBody = static_cast<PxRigidBody *>(m_rigidBody);
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
                    glm::vec4(0.0f, 0.0f, 1.0f, 0.5f),
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam.x * 2.0f))),
                    1);
            break;
        case ShapeType::Box:
            if (ImGui::Button("Apply mesh bound"))
            {
                statusChanged = true;
                auto &meshRenderer = GetOwner().GetPrivateComponent<MeshRenderer>();
                if (meshRenderer)
                {
                    auto bound = meshRenderer->m_mesh->GetBound();
                    glm::vec3 scale = GetOwner().GetComponentData<GlobalTransform>().GetScale();
                    m_shapeParam = bound.Size() * scale;
                    m_shapeParam = glm::max(glm::vec3(0.01f), m_shapeParam);
                }
            }
            if (ImGui::DragFloat3("XYZ Size", &m_shapeParam.x, 0.01f, 0.0f))
                statusChanged = true;
            if (m_drawBounds)
                RenderManager::DrawGizmoMesh(
                    DefaultResources::Primitives::Cube.get(),
                    EditorManager::GetSceneCamera().get(),
                    glm::vec4(0.0f, 0.0f, 1.0f, 0.5f),
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
                    glm::vec4(0.0f, 0.0f, 1.0f, 0.5f),
                    ltw.m_value * (m_shapeTransform * glm::scale(glm::vec3(m_shapeParam))),
                    1);
            break;
        }
        if (ImGui::DragFloat("Density", &m_density, 0.1f, 0.001f))
            statusChanged = true;
        if (statusChanged)
        {
            if (m_currentRegistered)
                PhysicsSimulationManager::GetInstance().m_physicsScene->removeActor(*m_rigidBody);
            if (staticChanged)
                UpdateBody();
            UpdateShape();
            if (!m_isStatic)
                PxRigidBodyExt::updateMassAndInertia(*reinterpret_cast<PxRigidDynamic *>(m_rigidBody), m_density);
            if (m_currentRegistered)
                PhysicsSimulationManager::GetInstance().m_physicsScene->addActor(*m_rigidBody);
        }
    }
}
