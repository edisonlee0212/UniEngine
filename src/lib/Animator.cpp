#include "Application.hpp"
#include "RenderManager.hpp"
#include <Animator.hpp>
#include <EditorManager.hpp>
using namespace UniEngine;
void Animator::Setup(const std::shared_ptr<Animation> &targetAnimation)
{
    m_animation = targetAnimation;
    m_boneSize = m_animation->m_boneSize;
    m_transformChain.resize(m_boneSize);
    m_boundEntities.resize(m_boneSize);
    m_names.resize(m_boneSize);
    m_bones.resize(m_boneSize);
    BoneSetter(m_animation->m_rootBone);
    m_offsetMatrices.resize(m_boneSize);
    for (auto &i : m_bones)
        m_offsetMatrices[i->m_index] = i->m_offsetMatrix.m_value;
}

void Animator::OnGui()
{
    ImGui::Text("Animation:");
    ImGui::SameLine();
    Animation *previous = m_animation.get();
    EditorManager::DragAndDrop(m_animation);
    if (previous != m_animation.get() && m_animation)
    {
        Setup(m_animation);
    }
    if (m_animation)
    {
        static bool debugRenderBones = true;
        static float debugRenderBonesSize = 0.5f;
        static glm::vec4 debugRenderBonesColor = glm::vec4(1, 1, 0, 0.5);
        ImGui::Checkbox("Display bones", &debugRenderBones);
        if (debugRenderBones)
        {
            ImGui::DragFloat("Size", &debugRenderBonesSize, 0.01f, 0.01f, 3.0f);
            ImGui::ColorEdit4("Color", &debugRenderBonesColor.x);
            DebugBoneRender(debugRenderBonesColor, debugRenderBonesSize);
        }
        m_animation->OnGui();
        if (ImGui::TreeNode("RagDoll"))
        {
            for (int i = 0; i < m_boundEntities.size(); i++)
            {
                ImGui::Text(("Bone: " + m_names[i]).c_str());
                ImGui::SameLine();
                if (EditorManager::DragAndDrop(m_boundEntities[i]))
                {
                    if (m_boundEntities[i].IsValid())
                    {
                        ResetTransform(i);
                    }
                }
            }
            ImGui::TreePop();
        }
        if (m_boneSize != 0)
        {
            if (m_animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
                m_animation->m_animationNameAndLength.end())
            {
                m_currentActivatedAnimation = m_animation->m_animationNameAndLength.begin()->first;
                m_currentAnimationTime = 0.0f;
            }
            if (ImGui::BeginCombo(
                    "Animations##Animator",
                    m_currentActivatedAnimation
                        .c_str())) // The second parameter is the label previewed before opening the combo.
            {
                for (auto &i : m_animation->m_animationNameAndLength)
                {
                    const bool selected =
                        m_currentActivatedAnimation ==
                        i.first; // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(i.first.c_str(), selected))
                    {
                        m_currentActivatedAnimation = i.first;
                        m_currentAnimationTime = 0.0f;
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling
                                                      // + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("AutoPlay", &m_autoPlay);
            ImGui::SliderFloat(
                "Animation time",
                &m_currentAnimationTime,
                0.0f,
                m_animation->m_animationNameAndLength[m_currentActivatedAnimation]);
        }
    }
}
void Animator::AutoPlay()
{
    if (!m_animation)
        return;
    m_currentAnimationTime += Application::Time().DeltaTime() * 1000.0f;
    if (m_currentAnimationTime > m_animation->m_animationNameAndLength[m_currentActivatedAnimation])
        m_currentAnimationTime =
            glm::mod(m_currentAnimationTime, m_animation->m_animationNameAndLength[m_currentActivatedAnimation]);
}
void Animator::Animate()
{
    if (m_boneSize == 0)
    {
        for (int i = 0; i < m_transformChain.size(); i++)
        {
            m_transformChain[i] = m_boundEntities[i].GetDataComponent<GlobalTransform>().m_value;
        }
    }
    else
    {
        auto owner = GetOwner();
        if (m_animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
            m_animation->m_animationNameAndLength.end())
        {
            m_currentActivatedAnimation = m_animation->m_animationNameAndLength.begin()->first;
            m_currentAnimationTime = 0.0f;
        }
        m_animation->Animate(
            m_currentActivatedAnimation,
            m_currentAnimationTime,
            owner.GetDataComponent<GlobalTransform>().m_value,
            m_boundEntities,
            m_transformChain);
    }
    ApplyOffsetMatrices();
}


void Animator::BoneSetter(const std::shared_ptr<Bone> &boneWalker)
{
    m_names[boneWalker->m_index] = boneWalker->m_name;
    m_bones[boneWalker->m_index] = boneWalker;
    for (auto &i : boneWalker->m_children)
    {
        BoneSetter(i);
    }
}

void Animator::Setup(
    std::vector<Entity> &boundEntities, std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices)
{
    m_bones.clear();
    assert(boundEntities.size() == name.size() && boundEntities.size() == offsetMatrices.size());
    m_transformChain.resize(boundEntities.size());
    m_boundEntities = boundEntities;
    m_names = name;
    m_offsetMatrices = offsetMatrices;
}

void Animator::ApplyOffsetMatrices()
{
    for (int i = 0; i < m_transformChain.size(); i++)
    {
        m_transformChain[i] *= m_offsetMatrices[i];
    }
}

void Animator::DebugBoneRender(const glm::vec4 &color, const float &size) const
{
    const auto selfScale = GetOwner().GetDataComponent<GlobalTransform>().GetScale();

    std::vector<glm::mat4> debugRenderingMatrices = m_transformChain;
    for (int index = 0; index < m_transformChain.size(); index++)
    {
        debugRenderingMatrices[index] =
            m_transformChain[index] * glm::inverse(m_offsetMatrices[index]) * glm::inverse(glm::scale(selfScale));
    }
    RenderManager::DrawGizmoMeshInstanced(
        DefaultResources::Primitives::Sphere.get(),
        color,
        debugRenderingMatrices,
        Transform().m_value,
        size);
}

void Animator::ResetTransform(const int &index)
{
    GlobalTransform globalTransform;
    auto &entity = m_boundEntities[index];
    globalTransform.m_value = m_transformChain[index] * glm::inverse(m_bones[index]->m_offsetMatrix.m_value);
    auto parent = m_boundEntities[index].GetParent();
    Transform localTransform;
    if (parent.IsValid())
    {
        const auto parentGlobalTransform = parent.GetDataComponent<GlobalTransform>();
        localTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
    }
    else
    {
        localTransform.m_value = globalTransform.m_value;
    }
    entity.SetDataComponent(localTransform);
}
