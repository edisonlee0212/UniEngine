#pragma once
#include "Application.hpp"
#include "Entity.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
namespace UniEngine
{

class UNIENGINE_API PointCloud : public IAsset
{
    glm::dvec3 m_min;
    glm::dvec3 m_max;
  public:
    glm::dvec3 m_offset;
    bool m_hasPositions = false;
    bool m_hasNormals = false;
    bool m_hasColors = false;
    std::vector<glm::dvec3> m_points;
    std::vector<glm::dvec3> m_normals;
    std::vector<glm::vec4> m_colors;
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
    void DebugRendering(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Camera>& camera, const glm::vec3 &cameraPosition,
                        const glm::quat &cameraRotation, float pointSize = 1.0f) const;
};
} // namespace UniEngine
