#pragma once
#include <Application.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class UNIENGINE_API PointCloud : public IPrivateComponent
{
    Bound m_boundingBox;
  public:
    std::vector<glm::vec3> m_compressed;
    std::vector<glm::vec3> m_points;
    float m_pointSize = 1.0f;
    float m_compressFactor = 50.0f;
    float m_scale = 1.0f;
    void OnCreate() override;
    void Load(const std::filesystem::path &path);
    void Save(const std::filesystem::path &path);
    void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target) override;
    void OnInspect() override;
    void Compress();
    void ApplyCompressed();
    void ApplyOriginal();
    void RecalculateBoundingBox();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

};
} // namespace UniEngine
