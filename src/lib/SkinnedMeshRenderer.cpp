#include <Application.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <RenderManager.hpp>
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;
void SkinnedMeshRenderer::RenderBound(glm::vec4 &color)
{
    const auto transform = GetOwner().GetDataComponent<GlobalTransform>().m_value;
    glm::vec3 size = m_skinnedMesh.Get<SkinnedMesh>()->m_bound.Size();
    if (size.x < 0.01f)
        size.x = 0.01f;
    if (size.z < 0.01f)
        size.z = 0.01f;
    if (size.y < 0.01f)
        size.y = 0.01f;
    RenderManager::DrawGizmoMesh(
        DefaultResources::Primitives::Cube,
        color,
        transform * (glm::translate(m_skinnedMesh.Get<SkinnedMesh>()->m_bound.Center()) * glm::scale(size)),
        1);
}

void SkinnedMeshRenderer::GetBoneMatrices()
{
    auto animator = m_animator.Get<Animator>();
    auto skinnedMesh = m_skinnedMesh.Get<SkinnedMesh>();
    if (!animator || !skinnedMesh)
        return;
    m_finalResults->m_value.resize(skinnedMesh->m_boneAnimatorIndices.size());
    for (int i = 0; i < skinnedMesh->m_boneAnimatorIndices.size(); i++)
    {
        m_finalResults->m_value[i] = animator->m_transformChain[skinnedMesh->m_boneAnimatorIndices[i]];
    }
}

void SkinnedMeshRenderer::AttachAnimator(const std::shared_ptr<Animator> &animator)
{
    if (animator->m_animation.Get<Animation>() == m_skinnedMesh.Get<SkinnedMesh>()->m_animation.Get<Animation>())
    {
        m_animator.Set(animator);
    }
    else
    {
        UNIENGINE_ERROR("Animator doesn't share same animation!");
    }
}

void SkinnedMeshRenderer::OnGui()
{
    auto animator = m_animator.Get<Animator>();
    EditorManager::DragAndDrop<Animator>(m_animator, "Animator");
    if(animator != m_animator.Get<Animator>()){
        AttachAnimator(m_animator.Get<Animator>());
    }

    ImGui::Checkbox("Forward Rendering##SkinnedMeshRenderer", &m_forwardRendering);
    if (!m_forwardRendering)
        ImGui::Checkbox("Receive shadow##SkinnedMeshRenderer", &m_receiveShadow);
    ImGui::Checkbox("Cast shadow##SkinnedMeshRenderer", &m_castShadow);
    EditorManager::DragAndDrop<Material>(m_material, "Material");
    if (m_material.Get<Material>())
    {
        if (ImGui::TreeNode("Material##SkinnedMeshRenderer"))
        {
            m_material.Get<Material>()->OnGui();
            ImGui::TreePop();
        }
    }
    EditorManager::DragAndDrop<SkinnedMesh>(m_skinnedMesh, "Skinned Mesh");
    if (m_skinnedMesh.Get<SkinnedMesh>())
    {
        if (ImGui::TreeNode("Skinned Mesh:##SkinnedMeshRenderer"))
        {
            static bool displayBound;
            ImGui::Checkbox("Display bounds##SkinnedMeshRenderer", &displayBound);
            if (displayBound)
            {
                static auto displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.2f);
                ImGui::ColorEdit4("Color:##SkinnedMeshRenderer", (float *)(void *)&displayBoundColor);
                RenderBound(displayBoundColor);
            }
            m_skinnedMesh.Get<SkinnedMesh>()->OnGui();
            ImGui::TreePop();
        }
    }
}

void SkinnedMeshRenderer::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_forwardRendering" << m_forwardRendering;
    out << YAML::Key << "m_castShadow" << m_castShadow;
    out << YAML::Key << "m_receiveShadow" << m_receiveShadow;

    m_animator.Save("m_animator", out);
    m_skinnedMesh.Save("m_skinnedMesh", out);
    m_material.Save("m_material", out);
}

void SkinnedMeshRenderer::Deserialize(const YAML::Node &in)
{
    m_forwardRendering = in["m_forwardRendering"].as<bool>();
    m_castShadow = in["m_castShadow"].as<bool>();
    m_receiveShadow = in["m_receiveShadow"].as<bool>();

    m_animator.Load("m_animator", in);
    m_skinnedMesh.Load("m_skinnedMesh", in);
    m_material.Load("m_material", in);
}
void SkinnedMeshRenderer::OnCreate()
{
    m_finalResults = std::make_shared<BoneMatrices>();
    SetEnabled(true);
}
void SkinnedMeshRenderer::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<SkinnedMeshRenderer>(target);
}
void SkinnedMeshRenderer::Relink(const std::unordered_map<Handle, Handle> &map)
{
    m_animator.Relink(map);
}
void SkinnedMeshRenderer::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_skinnedMesh);
    list.push_back(m_material);
}
void BoneMatrices::UploadBones(const std::shared_ptr<SkinnedMesh>& skinnedMesh) const
{
    skinnedMesh->UploadBones(m_value);
}
