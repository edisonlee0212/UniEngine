#pragma once
#include <Material.hpp>
#include <Mesh.hpp>
#include <Scene.hpp>
namespace UniEngine
{

class UNIENGINE_API PointCloud : ISerializable{
  public:
    std::vector<glm::mat4> m_value;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Load(const std::filesystem::path &path);
};

class UNIENGINE_API Particles : public IPrivateComponent
{
  public:
    void OnCreate() override;
    Bound m_boundingBox;
    bool m_forwardRendering = false;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::shared_ptr<PointCloud> m_matrices;
    AssetRef m_mesh;
    AssetRef m_material;
    void RecalculateBoundingBox();
    void OnGui() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void Clone(const std::shared_ptr<IPrivateComponent>& target) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;

};
} // namespace UniEngine
