#include "RenderManager.hpp"

#include <Animator.hpp>
using namespace UniEngine;

void Bone::Update(const std::string& name, const float &animationTime)
{
	const auto search = m_animations.find(name);
	if (search != m_animations.end())
	{
		const auto translation = m_animations[name].InterpolatePosition(animationTime);
		const auto rotation = m_animations[name].InterpolateRotation(animationTime);
		const auto scale = m_animations[name].InterpolateScaling(animationTime);
        m_boneTransform = translation * rotation * scale;
	}else
	{
        m_boneTransform = Transform().m_value;
	}
	for (auto &i : m_children)
	{
		i->Update(name, animationTime);
	}
}

void Bone::CalculateBoneTransform(const glm::mat4 &parentTransform)
{
    const glm::mat4 globalTransform = parentTransform * m_boneTransform;
    m_currentFinalMatrix = globalTransform * m_offsetMatrix.m_value;
	for (auto &i : m_children) i->CalculateBoneTransform(globalTransform);
}

int BoneAnimation::GetPositionIndex(const float &animationTime)
{
	const int size = m_positions.size();
	for (int index = 0; index < size - 1; ++index)
	{
		if (animationTime < m_positions[index + 1].m_timeStamp)
			return index;
	}
	return size - 2;
}

int BoneAnimation::GetRotationIndex(const float &animationTime)
{
	const int size = m_rotations.size();
	for (int index = 0; index < size - 1; ++index)
	{
		if (animationTime < m_rotations[index + 1].m_timeStamp)
			return index;
	}
	return size - 2;
}

int BoneAnimation::GetScaleIndex(const float &animationTime)
{
	const int size = m_scales.size();
	for (int index = 0; index < size - 1; ++index)
	{
		if (animationTime < m_scales[index + 1].m_timeStamp)
			return index;
	}
	return size - 2;
}

float BoneAnimation::GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime)
{
	const float midWayLength = animationTime - lastTimeStamp;
	const float framesDiff = nextTimeStamp - lastTimeStamp;
	if (framesDiff == 0.0f)
		return 0.0f;
	return glm::clamp(midWayLength / framesDiff, 0.0f, 1.0f);
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
	return glm::translate(finalPosition);
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

void Bone::RenderBones(const float &size, const glm::mat4 &parentTransform)
{
    const glm::mat4 transform = parentTransform * m_boneTransform;

    if(!m_name.empty()) RenderManager::DrawGizmoMesh(
        DefaultResources::Primitives::Cube.get(),
        RenderManager::GetMainCamera(),
        glm::vec4(1, 0, 0, 1),
        transform,
        size);
    for (auto &i : m_children)
        i->RenderBones(size, transform);
}

void Animator::OnGui()
{
    if (m_animationNameAndLength.find(m_currentActivatedAnimation) == m_animationNameAndLength.end())
    {
        m_currentActivatedAnimation = m_animationNameAndLength.begin()->first;
        m_currentAnimationTime = 0.0f;
    }
    ImGui::Text(("Current activated animation: " + m_currentActivatedAnimation).c_str());
    if (ImGui::SliderFloat("Time", &m_currentAnimationTime, 0.0f, m_animationNameAndLength[m_currentActivatedAnimation]))
    {
        Animate();
    }
    ImGui::Checkbox("Render bones", &m_renderBones);
	if(m_renderBones)
	{
        ImGui::DragFloat("Size: ", &m_renderSize, 0.01f, 0.01f, 1.0f);
        m_rootBone->RenderBones(m_renderSize, GetOwner().GetComponentData<GlobalTransform>().m_value);
	}
}

Animator::Animator()
{
}

void Animator::Animate()
{
	if (m_animationNameAndLength.find(m_currentActivatedAnimation) == m_animationNameAndLength.end())
	{
		m_currentActivatedAnimation = m_animationNameAndLength.begin()->first;
		m_currentAnimationTime = 0.0f;
	}
	m_rootBone->Update(m_currentActivatedAnimation, m_currentAnimationTime);
    Transform t;
    m_rootBone->CalculateBoneTransform(t.m_value);
}

Animator::~Animator()
{
}
