#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <MeshRenderer.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;
void MeshRenderer::RenderBound(glm::vec4 &color) const
{
    const auto transform = GetOwner().GetComponentData<GlobalTransform>().m_value;
    glm::vec3 size = m_mesh->m_bound.Size();
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
        transform * (glm::translate(m_mesh->m_bound.Center()) * glm::scale(size)),
        1);
}

void MeshRenderer::OnGui()
{
    ImGui::Checkbox("Forward Rendering##MeshRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##MeshRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##MeshRenderer", &m_castShadow);
    ImGui::Text("Material:");
    ImGui::SameLine();
    EditorManager::DragAndDrop(m_material);
    if (m_material)
    {
        if (ImGui::TreeNode("Material##MeshRenderer"))
        {
            m_material->OnGui();
            ImGui::TreePop();
        }
    }
    ImGui::Text("Mesh:");
    ImGui::SameLine();
    EditorManager::DragAndDrop(m_mesh);
    if (m_mesh)
    {
        if (ImGui::TreeNode("Mesh##MeshRenderer"))
        {
            static bool displayBound;
            ImGui::Checkbox("Display bounds##MeshRenderer", &displayBound);
            if (displayBound)
            {
                static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
                ImGui::ColorEdit4("Color:##MeshRenderer", (float *)(void *)&displayBoundColor);
                RenderBound(displayBoundColor);
            }
            m_mesh->OnGui();
            ImGui::TreePop();
        }
    }
}

MeshRenderer::MeshRenderer()
{
    SetEnabled(true);
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "ForwardRendering" << m_forwardRendering;
    out << YAML::Key << "CastShadow" << m_castShadow;
    out << YAML::Key << "ReceiveShadow" << m_receiveShadow;
}

void MeshRenderer::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["ForwardRendering"].as<bool>();
    m_castShadow = in["CastShadow"].as<bool>();
    m_receiveShadow = in["ReceiveShadow"].as<bool>();
}