#include <DefaultResources.hpp>
#include "Editor.hpp"
#include <Particles.hpp>
#include "Engine/Rendering/Graphics.hpp"

using namespace UniEngine;

void Particles::OnCreate()
{
    m_matrices = std::make_shared<ParticleMatrices>();
    m_boundingBox = Bound();
    SetEnabled(true);
}

void Particles::RecalculateBoundingBox()
{
    if (m_matrices->PeekMatrices().empty())
    {
        m_boundingBox.m_min = glm::vec3(0.0f);
        m_boundingBox.m_max = glm::vec3(0.0f);
        return;
    }
    glm::vec3 minBound = glm::vec3(static_cast<int>(INT_MAX));
    glm::vec3 maxBound = glm::vec3(static_cast<int>(INT_MIN));
    auto meshBound = m_mesh.Get<Mesh>()->GetBound();
    for (const auto &i : m_matrices->PeekMatrices())
    {
        glm::vec3 center = i * glm::vec4(meshBound.Center(), 1.0f);
        glm::vec3 size = glm::vec4(meshBound.Size(), 0) * i / 2.0f;
        minBound = glm::vec3(
            (glm::min)(minBound.x, center.x - size.x),
            (glm::min)(minBound.y, center.y - size.y),
            (glm::min)(minBound.z, center.z - size.z));

        maxBound = glm::vec3(
            (glm::max)(maxBound.x, center.x + size.x),
            (glm::max)(maxBound.y, center.y + size.y),
            (glm::max)(maxBound.z, center.z + size.z));
    }
    m_boundingBox.m_max = maxBound;
    m_boundingBox.m_min = minBound;
}

void Particles::OnInspect()
{
    ImGui::Checkbox("Forward Rendering##Particles", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##Particles", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##Particles", &m_castShadow);
    ImGui::Text(("Instance count##Particles" + std::to_string(m_matrices->PeekMatrices().size())).c_str());
    if (ImGui::Button("Calculate bounds##Particles"))
    {
        RecalculateBoundingBox();
    }
    static bool displayBound;
    ImGui::Checkbox("Display bounds##Particles", &displayBound);
    if (displayBound)
    {
        static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
        ImGui::ColorEdit4("Color:##Particles", (float *)(void *)&displayBoundColor);
        const auto transform = GetScene()->GetDataComponent<GlobalTransform>(GetOwner()).m_value;

        GizmoSettings gizmoSettings;
        gizmoSettings.m_drawSettings.m_cullFace = false;
        gizmoSettings.m_drawSettings.m_blending = true;
        gizmoSettings.m_drawSettings.m_polygonMode = OpenGLPolygonMode::Line;
        gizmoSettings.m_drawSettings.m_lineWidth = 3.0f;

        Gizmos::DrawGizmoMesh(
            DefaultResources::Primitives::Cube,
            displayBoundColor,
            transform * glm::translate(m_boundingBox.Center()) * glm::scale(m_boundingBox.Size()),
            1, gizmoSettings);
    }

    Editor::DragAndDropButton<Material>(m_material, "Material");
    Editor::DragAndDropButton<Mesh>(m_mesh, "Mesh");
}

void Particles::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
    out << YAML::Key << "m_castShadow" << m_castShadow;
    out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

    m_mesh.Save("m_mesh", out);
    m_material.Save("m_material", out);
    out << YAML::Key << "m_matrices" << YAML::BeginMap;
    m_matrices->Serialize(out);
    out << YAML::EndMap;
}

void Particles::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["m_forwardRendering"].as<bool>();
    m_castShadow = in["m_castShadow"].as<bool>();
    m_receiveShadow = in["m_receiveShadow"].as<bool>();

    m_mesh.Load("m_mesh", in);
    m_material.Load("m_material", in);

    m_matrices->Deserialize(in["m_matrices"]);
}
void Particles::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
}
void Particles::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_mesh);
    list.push_back(m_material);
}
void Particles::OnDestroy()
{
    m_mesh.Clear();
    m_material.Clear();
    m_matrices.reset();

    m_material.Clear();
    m_forwardRendering = false;
    m_castShadow = true;
    m_receiveShadow = true;
}