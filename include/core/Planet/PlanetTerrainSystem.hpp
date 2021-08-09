#pragma once
#include <Application.hpp>
#include <Planet/PlanetTerrain.hpp>
using namespace UniEngine;
namespace Planet
{
class PlanetTerrainSystem : public ISystem
{
    friend class PlanetTerrain;
    static std::shared_ptr<Material> m_defaultSurfaceMaterial;

  public:
    void OnCreate() override;
    void Update() override;
    void FixedUpdate() override;
    static std::shared_ptr<Material> GetDefaultSurfaceMaterial();
    void CheckLod(
        std::mutex &mutex,
        std::unique_ptr<TerrainChunk> &chunk,
        const PlanetInfo &info,
        const GlobalTransform &planetTransform,
        const GlobalTransform &cameraTransform);
    void RenderChunk(
        std::unique_ptr<TerrainChunk> &chunk,
        const std::shared_ptr<Material> &material,
        glm::mat4 &matrix,
        const std::shared_ptr<Camera> &camera,
        bool receiveShadow) const;
};
} // namespace Planet