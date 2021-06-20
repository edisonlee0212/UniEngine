#pragma once
#include <Application.hpp>
using namespace UniEngine;
namespace Planet {
	enum class ChunkDirection {
		Root,
		UpperLeft,
		UpperRight,
		LowerLeft,
		LowerRight
	};
	class PlanetTerrain;
	class TerrainChunk {
		PlanetTerrain* _PlanetTerrain;
	public:
		std::unique_ptr<Mesh> _Mesh;
		//The level of detail, the larger the detail, the smaller the chunk will be.
		unsigned DetailLevel;
		//The chunk coordinate in which a chunk belongs to the face
		glm::ivec2 ChunkCoordinate;
		ChunkDirection Direction;
		TerrainChunk* Parent;
		
		bool ChildrenActive = false;
		bool Active = false;
		//The index of four children, upperleft = 0, upperright = 1, lower left = 2, lower right = 3.
		std::unique_ptr<TerrainChunk> C0;
		std::unique_ptr<TerrainChunk> C1;
		std::unique_ptr<TerrainChunk> C2;
		std::unique_ptr<TerrainChunk> C3;
		glm::dvec3 LocalUp;
		glm::dvec3 AxisA;
		glm::dvec3 AxisB;
		glm::dvec3 ChunkCenterPosition(glm::dvec3 planetPosition, double radius, glm::quat rotation);
		TerrainChunk(PlanetTerrain* planetTerrain, TerrainChunk* parent, unsigned detailLevel, glm::ivec2 chunkCoordinate, ChunkDirection direction, glm::dvec3 localUp);
		void Expand(std::mutex& mutex);
		void GenerateTerrain(std::mutex& mutex, std::unique_ptr<TerrainChunk>& targetChunk) const;
		void Collapse();
	};
}
