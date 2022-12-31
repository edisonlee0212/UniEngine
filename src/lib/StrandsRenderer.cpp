#include "DefaultResources.hpp"
#include "Editor.hpp"
#include "StrandsRenderer.hpp"
#include "Graphics.hpp"
#include "Strands.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;

PrivateComponentRegistration<StrandsRenderer> StrandsRendererRegistry("StrandsRenderer");

void StrandsRenderer::RenderBound(glm::vec4 &color)
{
    const auto transform = GetScene()->GetDataComponent<GlobalTransform>(GetOwner()).m_value;
    glm::vec3 size = m_strands.Get<Strands>()->m_bound.Size();
    if (size.x < 0.01f)
        size.x = 0.01f;
    if (size.z < 0.01f)
        size.z = 0.01f;
    if (size.y < 0.01f)
        size.y = 0.01f;
    GizmoSettings gizmoSettings;
    gizmoSettings.m_drawSettings.m_cullFace = false;
    gizmoSettings.m_drawSettings.m_blending = true;
    gizmoSettings.m_drawSettings.m_polygonMode = OpenGLPolygonMode::Line;
    gizmoSettings.m_drawSettings.m_lineWidth = 3.0f;
    Gizmos::DrawGizmoMesh(
            DefaultResources::Primitives::Cube,
            color,
            transform * (glm::translate(m_strands.Get<Strands>()->m_bound.Center()) * glm::scale(size)),
            1, gizmoSettings);
}

void StrandsRenderer::OnInspect()
{
    ImGui::Checkbox("Forward Rendering##StrandsRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##StrandsRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##StrandsRenderer:", &m_castShadow);
    Editor::DragAndDropButton<Material>(m_material, "Material");
    Editor::DragAndDropButton<Strands>(m_strands, "Strands");
    if (m_strands.Get<Strands>())
    {
        if (ImGui::TreeNode("Strands##StrandsRenderer"))
        {
            static bool displayBound = true;
            ImGui::Checkbox("Display bounds##StrandsRenderer", &displayBound);
            if (displayBound)
            {
                static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
                ImGui::ColorEdit4("Color:##StrandsRenderer", (float *)(void *)&displayBoundColor);
                RenderBound(displayBoundColor);
            }
            ImGui::TreePop();
        }
    }
}

void StrandsRenderer::OnCreate()
{
    SetEnabled(true);
}


void StrandsRenderer::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
    out << YAML::Key << "m_castShadow" << m_castShadow;
    out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

    m_strands.Save("m_strands", out);
    m_material.Save("m_material", out);
}

void StrandsRenderer::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["m_forwardRendering"].as<bool>();
    m_castShadow = in["m_castShadow"].as<bool>();
    m_receiveShadow = in["m_receiveShadow"].as<bool>();

    m_strands.Load("m_strands", in);
    m_material.Load("m_material", in);
}
void StrandsRenderer::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
}
void StrandsRenderer::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_strands);
    list.push_back(m_material);
}
void StrandsRenderer::OnDestroy()
{
    m_strands.Clear();
    m_material.Clear();

    m_material.Clear();
    m_forwardRendering = false;
    m_castShadow = true;
    m_receiveShadow = true;
}

