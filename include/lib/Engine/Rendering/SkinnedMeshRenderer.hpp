#pragma once
#include "Animator.hpp"
#include <Material.hpp>
#include <SkinnedMesh.hpp>
#include "PrivateComponentRef.hpp"
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
    friend class AnimationLayer;
    friend class Prefab;
    friend class RenderLayer;
    void RenderBound(glm::vec4 &color);
    void GetBoneMatrices();

    friend class RenderManager;

    bool m_ragDoll = false;

    std::vector<glm::mat4> m_ragDollTransformChain;
    std::vector<EntityRef> m_boundEntities;

    void DebugBoneRender(const glm::vec4 &color, const float &size);

  public:
    bool m_ragDollFreeze = false;
    [[nodiscard]] bool RagDoll() const;
    void SetRagDoll(bool value);
    PrivateComponentRef m_animator;
    std::shared_ptr<BoneMatrices> m_finalResults;
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    AssetRef m_skinnedMesh;
    AssetRef m_material;
    void OnInspect() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Relink(const std::unordered_map<Handle, Handle> &map, const std::shared_ptr<Scene> &scene) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target) override;

    [[nodiscard]] size_t GetRagDollBoneSize() const;
    void SetRagDollBoundEntity(int index, const Entity& entity, bool resetTransform = true);
    void SetRagDollBoundEntities(const std::vector<Entity>& entities, bool resetTransform = true);
};


} // namespace UniEngine
