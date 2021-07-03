#pragma once
#include <OpenGLUtils.hpp>
#include <World.hpp>
#include <uniengine_export.h>

namespace UniEngine
{
#pragma region Bone
struct UNIENGINE_API BonePosition
{
    glm::vec3 m_value;
    float m_timeStamp;
};

struct UNIENGINE_API BoneRotation
{
    glm::quat m_value;
    float m_timeStamp;
};

struct UNIENGINE_API BoneScale
{
    glm::vec3 m_value;
    float m_timeStamp;
};
struct UNIENGINE_API Bone
{
    std::vector<BonePosition> m_positions;
    std::vector<BoneRotation> m_rotations;
    std::vector<BoneScale> m_scales;
    std::string m_name;
    glm::mat4 m_offsetMatrix = glm::mat4(0.0f);

    std::vector<std::shared_ptr<Bone>> m_children;   
    /**
     * \brief If the bone does not have an id, it must be bound to an Entity.
     */
    Entity m_attachedEntity;
    glm::mat4 m_localToParent = glm::mat4(0.0f);
    /*reads keyframes from aiNodeAnim*/
    void ReadKeyFrames(const aiNodeAnim *channel)
    {
        auto numPositions = channel->mNumPositionKeys;
        m_positions.resize(numPositions);
        for (int positionIndex = 0; positionIndex < numPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            BonePosition data;
            data.m_value = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
            data.m_timeStamp = timeStamp;
            m_positions.push_back(data);
        }

        auto numRotations = channel->mNumRotationKeys;
        m_rotations.resize(numRotations);
        for (int rotationIndex = 0; rotationIndex < numRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            BoneRotation data;
            data.m_value = glm::quat(aiOrientation.x, aiOrientation.y, aiOrientation.z, aiOrientation.w);
            data.m_timeStamp = timeStamp;
            m_rotations.push_back(data);
        }

        auto numScales = channel->mNumScalingKeys;
        m_scales.resize(numScales);
        for (int keyIndex = 0; keyIndex < numScales; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            BoneScale data;
            data.m_value = glm::vec3(scale.x, scale.y, scale.z);
            data.m_timeStamp = timeStamp;
            m_scales.push_back(data);
        }
    }

    /* Interpolates b/w positions,rotations & scaling keys based on the curren time of the
    animation and prepares the local transformation matrix by combining all keys tranformations */
    void Update(const float& animationTime)
    {
        const auto translation = InterpolatePosition(animationTime);
        const auto rotation = InterpolateRotation(animationTime);
        const auto scale = InterpolateScaling(animationTime);
        m_localToParent = translation * rotation * scale;
        for (auto& i : m_children)
        {
            i->Update(animationTime);
        }
    }

    /* Gets the current index on mKeyPositions to interpolate to based on the current
    animation time */
    int GetPositionIndex(const float &animationTime)
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

    /* Gets the current index on mKeyRotations to interpolate to based on the current
    animation time */
    int GetRotationIndex(const float &animationTime)
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

    /* Gets the current index on mKeyScalings to interpolate to based on the current
    animation time */
    int GetScaleIndex(const float &animationTime)
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

  private:
    /* Gets normalized value for Lerp & Slerp*/
    static float GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    /* figures out which position keys to interpolate b/w and performs the interpolation
    and returns the translation matrix */
    glm::mat4 InterpolatePosition(const float &animationTime)
    {
        if (1 == m_positions.size())
            return glm::translate(glm::mat4(1.0f), m_positions[0].m_value);

        const int p0Index = GetPositionIndex(animationTime);
        const int p1Index = p0Index + 1;
        const float scaleFactor =
            GetScaleFactor(m_positions[p0Index].m_timeStamp, m_positions[p1Index].m_timeStamp, animationTime);
        const glm::vec3 finalPosition = glm::mix(m_positions[p0Index].m_value, m_positions[p1Index].m_value, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    /* figures out which rotations keys to interpolate b/w and performs the interpolation
    and returns the rotation matrix */
    glm::mat4 InterpolateRotation(const float &animationTime)
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

    /* figures out which scaling keys to interpolate b/w and performs the interpolation
    and returns the scale matrix */
    glm::mat4 InterpolateScaling(const float &animationTime)
    {
        if (1 == m_scales.size())
            return glm::scale(m_scales[0].m_value);

        const int p0Index = GetScaleIndex(animationTime);
        const int p1Index = p0Index + 1;
        const float scaleFactor = GetScaleFactor(m_scales[p0Index].m_timeStamp, m_scales[p1Index].m_timeStamp, animationTime);
        const glm::vec3 finalScale = glm::mix(m_scales[p0Index].m_value, m_scales[p1Index].m_value, scaleFactor);
        return glm::scale(finalScale);
    }
};

#pragma endregion
class UNIENGINE_API Animation : public PrivateComponentBase
{
    friend class ResourceManager;
    std::shared_ptr<Bone> m_rootBone;

  public:
    [[nodiscard]] std::shared_ptr<Bone> &UnsafeGetRootBone();
    void OnGui() override;
    Animation();
    ~Animation() override;
};
}