#include "Application.hpp"
#include "RenderManager.hpp"
#include <EditorManager.hpp>
#include <Animator.hpp>
using namespace UniEngine;

void Bone::Animate(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &parentTransform,
    std::vector<glm::mat4> &results)
{
    glm::mat4 globalTransform = parentTransform;
    const auto search = m_animations.find(name);
    if (search != m_animations.end())
    {
        const auto translation = m_animations[name].InterpolatePosition(animationTime);
        const auto rotation = m_animations[name].InterpolateRotation(animationTime);
        const auto scale = m_animations[name].InterpolateScaling(animationTime);
        globalTransform *= translation * rotation * scale;
    }
    results[m_index] = globalTransform * m_offsetMatrix.m_value;
    for (auto &i : m_children)
    {
        i->Animate(name, animationTime, globalTransform, results);
    }
}

void Bone::ApplyLocalTransform(const glm::mat4 &parentTransform, std::vector<glm::mat4> &results)
{
    glm::mat4 globalTransform = parentTransform * m_localTransform.m_value;
    results[m_index] = globalTransform * m_offsetMatrix.m_value;
    for (auto &i : m_children)
    {
        i->ApplyLocalTransform(globalTransform, results);
    }
}

void Bone::OnGui() const
{
    if (ImGui::TreeNode((m_name + "##" + std::to_string(m_index)).c_str()))
    {
        for (auto& i : m_children)
        {
            i->OnGui();
        }
        ImGui::TreePop();
    }
}

void Bone::DebugRenderAnimated(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &parentTransform,
    const float &size,
    const glm::vec4 &color,
    const std::shared_ptr<Mesh> &mesh)
{
    glm::mat4 globalTransform = parentTransform;
    const auto search = m_animations.find(name);
    if (search != m_animations.end())
    {
        const auto translation = m_animations[name].InterpolatePosition(animationTime);
        const auto rotation = m_animations[name].InterpolateRotation(animationTime);
        const auto scale = m_animations[name].InterpolateScaling(animationTime);
        globalTransform *= translation * rotation * scale;
    }
    RenderManager::DrawGizmoMesh(mesh.get(), EditorManager::GetSceneCamera().get(), color, globalTransform, size);
    for (auto &i : m_children)
    {
        i->DebugRenderAnimated(name, animationTime, globalTransform, size, color, mesh);
    }
}

void Bone::DebugRenderLocalTransform(
    const glm::mat4 &parentTransform,
    const float &size,
    const glm::vec4 &color,
    const std::shared_ptr<Mesh> &mesh)
{
    const glm::mat4 globalTransform = parentTransform * m_localTransform.m_value;
    RenderManager::DrawGizmoMesh(mesh.get(), EditorManager::GetSceneCamera().get(), color, globalTransform, size);
    for (auto &i : m_children)
    {
        i->DebugRenderLocalTransform(globalTransform, size, color, mesh);
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
    m_transformChain.resize(m_animation->m_boneSize);
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
        ImGui::Checkbox("Display bones", &m_debugRenderBones);
        if (m_debugRenderBones)
        {
            ImGui::DragFloat("Size", &m_debugRenderBonesSize, 0.01f, 0.01f, 3.0f);
            ImGui::ColorEdit4("Color", &m_debugRenderBonesColor.x);
            m_animation->m_rootBone->DebugRenderAnimated(
                m_currentActivatedAnimation,
                m_currentAnimationTime,
                GetOwner().GetComponentData<GlobalTransform>().m_value,
                m_debugRenderBonesSize,
                m_debugRenderBonesColor,
                DefaultResources::Primitives::Sphere);
        }
        m_animation->OnGui();
        
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
                bool is_selected =
                    m_currentActivatedAnimation ==
                    i.first; // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(i.first.c_str(), is_selected))
                {
                    m_currentActivatedAnimation = i.first;
                    m_currentAnimationTime = 0.0f;
                }
                if (is_selected)
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
        m_currentAnimationTime = glm::mod(m_currentAnimationTime, m_animation->m_animationNameAndLength[m_currentActivatedAnimation]);
}
void Animator::Animate()
{
    if (!m_animation) return;
    if (m_animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
        m_animation->m_animationNameAndLength.end())
    {
        m_currentActivatedAnimation = m_animation->m_animationNameAndLength.begin()->first;
        m_currentAnimationTime = 0.0f;
    }
    m_animation->Animate(m_currentActivatedAnimation, m_currentAnimationTime, m_transformChain);
    AnimateHelper(GetOwner());
}

Animation::Animation()
{
}

void Animation::OnGui() const
{
    ImGui::Text(("Bone size: " + std::to_string(m_boneSize)).c_str());
    m_rootBone->OnGui();
}

void Animation::Animate(const std::string &name, const float &animationTime, std::vector<glm::mat4> &results)
{
    if (m_animationNameAndLength.find(name) == m_animationNameAndLength.end())
    {
        return;
    }
    m_rootBone->Animate(name, animationTime, Transform().m_value, results);
}

void Animator::AnimateHelper(const Entity &walker)
{
    if (walker.HasPrivateComponent<SkinnedMeshRenderer>())
    {
        walker.GetPrivateComponent<SkinnedMeshRenderer>()->CalculateBones(this);        
    }
    EntityManager::ForEachChild(walker, [&](Entity child) { AnimateHelper(child);});
}
