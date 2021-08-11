#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <Scene.hpp>
namespace UniEngine
{

class UNIENGINE_API ParticleMatrices : ISerializable{
  public:
    std::vector<glm::mat4> m_value;
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
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    void RecalculateBoundingBox();
    void OnGui() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Clone(const std::shared_ptr<IPrivateComponent>& target) override;
};
} // namespace UniEngine
