#pragma once
#include <Material.hpp>
#include <Mesh.hpp>

namespace UniEngine
{
class UNIENGINE_API MeshRenderer : public IPrivateComponent
{
    friend class EditorManager;
    void RenderBound(glm::vec4 &color);
  public:
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    AssetRef m_mesh;
    AssetRef m_material;
    void OnGui() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Clone(const std::shared_ptr<IPrivateComponent>& target) override;
};
} // namespace UniEngine
