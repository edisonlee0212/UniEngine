#pragma once
#include <Application.hpp>
#include <Planet/TerrainChunk.hpp>
#include <Planet/TerrainConstructionStageBase.hpp>
using namespace UniEngine;
namespace Planet
{
struct PlanetInfo
{
    unsigned m_maxLodLevel;
    double m_lodDistance;
    double m_radius;
    unsigned m_index;
    unsigned m_resolution;
};

struct MeshInfo
{
    unsigned m_index;
    bool m_enabled;
    MeshInfo(const unsigned index, const bool enabled = true) : m_index(index), m_enabled(enabled){};
};

class PlanetTerrain : public IPrivateComponent
{
    friend class TerrainChunk;
    friend class PlanetTerrainSystem;
    std::vector<std::unique_ptr<TerrainChunk>> m_chunks;
    PlanetInfo m_info;
    // Used for fast mesh generation;
    std::vector<Vertex> m_sharedVertices;
    std::vector<unsigned> m_sharedTriangles;

  public:
    void Deserialize(const YAML::Node &in) override;
    void Serialize(YAML::Emitter &out) override;
    PlanetTerrain() = default;
    std::shared_ptr<Material> m_surfaceMaterial;
    std::vector<std::shared_ptr<TerrainConstructionStageBase>> m_terrainConstructionStages;
    void Init(PlanetInfo &info);
    void Init(PlanetInfo &info, std::shared_ptr<Material> surfaceMaterial);
    void OnGui() override;
};
} // namespace Planet
