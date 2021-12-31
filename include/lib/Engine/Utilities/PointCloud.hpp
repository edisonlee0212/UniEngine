#pragma once
#include <Application.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class UNIENGINE_API PointCloud : public IAsset
{
    glm::dvec3 m_min;
    glm::dvec3 m_max;
  public:
    glm::dvec3 m_offset;
    std::vector<glm::dvec3> m_points;
    float m_pointSize = 0.01f;
    float m_compressFactor = 0.01f;
    void OnCreate() override;
    void Load(const std::filesystem::path &path);
    void Save(const std::filesystem::path &path);
    void OnInspect() override;
    void Compress(std::vector<glm::dvec3>& points);
    void ApplyCompressed();
    void ApplyOriginal();
    void RecalculateBoundingBox();
    void Crop(std::vector<glm::dvec3>& points, const glm::dvec3& min, const glm::dvec3& max);
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

};
} // namespace UniEngine
