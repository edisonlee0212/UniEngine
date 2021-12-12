#pragma once
#include <Application.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class UNIENGINE_API PointCloud : public IAsset
{
    Bound m_boundingBox;
  public:
    bool m_recenter = true;
    std::vector<glm::vec3> m_points;
    float m_pointSize = 0.01f;
    float m_compressFactor = 0.01f;
    float m_scale = 1.0f;
    void OnCreate() override;
    void Load(const std::filesystem::path &path);
    void Save(const std::filesystem::path &path);
    void OnInspect() override;
    void Compress(std::vector<glm::vec3>& points);
    void ApplyCompressed();
    void ApplyOriginal();
    void RecalculateBoundingBox();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

};
} // namespace UniEngine
