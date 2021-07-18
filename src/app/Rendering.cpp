#include "ResourceManager.hpp"

#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    Application::Init();
    auto &world = EntityManager::GetCurrentWorld();
    CameraControlSystem *ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    ccs->Enable();
    RenderManager::GetInstance().m_lightSettings.m_ambientLight = 0.5f;
#pragma region Set main camera to correct position and rotation
    auto mainCameraEntity = RenderManager::GetMainCamera()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, 0, 40));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    mainCameraEntity.SetPrivateComponent<PostProcessing>();
#pragma endregion

#pragma region Create 9 spheres in different PBR properties
    int amount = 4;
    auto collection = EntityManager::CreateEntity("Spheres");
    auto spheres = EntityManager::CreateEntities(amount * amount, "Instance");
    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            auto transform = sphere.GetDataComponent<Transform>();
            auto globalTransform = sphere.GetDataComponent<GlobalTransform>();
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            transform.SetPosition(position * 5.0f);
            globalTransform.SetPosition(position * 5.0f);
            transform.SetScale(glm::vec3(2.0f));
            globalTransform.SetScale(glm::vec3(2.0f));
            sphere.SetDataComponent(transform);
            sphere.SetDataComponent(globalTransform);
            auto &meshRenderer = sphere.SetPrivateComponent<MeshRenderer>();
            meshRenderer.m_mesh = DefaultResources::Primitives::Sphere;
            meshRenderer.m_material = ResourceManager::CreateResource<Material>();
            meshRenderer.m_material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer.m_material->m_roughness = static_cast<float>(i) / (amount - 1);
            meshRenderer.m_material->m_metallic = static_cast<float>(j) / (amount - 1);

            sphere.SetParent(collection);
        }
    }
#pragma endregion
#pragma region Load models and display
    auto sponza = ResourceManager::LoadModel(true, FileIO::GetResourcePath("Models/Sponza_FBX/Sponza.fbx"));
    auto sponzaEntity = ResourceManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), sponza);
    Transform sponzaTransform;
    sponzaTransform.SetValue(glm::vec3(0, -14, -60), glm::radians(glm::vec3(0, -90, 0)), glm::vec3(0.1));
    sponzaEntity.SetDataComponent(sponzaTransform);

#ifdef USE_ASSIMP

    auto dancingStormTrooper =
        ResourceManager::LoadModel(true, FileIO::GetResourcePath("Models/dancing-stormtrooper/silly_dancing.fbx"));
    auto dancingStormTrooperEntity =
        ResourceManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), dancingStormTrooper);
    Transform dancingStormTrooperTransform;
    dancingStormTrooperTransform.SetValue(glm::vec3(12, -14, 0), glm::vec3(0), glm::vec3(4));
    dancingStormTrooperEntity.SetDataComponent(dancingStormTrooperTransform);
    auto capoeira = ResourceManager::LoadModel(true, FileIO::GetResourcePath("Models/Capoeira.fbx"));
    auto capoeiraEntity = ResourceManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), capoeira);
    Transform capoeiraTransform;
    capoeiraTransform.SetValue(glm::vec3(0, -14, -15), glm::vec3(0), glm::vec3(0.1));
    capoeiraEntity.SetDataComponent(capoeiraTransform);
#endif
#pragma endregion
    /*
#pragma region Create ground
    auto ground = EntityManager::CreateEntity("Ground");
    auto groundMeshRenderer = std::make_unique<MeshRenderer>();
    groundMeshRenderer->m_value = DefaultResources::Materials::StandardMaterial;
    groundMeshRenderer->m_mesh = DefaultResources::Primitives::Cube;
    Transform groundTransform;
    groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    ground.SetDataComponent(groundTransform);
    ground.SetPrivateComponent(std::move(groundMeshRenderer));
#pragma endregion
    */
#pragma region Lighting

    auto dirLightEntity = EntityManager::CreateEntity("Dir Light");
    auto &dirLight = dirLightEntity.SetPrivateComponent<DirectionalLight>();
    dirLight.m_diffuseBrightness = 3.0f;
    dirLight.m_lightSize = 0.2f;
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100, 0, 0)));
    dirLightEntity.SetDataComponent(dirLightTransform);

    auto pointLightLeftEntity = EntityManager::CreateEntity("Right Point Light");
    auto &pointLightLeftRenderer = pointLightLeftEntity.SetPrivateComponent<MeshRenderer>();
    pointLightLeftRenderer.m_material =
        ResourceManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    pointLightLeftRenderer.m_material->m_albedoColor = glm::vec3(0.0, 0.5, 1.0);
    pointLightLeftRenderer.m_material->m_emission = 10.0f;
    pointLightLeftRenderer.m_mesh = DefaultResources::Primitives::Sphere;
    auto &pointLightLeft = pointLightLeftEntity.SetPrivateComponent<PointLight>();
    pointLightLeft.m_diffuseBrightness = 20;
    pointLightLeft.m_lightSize = 0.2f;
    pointLightLeft.m_diffuse = glm::vec3(0.0, 0.5, 1.0);
    Transform pointLightLeftTransform;
    pointLightLeftTransform.SetPosition(glm::vec3(glm::vec3(-40, 12, -50)));
    pointLightLeftEntity.SetDataComponent(pointLightLeftTransform);

    auto pointLightRightEntity = EntityManager::CreateEntity("Left Point Light");
    auto &pointLightRightRenderer = pointLightRightEntity.SetPrivateComponent<MeshRenderer>();
    pointLightRightRenderer.m_material =
        ResourceManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    pointLightRightRenderer.m_material->m_albedoColor = glm::vec3(1.0, 0.8, 0.0);
    pointLightRightRenderer.m_material->m_emission = 10.0f;
    pointLightRightRenderer.m_mesh = DefaultResources::Primitives::Sphere;
    auto &pointLightRight = pointLightRightEntity.SetPrivateComponent<PointLight>();
    pointLightRight.m_diffuseBrightness = 20;
    pointLightRight.m_lightSize = 0.2f;
    pointLightRight.m_diffuse = glm::vec3(1.0, 0.8, 0.0);
    Transform pointLightRightTransform;
    pointLightRightTransform.SetPosition(glm::vec3(glm::vec3(40, 12, -50)));
    pointLightRightEntity.SetDataComponent(pointLightRightTransform);

#pragma endregion
    Application::RegisterUpdateFunction([&]() {
        const float currentTime = Application::Time().CurrentTime();
        const float sinTime = glm::sin(currentTime / 5.0f);
        const float cosTime = glm::cos(currentTime / 5.0f);
        dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100.0f, currentTime * 10, 0.0f)));
        dirLightEntity.SetDataComponent(dirLightTransform);
        pointLightLeftTransform.SetPosition(glm::vec3(-40, 12, sinTime * 50 - 50));
        pointLightRightTransform.SetPosition(glm::vec3(40, 12, cosTime * 50 - 50));
        pointLightLeftEntity.SetDataComponent(pointLightLeftTransform);
        pointLightRightEntity.SetDataComponent(pointLightRightTransform);
    });

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}
