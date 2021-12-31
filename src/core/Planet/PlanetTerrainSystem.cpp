#include "AssetManager.hpp"

#include <Planet/PlanetTerrainSystem.hpp>
#include <glm/gtc/noise.hpp>

void Planet::PlanetTerrainSystem::OnCreate()
{
}

void Planet::PlanetTerrainSystem::Update()
{
    const std::vector<Entity> *const planetTerrainList =
        Entities::UnsafeGetPrivateComponentOwnersList<PlanetTerrain>(Entities::GetCurrentScene());
    if (planetTerrainList == nullptr)
        return;

    std::mutex meshGenLock;
    const auto mainCamera = Entities::GetCurrentScene()->m_mainCamera.Get<Camera>();
    if (mainCamera)
    {
        const auto cameraLtw = mainCamera->GetOwner().GetDataComponent<GlobalTransform>();
        for (auto i = 0; i < planetTerrainList->size(); i++)
        {
            auto planetTerrain = planetTerrainList->at(i).GetOrSetPrivateComponent<PlanetTerrain>().lock();
            if (!planetTerrain->IsEnabled())
                continue;
            auto &planetInfo = planetTerrain->m_info;
            auto planetTransform = planetTerrain->GetOwner().GetDataComponent<GlobalTransform>();
            auto &planetChunks = planetTerrain->m_chunks;
            // 1. Scan and expand.
            for (auto &chunk : planetChunks)
            {
                // futures.push_back(_PrimaryWorkers->Share([&, this](int id) { CheckLod(meshGenLock, chunk, planetInfo,
                // planetTransform, cameraLtw); }).share());
                CheckLod(meshGenLock, chunk, planetInfo, planetTransform, cameraLtw);
            }

            glm::mat4 matrix = glm::scale(
                glm::translate(glm::mat4_cast(planetTransform.GetRotation()), glm::vec3(planetTransform.GetPosition())),
                glm::vec3(1.0f));
            auto material = planetTerrain->m_surfaceMaterial.Get<Material>();
            if (material)
            {
                for (auto j = 0; j < planetChunks.size(); j++)
                {
                    RenderChunk(planetChunks[j], material, matrix, mainCamera, true);
                }
            }
        }
    }
}

void Planet::PlanetTerrainSystem::FixedUpdate()
{
}

void Planet::PlanetTerrainSystem::CheckLod(
    std::mutex &mutex,
    std::shared_ptr<TerrainChunk> &chunk,
    const PlanetInfo &info,
    const GlobalTransform &planetTransform,
    const GlobalTransform &cameraTransform)
{
    if (glm::distance(
            glm::dvec3(chunk->ChunkCenterPosition(
                planetTransform.GetPosition(), info.m_radius, planetTransform.GetRotation())),
            glm::dvec3(cameraTransform.GetPosition())) <
        info.m_lodDistance * info.m_radius / glm::pow(2, chunk->m_detailLevel + 1))
    {
        if (chunk->m_detailLevel < info.m_maxLodLevel)
        {
            chunk->Expand(mutex);
        }
    }
    if (chunk->m_c0)
        CheckLod(mutex, chunk->m_c0, info, planetTransform, cameraTransform);
    if (chunk->m_c1)
        CheckLod(mutex, chunk->m_c1, info, planetTransform, cameraTransform);
    if (chunk->m_c2)
        CheckLod(mutex, chunk->m_c2, info, planetTransform, cameraTransform);
    if (chunk->m_c3)
        CheckLod(mutex, chunk->m_c3, info, planetTransform, cameraTransform);
    if (glm::distance(
            glm::dvec3(chunk->ChunkCenterPosition(
                planetTransform.GetPosition(), info.m_radius, planetTransform.GetRotation())),
            glm::dvec3(cameraTransform.GetPosition())) >
        info.m_lodDistance * info.m_radius / glm::pow(2, chunk->m_detailLevel + 1))
    {
        chunk->Collapse();
    }
}

void Planet::PlanetTerrainSystem::RenderChunk(
    std::shared_ptr<TerrainChunk> &chunk,
    const std::shared_ptr<Material> &material,
    glm::mat4 &matrix,
    const std::shared_ptr<Camera> &camera,
    bool receiveShadow) const
{
    if (chunk->Active)
        Graphics::DrawMesh(chunk->m_mesh, material, matrix, camera, true);
    if (chunk->ChildrenActive)
    {
        RenderChunk(chunk->m_c0, material, matrix, camera, receiveShadow);
        RenderChunk(chunk->m_c1, material, matrix, camera, receiveShadow);
        RenderChunk(chunk->m_c2, material, matrix, camera, receiveShadow);
        RenderChunk(chunk->m_c3, material, matrix, camera, receiveShadow);
    }
}
