#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <Scene.hpp>
namespace UniEngine
{

class UNIENGINE_API ParticleMatrices : ISerializable{
    std::shared_ptr<OpenGLUtils::GLVBO> m_buffer;
    bool m_bufferReady = false;
    friend class Mesh;
    friend class SkinnedMesh;
    size_t m_version = 0;
  public:
    [[nodiscard]] size_t &GetVersion();
    ParticleMatrices();
    std::vector<glm::mat4> m_value;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Update();
};

class UNIENGINE_API Particles : public IPrivateComponent
{
  public:
    void OnCreate() override;
    Bound m_boundingBox;
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::shared_ptr<ParticleMatrices> m_matrices;
    AssetRef m_mesh;
    AssetRef m_material;
    void RecalculateBoundingBox();
    void OnInspect() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;
    void OnDestroy() override;
};
} // namespace UniEngine
