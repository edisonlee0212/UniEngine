#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Transform.hpp>
#include <World.hpp>
#include <uniengine_export.h>
#include <Animation.hpp>
namespace UniEngine
{
class UNIENGINE_API Animator : public IPrivateComponent
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