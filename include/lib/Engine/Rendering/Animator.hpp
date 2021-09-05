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
    std::vector<EntityRef> m_boundEntities;


    AssetRef m_animation;
    bool m_needAnimationSetup = true;
    size_t m_boneSize = 0;
    void BoneSetter(const std::shared_ptr<Bone> &boneWalker);
    void Setup();

    bool m_animatedCurrentFrame = false;
  public:
    bool m_needAnimate = true;
    // Create an animator which every bone is attached to an Entity.
    void Setup(
        std::vector<Entity> &boundEntities, std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices);
    void ApplyOffsetMatrices();
    void DebugBoneRender(const glm::vec4 &color, const float &size);
    void ResetTransform(const int &index);
    bool m_autoPlay = false;
    std::string m_currentActivatedAnimation;
    float m_currentAnimationTime;
    void AutoPlay();
    bool AnimatedCurrentFrame();
    void Setup(const std::shared_ptr<Animation> &targetAnimation);
    void OnInspect() override;
    void Animate();
    void Clone(const std::shared_ptr<IPrivateComponent>& target) override;
    std::shared_ptr<Animation> GetAnimation();
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;
    void Relink(const std::unordered_map<Handle, Handle> &map) override;
};
} // namespace UniEngine