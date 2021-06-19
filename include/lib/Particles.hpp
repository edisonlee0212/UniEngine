#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <World.hpp>
namespace UniEngine
{
class UNIENGINE_API Particles : public PrivateComponentBase
{
  public:
    glm::vec4 m_displayBoundColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.5f);
    bool m_displayBound = true;
    Particles();
    Bound m_boundingBox;
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::vector<glm::mat4> m_matrices;
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    void RecalculateBoundingBox();
    void OnGui() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine
