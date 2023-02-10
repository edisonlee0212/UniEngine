#pragma once
#include "Material.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
namespace UniEngine
{

class UNIENGINE_API ParticleMatrices : ISerializable{
    std::shared_ptr<OpenGLUtils::GLBuffer> m_buffer;
    std::shared_ptr<OpenGLUtils::GLBuffer> m_colorBuffer;
    bool m_bufferReady = false;
    friend class Mesh;
    friend class Strands;
    friend class SkinnedMesh;
    size_t m_version = 0;

    std::vector<glm::vec4> m_colors;
    std::vector<glm::mat4> m_matrices;
  public:
	void Reset();
    [[nodiscard]] size_t GetVersion() const;
    ParticleMatrices();
    std::vector<glm::vec4>& RefColors();
    std::vector<glm::mat4>& RefMatrices();
    const std::vector<glm::vec4>& PeekColors() const;
    const std::vector<glm::mat4>& PeekMatrices() const;

    void SetValue(const std::vector<glm::vec4>& colors, const std::vector<glm::mat4>& matrices);
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
