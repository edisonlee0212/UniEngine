#pragma once
#include <Animator.hpp>
#include <IAsset.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <SkinnedMesh.hpp>
#include <Transform.hpp>

namespace UniEngine
{
class UNIENGINE_API Prefab
{
  private:
    std::vector<IDataComponent *> m_dataComponents;
    std::vector<IPrivateComponent *> m_privateComponents;
    std::vector<Prefab> m_children;
  public:

};

class UNIENGINE_API PrefabHolder : public IAsset{
  public:
    std::shared_ptr<Prefab> m_root;
    void OnCreate() override;
};
} // namespace UniEngine