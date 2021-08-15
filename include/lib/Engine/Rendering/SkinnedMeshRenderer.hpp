#pragma once
#include "Animator.hpp"
#include <Material.hpp>
#include <SkinnedMesh.hpp>

namespace UniEngine
{
class UNIENGINE_API BoneMatrices
{
  public:
    std::vector<glm::mat4> m_value;
    void UploadBones(const std::shared_ptr<SkinnedMesh> &skinnedMesh) const;
};

class UNIENGINE_API SkinnedMeshRenderer : public IPrivateComponent
{
    friend class EditorManager;
    friend class Animator;
    friend class AnimationManager;
    friend class Prefab;
    void RenderBound(glm::vec4 &color);
    void GetBoneMatrices();
    std::shared_ptr<BoneMatrices> m_finalResults;
    friend class RenderManager;
    PrivateComponentRef m_animator;
  public:
    void AttachAnimator(const std::shared_ptr<Animator> &animator);
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    AssetRef m_skinnedMesh;
    AssetRef m_material;
    void OnGui() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Relink(const std::unordered_map<Handle, Handle> &map) override;
    void Clone(const std::shared_ptr<IPrivateComponent> &target) override;
};


} // namespace UniEngine
