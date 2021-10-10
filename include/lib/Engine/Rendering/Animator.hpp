#pragma once
#include <Animation.hpp>
#include <Core/OpenGLUtils.hpp>
#include <Scene.hpp>
#include <Transform.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API Animator : public IPrivateComponent
{
    std::vector<std::shared_ptr<Bone>> m_bones;
    friend class SkinnedMeshRenderer;
    friend class AnimationManager;

    std::vector<glm::mat4> m_transformChain;
    std::vector<glm::mat4> m_offsetMatrices;
    std::vector<std::string> m_names;

    AssetRef m_animation;
    bool m_needAnimationSetup = true;
    size_t m_boneSize = 0;
    void BoneSetter(const std::shared_ptr<Bone> &boneWalker);
    void Setup();
    bool m_animatedCurrentFrame = false;
  public:
    bool m_needAnimate = true;
    /**
     * Only set offset matrices, so the animator can be used as ragDoll.
     * @param name Name of the bones
     * @param offsetMatrices The collection of offset matrices.
     */
    void Setup(std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices);
    void ApplyOffsetMatrices();

    glm::mat4 GetReverseTransform(const int &index, const Entity& entity);
    bool m_autoPlay = false;
    std::string m_currentActivatedAnimation;
    float m_currentAnimationTime;
    void AutoPlay();
    bool AnimatedCurrentFrame();
    void Setup(const std::shared_ptr<Animation> &targetAnimation);
    void OnInspect() override;
    void Animate();
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
    std::shared_ptr<Animation> GetAnimation();
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;
};
} // namespace UniEngine