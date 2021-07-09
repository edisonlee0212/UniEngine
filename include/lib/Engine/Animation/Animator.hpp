#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Transform.hpp>
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
        std::vector<Entity> &boundEntities,
        std::vector<glm::mat4> &results);
    void OnGui();
};

#pragma endregion
class UNIENGINE_API Animation : public ResourceBehaviour
{
  public:
    std::map<std::string, float> m_animationNameAndLength;
    std::shared_ptr<Bone> m_rootBone;
    size_t m_boneSize;
    [[nodiscard]] std::shared_ptr<Bone> &UnsafeGetRootBone();
    Animation();
    void OnGui() const;
    void Animate(
        const std::string &name,
        const float &animationTime,
        const glm::mat4 &rootTransform,
        std::vector<Entity> &boundEntities,
        std::vector<glm::mat4> &results);
};
class UNIENGINE_API Animator : public PrivateComponentBase
{
    std::vector<std::shared_ptr<Bone>> m_bones;
    friend class SkinnedMeshRenderer;
    void BoneSetter(const std::shared_ptr<Bone> &boneWalker);
    std::vector<glm::mat4> m_transformChain;
    std::vector<glm::mat4> m_offsetMatrices;
    std::vector<std::string> m_names;
    std::vector<Entity> m_boundEntities;

  public:
    size_t m_boneSize = 0;
    // Create an animator which every bone is attached to an Entity.
    void Setup(
        std::vector<Entity> &boundEntities, std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices);
    void ApplyOffsetMatrices();
    void DebugBoneRender(const glm::vec4 &color, const float &size) const;
    void ResetTransform(const int &index);
    std::shared_ptr<Animation> m_animation;
    bool m_autoPlay = true;
    std::string m_currentActivatedAnimation;
    float m_currentAnimationTime;
    void AutoPlay();
    void Setup(const std::shared_ptr<Animation> &targetAnimation);
    void OnGui() override;
    void Animate();
};
} // namespace UniEngine