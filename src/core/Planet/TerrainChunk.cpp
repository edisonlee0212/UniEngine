#include <Planet/TerrainChunk.hpp>
#include <Planet/PlanetTerrainSystem.hpp>

glm::dvec3 Planet::TerrainChunk::ChunkCenterPosition(glm::dvec3 planetPosition, double radius, glm::quat rotation)
{

	int actualDetailLevel = (int)glm::pow(2, DetailLevel);
	glm::dvec2 percent = glm::dvec2(0.5, 0.5) / (double)actualDetailLevel;
	glm::dvec3 point = LocalUp + (percent.x + (double)ChunkCoordinate.x / (double)actualDetailLevel - 0.5) * 2 * AxisA + (percent.y + (double)ChunkCoordinate.y / (double)actualDetailLevel - 0.5) * 2 * AxisB;
	double x = rotation.x * 2.0f;
	double y = rotation.y * 2.0f;
	double z = rotation.z * 2.0f;
	double xx = rotation.x * x;
	double yy = rotation.y * y;
	double zz = rotation.z * z;
	double xy = rotation.x * y;
	double xz = rotation.x * z;
	double yz = rotation.y * z;
	double wx = rotation.w * x;
	double wy = rotation.w * y;
	double wz = rotation.w * z;

	glm::dvec3 res;
	res.x = (1.0f - (yy + zz)) * point.x + (xy - wz) * point.y + (xz + wy) * point.z;
	res.y = (xy + wz) * point.x + (1.0f - (xx + zz)) * point.y + (yz - wx) * point.z;
	res.z = (xz - wy) * point.x + (yz + wx) * point.y + (1.0f - (xx + yy)) * point.z;
	res = glm::normalize(res);
	glm::dvec3 ret = res * radius + planetPosition;
	return ret;

}

Planet::TerrainChunk::TerrainChunk(PlanetTerrain* planetTerrain, TerrainChunk* parent, unsigned detailLevel, glm::ivec2 chunkCoordinate, ChunkDirection direction, glm::dvec3 localUp)
{
	_PlanetTerrain = planetTerrain;
	ChunkCoordinate = chunkCoordinate;
	DetailLevel = detailLevel;
	Parent = parent;
	LocalUp = localUp;
	AxisA = glm::dvec3(localUp.y, localUp.z, localUp.x);
	AxisB = glm::cross(localUp, AxisA);
	LocalUp = glm::normalize(LocalUp);
}

void Planet::TerrainChunk::Expand(std::mutex& mutex)
{
	if (!Active) return;
	if (!C0)
	{
		auto chunk0 = std::make_unique<TerrainChunk>(_PlanetTerrain, this, DetailLevel + 1, glm::ivec2(ChunkCoordinate.x * 2, ChunkCoordinate.y * 2 + 1), ChunkDirection::UpperLeft, LocalUp);
		GenerateTerrain(mutex, chunk0);
		C0 = std::move(chunk0);
	}
	if (!C1)
	{
		auto chunk1 = std::make_unique<TerrainChunk>(_PlanetTerrain, this, DetailLevel + 1, glm::ivec2(ChunkCoordinate.x * 2 + 1, ChunkCoordinate.y * 2 + 1), ChunkDirection::UpperRight, LocalUp);
		GenerateTerrain(mutex, chunk1);
		C1 = std::move(chunk1);
	}
	if (!C2)
	{
		auto chunk2 = std::make_unique<TerrainChunk>(_PlanetTerrain, this, DetailLevel + 1, glm::ivec2(ChunkCoordinate.x * 2, ChunkCoordinate.y * 2), ChunkDirection::LowerLeft, LocalUp);
		GenerateTerrain(mutex, chunk2);
		C2 = std::move(chunk2);
	}
	if (!C3)
	{
		auto chunk3 = std::make_unique<TerrainChunk>(_PlanetTerrain, this, DetailLevel + 1, glm::ivec2(ChunkCoordinate.x * 2 + 1, ChunkCoordinate.y * 2), ChunkDirection::LowerRight, LocalUp);
		GenerateTerrain(mutex, chunk3);
		C3 = std::move(chunk3);
	}
	C0->Active = true;
	C1->Active = true;
	C2->Active = true;
	C3->Active = true;
	Active = false;
	ChildrenActive = true;
}

void Planet::TerrainChunk::GenerateTerrain(std::mutex& mutex, std::unique_ptr<TerrainChunk>& targetChunk) const
{
	if (targetChunk->_Mesh) {
		Debug::Error("Mesh Exist!");
	}
	std::vector<Vertex>& vertices = _PlanetTerrain->_SharedVertices;
	auto size = vertices.size();
	auto resolution = _PlanetTerrain->_Info.Resolution;
	for (auto index = 0; index < size; index++) {
		int actualDetailLevel = (int)glm::pow(2, targetChunk->DetailLevel);
		int x = index % resolution;
		int y = index / resolution;
		glm::dvec2 percent = glm::dvec2(x, y) / (double)(resolution - 1) / (double)actualDetailLevel;
		glm::dvec2 globalPercent = 45.0 * glm::dvec2((percent.x + (double)targetChunk->ChunkCoordinate.x / actualDetailLevel - 0.5) * 2.0, (percent.y + (double)targetChunk->ChunkCoordinate.y / actualDetailLevel - 0.5) * 2.0);
		glm::dvec2 actualPercent = glm::dvec2(glm::tan(glm::radians(globalPercent.x)), glm::tan(glm::radians(globalPercent.y)));
		glm::dvec3 pointOnUnitCube = targetChunk->LocalUp + actualPercent.x * targetChunk->AxisA + actualPercent.y * targetChunk->AxisB;
		pointOnUnitCube = glm::normalize(pointOnUnitCube);
		double elevation = 1.0;
		
		double previousResult = 1.0;
		for(auto& stage : _PlanetTerrain->TerrainConstructionStages)
		{
			stage->Process(pointOnUnitCube, previousResult, elevation);
			previousResult = elevation;
		}
		vertices.at(index).m_position = glm::vec3(pointOnUnitCube * _PlanetTerrain->_Info.Radius * elevation);
	}
	std::lock_guard<std::mutex> lock(mutex);
	auto mesh = std::make_unique<Mesh>();
	mesh->SetVertices(1 | static_cast<unsigned>(VertexAttribute::TexCoord), _PlanetTerrain->_SharedVertices, _PlanetTerrain->_SharedTriangles);
	targetChunk->_Mesh = std::move(mesh);
}

void Planet::TerrainChunk::Collapse()
{
	if (!C0 || !C1 || !C2 || !C3) return;
	if (!C0->Active || !C1->Active || !C2->Active || !C3->Active) return;
	C0->Active = false;
	C1->Active = false;
	C2->Active = false;
	C3->Active = false;
	Active = true;
	ChildrenActive = false;
}
