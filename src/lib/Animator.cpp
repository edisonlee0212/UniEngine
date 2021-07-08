#include "Application.hpp"
#include "RenderManager.hpp"
#include <Animator.hpp>
#include <EditorManager.hpp>
using namespace UniEngine;

void Bone::Animate(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &parentTransform,
    const glm::mat4 &rootTransform,
    std::vector<Entity> &boundEntities,
    std::vector<glm::mat4> &results)
{
    glm::mat4 globalTransform = parentTransform;
    if (boundEntities[m_index].IsValid())
    {
        globalTransform =
            boundEntities[m_index].GetComponentData<GlobalTransform>().m_value;
    }
    else
    {
        const auto search = m_animations.find(name);
        if (search != m_animations.end())
        {
            const auto translation = m_animations[name].InterpolatePosition(animationTime);
            const auto rotation = m_animations[name].InterpolateRotation(animationTime);
            const auto scale = m_animations[name].InterpolateScaling(animationTime);
            globalTransform *= translation * rotation * scale;
        }
    }
    results[m_index] = globalTransform;
    for (auto &i : m_children)
    {
        i->Animate(name, animationTime, globalTransform, rootTransform, boundEntities, results);
    }
}

void Bone::OnGui()
{
    if (ImGui::TreeNode((m_name + "##" + std::to_string(m_index)).c_str()))
    {
        ImGui::Text("Controller: ");
        ImGui::SameLine();
        for (auto &i : m_children)
        {
            i->OnGui();
        }
        ImGui::TreePop();
    }
}

int BoneKeyFrames::GetPositionIndex(const float &animationTime)
{
    const int size = m_positions.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_positions[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

int BoneKeyFrames::GetRotationIndex(const float &animationTime)
{
    const int size = m_rotations.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_rotations[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

int BoneKeyFrames::GetScaleIndex(const float &animationTime)
{
    const int size = m_scales.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_scales[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

float BoneKeyFrames::GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime)
{
    const float midWayLength = animationTime - lastTimeStamp;
    const float framesDiff = nextTimeStamp - lastTimeStamp;
    if (framesDiff == 0.0f)
        return 0.0f;
    return glm::clamp(midWayLength / framesDiff, 0.0f, 1.0f);
}

glm::mat4 BoneKeyFrames::InterpolatePosition(const float &animationTime)
{
    if (1 == m_positions.size())
        return glm::translate(glm::mat4(1.0f), m_positions[0].m_value);

    const int p0Index = GetPositionIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_positions[p0Index].m_timeStamp, m_positions[p1Index].m_timeStamp, animationTime);
    const glm::vec3 finalPosition = glm::mix(m_positions[p0Index].m_value, m_positions[p1Index].m_value, scaleFactor);
    return glm::translate(finalPosition);
}

glm::mat4 BoneKeyFrames::InterpolateRotation(const float &animationTime)
{
    if (1 == m_rotations.size())
    {
        const auto rotation = glm::normalize(m_rotations[0].m_value);
        return glm::mat4_cast(rotation);
    }

    const int p0Index = GetRotationIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_rotations[p0Index].m_timeStamp, m_rotations[p1Index].m_timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].m_value, m_rotations[p1Index].m_value, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::mat4_cast(finalRotation);
}

glm::mat4 BoneKeyFrames::InterpolateScaling(const float &animationTime)
{
    if (1 == m_scales.size())
        return glm::scale(m_scales[0].m_value);

    const int p0Index = GetScaleIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_scales[p0Index].m_timeStamp, m_scales[p1Index].m_timeStamp, animationTime);
    const glm::vec3 finalScale = glm::mix(m_scales[p0Index].m_value, m_scales[p1Index].m_value, scaleFactor);
    return glm::scale(finalScale);
}

std::shared_ptr<Bone> &Animation::UnsafeGetRootBone()
{
    return m_rootBone;
}

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
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling +
                                                  // for keyboard navigation support)
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
        return;
    if (!m_animation)
    {
        for(int i = 0; i < m_transformChain.size(); i++)
        {
            m_transformChain[i] = m_boundEntities[i].GetComponentData<GlobalTransform>().m_value;
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
            owner.GetComponentData<GlobalTransform>().m_value,
            m_boundEntities,
            m_transformChain);
    }
    ApplyOffsetMatrices();
}

Animation::Animation()
{
}

void Animation::OnGui() const
{
    ImGui::Text(("Bone size: " + std::to_string(m_boneSize)).c_str());
    m_rootBone->OnGui();
}

void Animation::Animate(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &rootTransform,
    std::vector<Entity> &boundEntities,
    std::vector<glm::mat4> &results)
{
    if (m_animationNameAndLength.find(name) == m_animationNameAndLength.end())
    {
        return;
    }
    m_rootBone->Animate(name, animationTime, rootTransform, rootTransform, boundEntities, results);
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
    m_boneSize = m_boundEntities.size();
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
    const auto selfScale = GetOwner().GetComponentData<GlobalTransform>().GetScale();

    std::vector<glm::mat4> debugRenderingMatrices = m_transformChain;
    for (int index = 0; index < m_transformChain.size(); index++)
    {
        debugRenderingMatrices[index] = m_transformChain[index] * glm::inverse(m_bones[index]->m_offsetMatrix.m_value) *
                                        glm::inverse(glm::scale(selfScale));
    }
    RenderManager::DrawGizmoMeshInstanced(
        DefaultResources::Primitives::Sphere.get(),
        EditorManager::GetSceneCamera().get(),
        color,
        debugRenderingMatrices.data(),
        debugRenderingMatrices.size(),
        Transform().m_value,
        size);
}

void Animator::ResetTransform(const int &index)
{
    GlobalTransform globalTransform;
    auto &entity = m_boundEntities[index];
    globalTransform.m_value =
        m_transformChain[index] * glm::inverse(m_bones[index]->m_offsetMatrix.m_value);
    auto parent = EntityManager::GetParent(m_boundEntities[index]);
    Transform localTransform;
    if (parent.IsValid())
    {
        const auto parentGlobalTransform = parent.GetComponentData<GlobalTransform>();
        localTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
    }
    else
    {
        localTransform.m_value = globalTransform.m_value;
    }
    entity.SetComponentData(localTransform);
}
