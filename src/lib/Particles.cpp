#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <Particles.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void Particles::OnCreate()
{
    m_matrices = std::make_shared<ParticleMatrices>();
    m_boundingBox = Bound();
    SetEnabled(true);
}

void Particles::RecalculateBoundingBox()
{
    if (m_matrices->m_value.empty())
    {
        m_boundingBox.m_min = glm::vec3(0.0f);
        m_boundingBox.m_max = glm::vec3(0.0f);
        return;
    }
    glm::vec3 minBound = glm::vec3(static_cast<int>(INT_MAX));
    glm::vec3 maxBound = glm::vec3(static_cast<int>(INT_MIN));
    auto meshBound = m_mesh->GetBound();
    for (auto &i : m_matrices->m_value)
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

void Particles::OnGui()
{
    ImGui::Checkbox("Forward Rendering##Particles", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##Particles", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##Particles", &m_castShadow);
    ImGui::Text(("Instance count##Particles" + std::to_string(m_matrices->m_value.size())).c_str());
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
        const auto transform = GetOwner().GetDataComponent<GlobalTransform>().m_value;
        RenderManager::DrawGizmoMesh(
            DefaultResources::Primitives::Cube,
            displayBoundColor,
            transform * glm::translate(m_boundingBox.Center()) * glm::scale(m_boundingBox.Size()),
            1);
    }

    ImGui::Text("Material:##Particles");
    ImGui::SameLine();
    EditorManager::DragAndDrop(m_material);
    if (m_material)
    {
        if (ImGui::TreeNode("Material##Particles"))
        {
            m_material->OnGui();
            ImGui::TreePop();
        }
    }
    ImGui::Text("Mesh:##Particles");
    ImGui::SameLine();
    EditorManager::DragAndDrop(m_mesh);
    if (m_mesh)
    {
        if (ImGui::TreeNode("Mesh##Particles"))
        {
            m_mesh->OnGui();
            ImGui::TreePop();
        }
    }
}

void Particles::Serialize(YAML::Emitter &out)
{
}

void Particles::Deserialize(const YAML::Node &in)
{
}
