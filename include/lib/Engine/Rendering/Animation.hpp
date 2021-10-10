#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Scene.hpp>
#include <Transform.hpp>
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

struct UNIENGINE_API BoneKeyFrames
{
    std::vector<BonePosition> m_positions;
    std::vector<BoneRotation> m_rotations;
    std::vector<BoneScale> m_scales;
    float m_maxTimeStamp = 0.0f;
    /* Gets the current index on mKeyPositions to interpolate to based on the current
    animation time */
    int GetPositionIndex(const float &animationTime);

    /* Gets the current index on mKeyRotations to interpolate to based on the current
    animation time */
    int GetRotationIndex(const float &animationTime);

    /* Gets the current index on mKeyScalings to interpolate to based on the current
    animation time */
    int GetScaleIndex(const float &animationTime);

    /* Gets normalized value for Lerp & Slerp*/
    static float GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime);

    /* figures out which position keys to interpolate b/w and performs the interpolation
    and returns the translation matrix */
    glm::mat4 InterpolatePosition(const float &animationTime);

    /* figures out which rotations keys to interpolate b/w and performs the interpolation
    and returns the rotation matrix */
    glm::mat4 InterpolateRotation(const float &animationTime);

    /* figures out which scaling keys to interpolate b/w and performs the interpolation
    and returns the scale matrix */
    glm::mat4 InterpolateScaling(const float &animationTime);

    void Serialize(YAML::Emitter &out) const;
    void Deserialize(const YAML::Node &in);
};

struct UNIENGINE_API Bone
{
    std::map<std::string, BoneKeyFrames> m_animations;
    std::string m_name;
    Transform m_offsetMatrix = Transform();
    size_t m_index;
    std::vector<std::shared_ptr<Bone>> m_children;
    /* Interpolates b/w positions,rotations & scaling keys based on the current time of the
    animation and prepares the local transformation matrix by combining all keys transformations */
    void Animate(
        const std::string &name,
        const float &animationTime,
        const glm::mat4 &parentTransform,
        const glm::mat4 &rootTransform,
        std::vector<glm::mat4> &results);
    void OnInspect();

    void Serialize(YAML::Emitter &out) const;
    void Deserialize(const YAML::Node &in);
};

#pragma endregion
class UNIENGINE_API Animation : public IAsset
{
  public:
    std::map<std::string, float> m_animationNameAndLength;
    std::shared_ptr<Bone> m_rootBone;
    size_t m_boneSize = 0;
    [[nodiscard]] std::shared_ptr<Bone> &UnsafeGetRootBone();
    void OnInspect() override;
    void Animate(
        const std::string &name,
        const float &animationTime,
        const glm::mat4 &rootTransform,
        std::vector<glm::mat4> &results);

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine