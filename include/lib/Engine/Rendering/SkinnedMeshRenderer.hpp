#pragma once
#include <Material.hpp>
#include <SkinnedMesh.hpp>

namespace UniEngine
{
class UNIENGINE_API SkinnedMeshRenderer : public IPrivateComponent
{
    friend class EditorManager;
    friend class Animator;
    friend class AnimationManager;
    void RenderBound(glm::vec4 &color) const;
    void GetBoneMatrices();
    std::vector<glm::mat4> m_finalResults;

    friend class RenderManager;

  public:
    Entity m_animator;
    void AttachAnimator(const Entity &animator);
    void UploadBones();
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::shared_ptr<SkinnedMesh> m_skinnedMesh;
    std::shared_ptr<Material> m_material;
    void OnGui() override;
    SkinnedMeshRenderer();
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

} // namespace UniEngine
