#include <Model.hpp>
using namespace UniEngine;

void Model::OnCreate()
{
    m_rootNode = std::make_unique<ModelNode>();
    m_name = "New model";
}

std::unique_ptr<ModelNode> &Model::RootNode()
{
    return m_rootNode;
}
