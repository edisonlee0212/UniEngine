#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <ResourceBehaviour.hpp>
#include <Transform.hpp>
namespace UniEngine
{
struct ModelNode
{
    Transform m_localToParent;
    std::vector<std::pair<std::shared_ptr<Material>, std::shared_ptr<Mesh>>> m_meshMaterials;
    std::vector<std::unique_ptr<ModelNode>> m_children;
};
class UNIENGINE_API Model : public ResourceBehaviour
{
    std::unique_ptr<ModelNode> m_rootNode;

  public:
    void OnCreate() override;
    std::unique_ptr<ModelNode> &RootNode();
};
} // namespace UniEngine