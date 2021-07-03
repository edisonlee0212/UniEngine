#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <ResourceBehaviour.hpp>
#include <Transform.hpp>
#include <SkinnedMesh.hpp>
#include <Animation.hpp>

namespace UniEngine
{
struct ModelNode
{
    Transform m_localToParent;
    friend class ResourceManager;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<SkinnedMesh> m_skinnedMesh;

    std::vector<std::unique_ptr<ModelNode>> m_children;
};
class UNIENGINE_API Model : public ResourceBehaviour
{
    friend class ResourceManager;
    std::unique_ptr<ModelNode> m_rootNode;
    std::shared_ptr<Animation> m_animation;
  public:
    void OnCreate() override;
    std::unique_ptr<ModelNode> &RootNode();
};

} // namespace UniEngine