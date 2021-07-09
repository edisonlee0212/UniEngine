#pragma once
#include <Animator.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <ResourceBehaviour.hpp>
#include <SkinnedMesh.hpp>
#include <Transform.hpp>

namespace UniEngine
{
enum class ModelNodeType
{
    Mesh,
    SkinnedMesh,
};
struct ModelNode
{
    ModelNodeType m_type = ModelNodeType::Mesh;
    std::string m_name;
    bool m_necessity = false;
    Transform m_localTransform;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<SkinnedMesh> m_skinnedMesh;
    std::shared_ptr<ModelNode> m_parent;
    std::vector<std::shared_ptr<ModelNode>> m_children;
};
class UNIENGINE_API Model : public ResourceBehaviour
{
    friend class ResourceManager;
    std::shared_ptr<ModelNode> m_rootNode;
    std::shared_ptr<Animation> m_animation;

  public:
    void OnCreate() override;
    std::shared_ptr<ModelNode> &RootNode();
};

} // namespace UniEngine