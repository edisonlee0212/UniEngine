#include <Animator.hpp>
using namespace UniEngine;

void Bone::Update(const std::string& name, const float &animationTime)
{
    auto search = m_animations.find(name);
    if (search != m_animations.end())
    {
        const auto translation = m_animations[name].InterpolatePosition(animationTime);
        const auto rotation = m_animations[name].InterpolateRotation(animationTime);
        const auto scale = m_animations[name].InterpolateScaling(animationTime);
        m_localToParent.m_value = translation * rotation * scale;
    }else
    {
        m_localToParent = Transform();
    }
    for (auto &i : m_children)
    {
        i->Update(name, animationTime);
    }
}

int BoneAnimation::GetPositionIndex(const float &animationTime)
{
    auto size = m_positions.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_positions[index + 1].m_timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int BoneAnimation::GetRotationIndex(const float &animationTime)
{
    auto size = m_rotations.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_rotations[index + 1].m_timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

int BoneAnimation::GetScaleIndex(const float &animationTime)
{
    auto size = m_scales.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_scales[index + 1].m_timeStamp)
            return index;
    }
    assert(0);
    return 0;
}

float BoneAnimation::GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 BoneAnimation::InterpolatePosition(const float &animationTime)
{
    if (1 == m_positions.size())
        return glm::translate(glm::mat4(1.0f), m_positions[0].m_value);

    const int p0Index = GetPositionIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_positions[p0Index].m_timeStamp, m_positions[p1Index].m_timeStamp, animationTime);
    const glm::vec3 finalPosition =
        glm::mix(m_positions[p0Index].m_value, m_positions[p1Index].m_value, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 BoneAnimation::InterpolateRotation(const float &animationTime)
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

glm::mat4 BoneAnimation::InterpolateScaling(const float &animationTime)
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

std::shared_ptr<Bone> & Animator::UnsafeGetRootBone()
{
    return m_rootBone;
}

void Animator::OnGui()
{
}

Animator::Animator()
{
}

void Animator::Update(const std::string &name, const float &animationTime)
{
    m_rootBone->Update(name, animationTime);
}

Animator::~Animator()
{
}
