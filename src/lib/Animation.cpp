#include <Animation.hpp>
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
        globalTransform = boundEntities[m_index].GetDataComponent<GlobalTransform>().m_value;
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

Animation::Animation()
{
}

void Animation::OnGui() const
{
    if (!m_rootBone)
        return;
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
    if (m_animationNameAndLength.find(name) == m_animationNameAndLength.end() || !m_rootBone)
    {
        return;
    }
    m_rootBone->Animate(name, animationTime, rootTransform, rootTransform, boundEntities, results);
}
void Animation::Serialize(YAML::Emitter &out)
{

}
void Animation::Deserialize(const YAML::Node &in)
{

}
