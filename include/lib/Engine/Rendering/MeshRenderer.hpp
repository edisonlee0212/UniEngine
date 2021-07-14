#pragma once
#include <Material.hpp>
#include <Mesh.hpp>

namespace UniEngine
{
class UNIENGINE_API MeshRenderer : public PrivateComponentBase
{
    friend class EditorManager;
    void RenderBound(glm::vec4 &color) const;

  public:
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    void OnGui() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine
