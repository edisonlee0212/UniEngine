#include <Planet/PlanetTerrainSystem.hpp>
#include <glm/gtc/noise.hpp>

std::shared_ptr<Material> Planet::PlanetTerrainSystem::m_defaultSurfaceMaterial;

void Planet::PlanetTerrainSystem::OnCreate()
{
	m_defaultSurfaceMaterial = std::make_shared<Material>();
	m_defaultSurfaceMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
	m_defaultSurfaceMaterial->SetTexture(DefaultResources::Textures::Border);
}

void Planet::PlanetTerrainSystem::Update()
{
	const std::vector<Entity>* const planetTerrainList = EntityManager::GetPrivateComponentOwnersList<PlanetTerrain>();
	for (auto i = 0; i < planetTerrainList->size(); i++) {
		auto& planetTerrain = planetTerrainList->at(i).GetPrivateComponent<PlanetTerrain>();
		if (!planetTerrain->IsEnabled()) continue;
		auto& planetChunks = planetTerrain->m_chunks;
		auto planetTransform = planetTerrain->GetOwner().GetComponentData<GlobalTransform>();
		glm::mat4 matrix = glm::scale(glm::translate(glm::mat4_cast(planetTransform.GetRotation()), glm::vec3(planetTransform.GetPosition())), glm::vec3(1.0f));
		for (auto j = 0; j < planetChunks.size(); j++) {
			RenderChunk(planetChunks[j], m_defaultSurfaceMaterial.get(), matrix, RenderManager::GetMainCamera(), true);
		}
	}
	std::mutex meshGenLock;
	const auto mainCamera = RenderManager::GetMainCamera();
	if (mainCamera) {
		const auto cameraLtw = mainCamera->GetOwner().GetComponentData<GlobalTransform>();
		for (auto i = 0; i < planetTerrainList->size(); i++) {
			auto& planetTerrain = planetTerrainList->at(i).GetPrivateComponent<PlanetTerrain>();
			if (!planetTerrain->IsEnabled()) continue;
			auto& planetInfo = planetTerrain->m_info;
			auto planetTransform = planetTerrain->GetOwner().GetComponentData<GlobalTransform>();
			//1. Scan and expand.
			for (auto& chunk : planetTerrain->m_chunks) {
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
	return m_defaultSurfaceMaterial;
}

void Planet::PlanetTerrainSystem::CheckLod(std::mutex& mutex, std::unique_ptr<TerrainChunk>& chunk, const PlanetInfo& info, const GlobalTransform& planetTransform, const GlobalTransform& cameraTransform)
{
	if (glm::distance(glm::dvec3(chunk->ChunkCenterPosition(planetTransform.GetPosition(), info.m_radius, planetTransform.GetRotation())), glm::dvec3(cameraTransform.GetPosition()))
		< info.m_lodDistance * info.m_radius / glm::pow(2, chunk->m_detailLevel + 1)) {
		if (chunk->m_detailLevel < info.m_maxLodLevel) {
			chunk->Expand(mutex);
		}
	}
	if (chunk->m_c0)CheckLod(mutex, chunk->m_c0, info, planetTransform, cameraTransform);
	if (chunk->m_c1)CheckLod(mutex, chunk->m_c1, info, planetTransform, cameraTransform);
	if (chunk->m_c2)CheckLod(mutex, chunk->m_c2, info, planetTransform, cameraTransform);
	if (chunk->m_c3)CheckLod(mutex, chunk->m_c3, info, planetTransform, cameraTransform);
	if (glm::distance(glm::dvec3(chunk->ChunkCenterPosition(planetTransform.GetPosition(), info.m_radius, planetTransform.GetRotation())), glm::dvec3(cameraTransform.GetPosition()))
		> info.m_lodDistance * info.m_radius / glm::pow(2, chunk->m_detailLevel + 1))
	{
		chunk->Collapse();
	}
}

void Planet::PlanetTerrainSystem::RenderChunk(std::unique_ptr<TerrainChunk>& chunk, Material* material,
	glm::mat4& matrix, CameraComponent* camera, bool receiveShadow) const
{
	if (chunk->Active) RenderManager::DrawMesh(chunk->m_mesh.get(), m_defaultSurfaceMaterial.get(), matrix, RenderManager::GetMainCamera(), true);
	if (chunk->ChildrenActive)
	{
		RenderChunk(chunk->m_c0, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->m_c1, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->m_c2, material, matrix, camera, receiveShadow);
		RenderChunk(chunk->m_c3, material, matrix, camera, receiveShadow);
	}
}
