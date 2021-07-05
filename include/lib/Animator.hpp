#pragma once
#include <OpenGLUtils.hpp>
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

struct UNIENGINE_API BoneAnimation
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
    std::map<std::string, BoneAnimation> m_animations;
    std::string m_name;
    Transform m_offsetMatrix = Transform();

    std::vector<std::shared_ptr<Bone>> m_children;

    bool m_isEntity = false;
    /**
     * \brief If the bone does not have an id, it must be bound to an Entity.
     */
    Entity m_attachedEntity;
    Transform m_localTransform = Transform();
    glm::mat4 m_boneTransform;
    glm::mat4 m_currentFinalMatrix;
    /* Interpolates b/w positions,rotations & scaling keys based on the current time of the
    animation and prepares the local transformation matrix by combining all keys transformations */
    void Update(const std::string &name, const float &animationTime);
    void RenderBones(const float& size, const glm::mat4 &parentTransform) const;
    void CalculateBoneTransform(const glm::mat4 &parentTransform);
};

#pragma endregion
class UNIENGINE_API Animator : public PrivateComponentBase
{
  public:
    std::map<std::string, float> m_animationNameAndLength;
    std::shared_ptr<Bone> m_rootBone;
    std::string m_currentActivatedAnimation;
    float m_currentAnimationTime;

    bool m_autoPlay = false;

    [[nodiscard]] std::shared_ptr<Bone> &UnsafeGetRootBone();
    void OnGui() override;
    Animator();
    void Animate();
    ~Animator() override;
};
} // namespace UniEngine