#include <Planet/PlanetTerrain.hpp>
#include <yaml-cpp/yaml.h>
#include <Planet/PlanetTerrainSystem.hpp>
void Planet::PlanetTerrain::Serialize(YAML::Emitter& out)
{
	out << YAML::Key << "PlanetInfo";
	out << YAML::BeginMap;
	out << YAML::Key << "MaxLodLevel" << YAML::Value << m_info.m_maxLodLevel;
	out << YAML::Key << "LodDistance" << YAML::Value << m_info.m_lodDistance;
	out << YAML::Key << "Radius" << YAML::Value << m_info.m_radius;
	out << YAML::Key << "Index" << YAML::Value << m_info.m_index;
	out << YAML::Key << "Resolution" << YAML::Value << m_info.m_resolution;
	out << YAML::EndMap;
	
}

void Planet::PlanetTerrain::Deserialize(const YAML::Node& out)
{
	auto info = out["PlanetInfo"];
	PlanetInfo planetInfo;
	planetInfo.m_maxLodLevel = info["MaxLodLevel"].as<unsigned>();
	planetInfo.m_lodDistance = info["LodDistance"].as<double>();
	planetInfo.m_radius = info["Radius"].as<double>();
	planetInfo.m_index = info["Index"].as<unsigned>();
	planetInfo.m_resolution = info["Resolution"].as<unsigned>();
	Init(planetInfo);
}

void Planet::PlanetTerrain::Init(PlanetInfo& info)
{
	Init(info, PlanetTerrainSystem::m_defaultSurfaceMaterial);
}

void Planet::PlanetTerrain::Init(PlanetInfo& info, std::shared_ptr<Material> surfaceMaterial)
{
	m_info = info;
	m_surfaceMaterial = std::move(surfaceMaterial);
	m_sharedVertices = std::vector<Vertex>();
	size_t resolution = info.m_resolution;
	m_sharedVertices.resize(resolution * resolution);
	m_sharedTriangles = std::vector<unsigned>();
	m_sharedTriangles.resize((resolution - 1) * (resolution - 1) * 6);

	size_t triIndex = 0;
	for (size_t y = 0; y < resolution; y++)
	{
		for (size_t x = 0; x < resolution; x++)
		{
			size_t i = x + y * resolution;
			m_sharedVertices[i].m_texCoords = glm::vec2(static_cast<float>(x) / (resolution - 1), static_cast<float>(y) / (resolution - 1));
			if (x != resolution - 1 && y != resolution - 1)
			{
				m_sharedTriangles[triIndex] = i;
				m_sharedTriangles[triIndex + 1] = i + resolution + 1;
				m_sharedTriangles[triIndex + 2] = i + resolution;

				m_sharedTriangles[triIndex + 3] = i;
				m_sharedTriangles[triIndex + 4] = i + 1;
				m_sharedTriangles[triIndex + 5] = i + resolution + 1;
				triIndex += 6;
			}
		}
	}
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(1, 0, 0)));
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 1, 0)));
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 0, 1)));
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(-1, 0, 0)));
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, -1, 0)));
	m_chunks.push_back(std::make_unique<TerrainChunk>(this, nullptr, 0, glm::ivec2(0), ChunkDirection::Root, glm::dvec3(0, 0, -1)));

	std::mutex m;
	for (auto& chunk : m_chunks)
	{
		chunk->GenerateTerrain(m, chunk);
		chunk->Active = true;
	}
	SetEnabled(true);
}

void Planet::PlanetTerrain::OnGui()
{
	if (m_surfaceMaterial) m_surfaceMaterial->OnGui();
}

