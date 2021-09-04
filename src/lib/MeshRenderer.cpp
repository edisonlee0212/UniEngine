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

void MeshRenderer::OnInspect()
{
    ImGui::Checkbox("Forward Rendering##MeshRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##MeshRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##MeshRenderer", &m_castShadow);
    EditorManager::DragAndDropButton<Material>(m_material, "Material");
    EditorManager::DragAndDropButton<Mesh>(m_mesh, "Mesh");
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
    out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
    out << YAML::Key << "m_castShadow" << m_castShadow;
    out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

    m_mesh.Save("m_mesh", out);
    m_material.Save("m_material", out);
}

void MeshRenderer::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["m_forwardRendering"].as<bool>();
    m_castShadow = in["m_castShadow"].as<bool>();
    m_receiveShadow = in["m_receiveShadow"].as<bool>();

    m_mesh.Load("m_mesh", in);
    m_material.Load("m_material", in);
}
void MeshRenderer::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<MeshRenderer>(target);
}
void MeshRenderer::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_mesh);
    list.push_back(m_material);
}
