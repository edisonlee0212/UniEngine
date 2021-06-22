#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <ResourceBehaviour.hpp>
namespace UniEngine
{
struct ModelNode
{
    glm::mat4 m_localToParent;
    std::vector<std::pair<std::shared_ptr<Material>, std::shared_ptr<Mesh>>> m_meshMaterials;
    std::vector<std::unique_ptr<ModelNode>> m_children;
};
class UNIENGINE_API Model : public ResourceBehaviour
{
    std::unique_ptr<ModelNode> m_rootNode;

  public:
    Model();
    std::unique_ptr<ModelNode> &RootNode();
};
} // namespace UniEngine