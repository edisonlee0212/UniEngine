#include <Planet/PlanetTerrain.hpp>
#include <yaml-cpp/yaml.h>
#include <Planet/PlanetTerrainSystem.hpp>
void Planet::PlanetTerrain::Serialize(YAML::Emitter& out)
{
	out << YAML::Key << "PlanetInfo";
	out << YAML::BeginMap;
	out << YAML::Key << "MaxLodLevel" << YAML::Value << _Info.MaxLodLevel;
	out << YAML::Key << "LodDistance" << YAML::Value << _Info.LodDistance;
	out << YAML::Key << "Radius" << YAML::Value << _Info.Radius;
	out << YAML::Key << "Index" << YAML::Value << _Info.Index;
	out << YAML::Key << "Resolution" << YAML::Value << _Info.Resolution;
	out << YAML::EndMap;
	
}

void Planet::PlanetTerrain::Deserialize(const YAML::Node& out)
{
	auto info = out["PlanetInfo"];
	PlanetInfo planetInfo;
	planetInfo.MaxLodLevel = info["MaxLodLevel"].as<unsigned>();
	planetInfo.LodDistance = info["LodDistance"].as<double>();
	planetInfo.Radius = info["Radius"].as<double>();
	planetInfo.Index = info["Index"].as<unsigned>();
	planetInfo.Resolution = info["Resolution"].as<unsigned>();
	Init(planetInfo);
}

void Planet::PlanetTerrain::Init(PlanetInfo& info)
{
	Init(info, PlanetTerrainSystem::_DefaultSurfaceMaterial);
}

void Planet::PlanetTerrain::Init(PlanetInfo& info, std::shared_ptr<Material> surfaceMaterial)
{
	_Info = info;
	SurfaceMaterial = std::move(surfaceMaterial);
	_SharedVertices = std::vector<Vertex>();
	size_t resolution = info.Resolution;
	_SharedVertices.resize(resolution * resolution);
	_SharedTriangles = std::vector<unsigned>();
	_SharedTriangles.resize((resolution - 1) * (resolution - 1) * 6);

	size_t triIndex = 0;
	for (size_t y = 0; y < resolution; y++)
	{
		for (size_t x = 0; x < resolution; x++)
		{
			size_t i = x + y * resolution;
			_SharedVertices[i].m_texCoords = glm::vec2(static_cast<float>(x) / (resolution - 1), static_cast<float>(y) / (resolution - 1));
			if (x != resolution - 1 && y != resolution - 1)
			{
				_SharedTriangles[triIndex] = i;
				_SharedTriangles[triIndex + 1] = i + resolution + 1;
				_SharedTriangles[triIndex + 2] = i + resolution;

				_SharedTriangles[triIndex + 3] = i;
				_SharedTriangles[triIndex + 4] = i + 1;
				_SharedTriangles[triIndex + 5] = i + resolution + 1;
				triIndex += 6;
			}
		}
	}
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(1, 0, 0)));
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 1, 0)));
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 0, 1)));
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(-1, 0, 0)));
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, -1, 0)));
	_Chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 0, -1)));

	std::mutex m;
	for (auto& chunk : _Chunks)
	{
		chunk->GenerateTerrain(m, chunk);
		chunk->Active = true;
	}
	SetEnabled(true);
}

void Planet::PlanetTerrain::OnGui()
{
	if (SurfaceMaterial) SurfaceMaterial->OnGui();
}

