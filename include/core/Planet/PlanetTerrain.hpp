#pragma once
#include <Application.hpp>
#include <Planet/TerrainChunk.hpp>
#include <Planet/TerrainConstructionStageBase.hpp>
using namespace UniEngine;
namespace Planet {
	struct PlanetInfo {
		unsigned MaxLodLevel;
		double LodDistance;
		double Radius;
		unsigned Index;
		unsigned Resolution;
	};

	struct MeshInfo {
		unsigned Index;
		bool Enabled;
		MeshInfo(unsigned index, bool enabled = true) : Index(index), Enabled(enabled) {};
	};

	class PlanetTerrain : public PrivateComponentBase
	{
		friend class TerrainChunk;
		friend class PlanetTerrainSystem;
		std::vector<std::unique_ptr<TerrainChunk>> _Chunks;
		PlanetInfo _Info;
		//Used for fast mesh generation;
		std::vector<Vertex> _SharedVertices;
		std::vector<unsigned> _SharedTriangles;
	public:
		void Deserialize(const YAML::Node& in) override;
		void Serialize(YAML::Emitter& out) override;
		PlanetTerrain() = default;
		std::shared_ptr<Material> SurfaceMaterial;
		std::vector<std::shared_ptr<TerrainConstructionStageBase>> TerrainConstructionStages;
		void Init(PlanetInfo& info);
		void Init(PlanetInfo& info, std::shared_ptr<Material> surfaceMaterial);
		void OnGui() override;
	};
}
