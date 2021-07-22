#pragma once
#include <Animator.hpp>
#include <IAsset.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <SkinnedMesh.hpp>
#include <Transform.hpp>

namespace UniEngine
{
class Prefab : public IAsset
{
  private:
    std::vector<IDataComponent *> m_dataComponents;
    std::vector<IPrivateComponent *> m_privateComponents;
    std::vector<Prefab> m_children;
  public:
    void OnCreate() override;
};
} // namespace UniEngine