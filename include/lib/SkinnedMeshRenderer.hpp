#pragma once
#include <Material.hpp>
#include <SkinnedMesh.hpp>

namespace UniEngine
{
class UNIENGINE_API SkinnedMeshRenderer : public PrivateComponentBase
{
    friend class EditorManager;
    void RenderBound(glm::vec4 &color) const;

  public:
    glm::vec4 m_displayBoundColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
    bool m_displayBound = true;
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
