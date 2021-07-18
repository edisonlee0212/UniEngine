#include <Articulation.hpp>
#include <PhysicsManager.hpp>
#include <Application.hpp>
#include <EditorManager.hpp>
using namespace UniEngine;

void Articulation::OnCreate()
{
    m_material = PhysicsManager::GetInstance().m_defaultMaterial;
    UpdateBody();
    PxRigidBodyExt::updateMassAndInertia(*m_root, m_density, &m_massCenter);
    SetEnabled(false);
}
ArticulationType Articulation::GetType() const
{
    return m_type;
}
void Articulation::SetType(const ArticulationType &mType)
{
    m_type = mType;
}

static const char *RigidBodyShape[]{"Sphere", "Box", "Capsule"};
void Articulation::OnGui()
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

    if (Application::IsPlaying())
    {
        m_linearVelocity = m_root->getLinearVelocity();
        m_angularVelocity = m_root->getAngularVelocity();
    }
    if (ImGui::DragFloat3("Linear Velocity", &m_linearVelocity.x, 0.01f))
    {
        m_root->setLinearVelocity(m_linearVelocity);
    }

    if (Application::IsPlaying())
    {
        ImGui::Text("Pause Engine to edit shape.");
    }
    else
    {
        bool statusChanged = false;
        bool staticChanged = false;

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

        auto ltw = GetOwner().GetDataComponent<GlobalTransform>();
        ltw.SetScale(glm::vec3(1.0f));

        switch (m_shapeType)
        {
        case ShapeType::Sphere:
            if (ImGui::DragFloat("Radius", &m_shapeParam.x, 0.01f, 0.0001f))
                statusChanged = true;
            if (m_drawBounds)
                RenderManager::DrawGizmoMesh(
                    DefaultResources::Primitives::Sphere.get(),
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
void Articulation::SetSolverIterations(const unsigned int &position, const unsigned int &velocity)
{
    m_minPositionIterations = position;
    m_minVelocityIterations = velocity;
    m_articulation->setSolverIterationCounts(m_minPositionIterations, m_minVelocityIterations);
}
void Articulation::UpdateBody()
{
    GlobalTransform ltw = GetOwner().GetDataComponent<GlobalTransform>();
    ltw.SetScale(glm::vec3(1.0f));
    PxTransform pose = PxTransform(*(PxMat44 *)(void *)&ltw.m_value);
    switch (m_type)
    {
    case ArticulationType::Maximal:
        m_articulation = PhysicsManager::GetInstance().m_physics->createArticulation();
        m_root = m_articulation->createLink(nullptr, pose);
        break;
    case ArticulationType::Reduced:
        m_articulation = PhysicsManager::GetInstance().m_physics->createArticulationReducedCoordinate();
        m_root = m_articulation->createLink(nullptr, pose);
        break;
    }
}
void Articulation::SetDensityAndMassCenter(const float &value, const glm::vec3 &center)
{
    m_density = value;
    m_massCenter = PxVec3(center.x, center.y, center.z);
        PxRigidBodyExt::updateMassAndInertia(
            *m_root, m_density, &m_massCenter);
}
void Articulation::SetAngularVelocity(const glm::vec3 &velocity)
{
    m_angularVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    m_root->setAngularVelocity(m_angularVelocity);
}
void Articulation::SetLinearVelocity(const glm::vec3 &velocity)
{
    m_linearVelocity = PxVec3(velocity.x, velocity.y, velocity.z);
    m_root->setLinearVelocity(m_linearVelocity);
}
void Articulation::ApplyMeshBound()
{
    if (!GetOwner().IsValid() || !GetOwner().HasPrivateComponent<MeshRenderer>())
        return;
    auto &meshRenderer = GetOwner().GetPrivateComponent<MeshRenderer>();
    auto bound = meshRenderer.m_mesh->GetBound();
    glm::vec3 scale = GetOwner().GetDataComponent<GlobalTransform>().GetScale();
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
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    m_shapeUpdated = false;
}
void Articulation::SetShapeType(ShapeType type)
{
    if (m_shapeType == type)
        return;
    m_shapeType = type;
    m_shapeUpdated = false;
}
void Articulation::SetShapeParam(glm::vec3 value)
{
    if (m_shapeParam == value)
        return;
    m_shapeParam = value;
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    m_shapeUpdated = false;
}
void Articulation::SetShapeTransform(glm::mat4 value)
{
    GlobalTransform ltw;
    ltw.m_value = value;
    ltw.SetScale(glm::vec3(1.0f));
    m_shapeTransform = ltw.m_value;
    m_shapeUpdated = false;
}
void Articulation::SetMaterial(const std::shared_ptr<PhysicsMaterial> &value)
{
    if (value && m_material != value)
    {
        m_material = value;
        m_shapeUpdated = false;
    }
}
void Articulation::OnDestroy()
{
    if (m_root)
    {
        m_root->release();
        m_root = nullptr;
    }
    if (m_shape)
    {
        m_shape->release();
        m_shape = nullptr;
    }
}
