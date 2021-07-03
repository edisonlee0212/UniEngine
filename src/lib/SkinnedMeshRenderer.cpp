#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;
void SkinnedMeshRenderer::RenderBound(glm::vec4 &color) const
{
    const auto transform = GetOwner().GetComponentData<GlobalTransform>().m_value;
    glm::vec3 size = m_skinnedMesh->m_bound.Size();
    if (size.x < 0.01f)
        size.x = 0.01f;
    if (size.z < 0.01f)
        size.z = 0.01f;
    if (size.y < 0.01f)
        size.y = 0.01f;
    RenderManager::DrawGizmoMesh(
        DefaultResources::Primitives::Cube.get(),
        EditorManager::GetSceneCamera().get(),
        color,
        transform * (glm::translate(m_skinnedMesh->m_bound.Center()) * glm::scale(size)),
        1);
}

void SkinnedMeshRenderer::OnGui()
{
    ImGui::Checkbox("Forward Rendering##SkinnedMeshRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##SkinnedMeshRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##SkinnedMeshRenderer", &m_castShadow);
    ImGui::Text("Material:");
    ImGui::SameLine();
    EditorManager::DragAndDrop(m_material);
    if (m_material)
    {
        if (ImGui::TreeNode("Material##SkinnedMeshRenderer"))
        {
            m_material->OnGui();
            ImGui::TreePop();
        }
    }
    if (m_skinnedMesh)
    {
        if (ImGui::TreeNode("Skinned Mesh:##SkinnedMeshRenderer"))
        {
            ImGui::Checkbox("Display bounds##SkinnedMeshRenderer", &m_displayBound);
            if (m_displayBound)
            {
                ImGui::ColorEdit4("Color:##SkinnedMeshRenderer", (float *)(void *)&m_displayBoundColor);
                RenderBound(m_displayBoundColor);
            }
            m_skinnedMesh->OnGui();
            ImGui::TreePop();
        }
    }
}

SkinnedMeshRenderer::SkinnedMeshRenderer()
{
    SetEnabled(true);
}


void SkinnedMeshRenderer::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "ForwardRendering" << m_forwardRendering;
    out << YAML::Key << "CastShadow" << m_castShadow;
    out << YAML::Key << "ReceiveShadow" << m_receiveShadow;
}

void SkinnedMeshRenderer::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["ForwardRendering"].as<bool>();
    m_castShadow = in["CastShadow"].as<bool>();
    m_receiveShadow = in["ReceiveShadow"].as<bool>();
}