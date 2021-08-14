#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <MeshRenderer.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;
void MeshRenderer::RenderBound(glm::vec4 &color)
{
    const auto transform = GetOwner().GetDataComponent<GlobalTransform>().m_value;
    glm::vec3 size = m_mesh.Get<Mesh>()->m_bound.Size();
    if (size.x < 0.01f)
        size.x = 0.01f;
    if (size.z < 0.01f)
        size.z = 0.01f;
    if (size.y < 0.01f)
        size.y = 0.01f;
    RenderManager::DrawGizmoMesh(
        DefaultResources::Primitives::Cube,
        color,
        transform * (glm::translate(m_mesh.Get<Mesh>()->m_bound.Center()) * glm::scale(size)),
        1);
}

void MeshRenderer::OnGui()
{
    ImGui::Checkbox("Forward Rendering##MeshRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##MeshRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##MeshRenderer", &m_castShadow);
    EditorManager::DragAndDrop<Material>(m_material, "Material");
    if (m_material.Get<Material>())
    {
        if (ImGui::TreeNode("Material##MeshRenderer"))
        {
            m_material.Get<Material>()->OnGui();
            ImGui::TreePop();
        }
    }
    EditorManager::DragAndDrop<Mesh>(m_mesh, "Mesh");
    if (m_mesh.Get<Mesh>())
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
            m_mesh.Get<Mesh>()->OnGui();
            ImGui::TreePop();
        }
    }
}

void MeshRenderer::OnCreate()
{
    SetEnabled(true);
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
void MeshRenderer::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<MeshRenderer>(target);
}
