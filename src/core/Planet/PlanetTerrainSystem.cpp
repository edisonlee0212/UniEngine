#include <Planet/PlanetTerrainSystem.hpp>
#include <glm/gtc/noise.hpp>

std::shared_ptr<Material> Planet::PlanetTerrainSystem::_DefaultSurfaceMaterial;

void Planet::PlanetTerrainSystem::OnCreate()
{
	_DefaultSurfaceMaterial = std::make_shared<Material>();
	_DefaultSurfaceMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
	_DefaultSurfaceMaterial->SetTexture(DefaultResources::Textures::Border);
}

void Planet::PlanetTerrainSystem::Update()
{
	const std::vector<Entity>* const planetTerrainList = EntityManager::GetPrivateComponentOwnersList<PlanetTerrain>();
	for (auto i = 0; i < planetTerrainList->size(); i++) {
		auto& planetTerrain = planetTerrainList->at(i).GetPrivateComponent<PlanetTerrain>();
		if (!planetTerrain->IsEnabled()) continue;
		auto& planetChunks = planetTerrain->_Chunks;
		auto planetTransform = planetTerrain->GetOwner().GetComponentData<GlobalTransform>();
		glm::mat4 matrix = glm::scale(glm::translate(glm::mat4_cast(planetTransform.GetRotation()), glm::vec3(planetTransform.GetPosition())), glm::vec3(1.0f));
		for (auto j = 0; j < planetChunks.size(); j++) {
			RenderChunk(planetChunks[j], _DefaultSurfaceMaterial.get(), matrix, RenderManager::GetMainCamera(), true);
		}
	}
	std::mutex meshGenLock;
	const auto mainCamera = RenderManager::GetMainCamera();
	if (mainCamera) {
		const auto cameraLtw = mainCamera->GetOwner().GetComponentData<GlobalTransform>();
		for (auto i = 0; i < planetTerrainList->size(); i++) {
			auto& planetTerrain = planetTerrainList->at(i).GetPrivateComponent<PlanetTerrain>();
			if (!planetTerrain->IsEnabled()) continue;
			auto& planetInfo = planetTerrain->_Info;
			auto planetTransform = planetTerrain->GetOwner().GetComponentData<GlobalTransform>();
			//1. Scan and expand.
			for (auto& chunk : planetTerrain->_Chunks) {
				//futures.push_back(_PrimaryWorkers->Push([&, this](int id) { CheckLod(meshGenLock, chunk, planetInfo, planetTransform, cameraLtw); }).share());
				CheckLod(meshGenLock, chunk, planetInfo, planetTransform, cameraLtw);
			}
		}
	}
}

void Planet::PlanetTerrainSystem::FixedUpdate()
{
}

std::shared_ptr<Material> Planet::PlanetTerrainSystem::GetDefaultSurfaceMaterial()
{
	return _DefaultSurfaceMaterial;
}

void Planet::PlanetTerrainSystem::CheckLod(std::mutex& mutex, std::unique_ptr<TerrainChunk>& chunk, const PlanetInfo& info, const GlobalTransform& planetTransform, const GlobalTransform& cameraTransform)
{
	if (glm::distance(glm::dvec3(chunk->ChunkCenterPosition(planetTransform.GetPosition(), info.Radius, planetTransform.GetRotation())), glm::dvec3(cameraTransform.GetPosition()))
		< info.LodDistance * info.Radius / glm::pow(2, chunk->DetailLevel + 1)) {
		if (chunk->DetailLevel < info.MaxLodLevel) {
			chunk->Expand(mutex);
		}
	}
	if (chunk->C0)CheckLod(mutex, chunk->C0, info, planetTransform, cameraTransform);
	if (chunk->C1)CheckLod(mutex, chunk->C1, info, planetTransform, cameraTransform);
	if (chunk->C2)CheckLod(mutex, chunk->C2, info, planetTransform, cameraTransform);
	if (chunk->C3)CheckLod(mutex, chunk->C3, info, planetTransform, cameraTransform);
	if (glm::distance(glm::dvec3(chunk->ChunkCenterPosition(planetTransform.GetPosition(), info.Radius, planetTransform.GetRotation())), glm::dvec3(cameraTransform.GetPosition()))
		> info.LodDistance * info.Radius / glm::pow(2, chunk->DetailLevel + 1))
	{
		chunk->Collapse();
	}
}

void Planet::PlanetTerrainSystem::RenderChunk(std::unique_ptr<TerrainChunk>& chunk, Material* material,
	glm::mat4& matrix, CameraComponent* camera, bool receiveShadow)
{
	if (chunk->Active) RenderManager::DrawMesh(chunk->_Mesh.get(), _DefaultSurfaceMaterial.get(), matrix, RenderManager::GetMainCamera(), true);
	if (chunk->ChildrenActive)
	{
		RenderChunk(chunk->C0, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->C1, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->C2, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->C3, material, matrix, camera, receiveShadow);
	}
}
