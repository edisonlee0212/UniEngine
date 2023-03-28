#include "Graphics.hpp"
#include "Prefab.hpp"
#include "ProjectManager.hpp"
#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;
void LoadScene();
int main()
{
    /*
     * Please change this to the root folder.
     */
    const std::filesystem::path resourceFolderPath("../../../Resources");

    for (const auto i : std::filesystem::recursive_directory_iterator(resourceFolderPath))
    {
        if (i.is_directory()) continue;
        if (i.path().extension().string() == ".uescene" || i.path().extension().string() == ".umeta" || i.path().extension().string() == ".ueproj" || i.path().extension().string() == ".ufmeta")
        {
            std::filesystem::remove(i.path());
        }
    }


    ProjectManager::SetScenePostLoadActions([]() { LoadScene(); });
    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Rendering/Rendering.ueproj";
    Application::Create(applicationConfigs);

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Start();
    Application::End();

#pragma endregion
    return 0;
}
void LoadScene()
{
    auto scene = Application::GetActiveScene();
    double time = 0;
    const float sinTime = glm::sin(time / 5.0f);
    const float cosTime = glm::cos(time / 5.0f);
    scene->m_environmentSettings.m_ambientLightIntensity = 0.5f;
#pragma region Set main camera to correct position and rotation
    const auto mainCamera = scene->m_mainCamera.Get<Camera>();
    auto mainCameraEntity = mainCamera->GetOwner();
    auto mainCameraTransform = scene->GetDataComponent<Transform>(mainCameraEntity);
    mainCameraTransform.SetPosition(glm::vec3(0, 0, 40));
    scene->SetDataComponent(mainCameraEntity, mainCameraTransform);
    scene->GetOrSetPrivateComponent<PostProcessing>(mainCameraEntity);
    auto camera = scene->GetOrSetPrivateComponent<Camera>(mainCameraEntity).lock();
    scene->GetOrSetPrivateComponent<PlayerController>(mainCameraEntity);
#pragma endregion

#pragma region Create 9 spheres in different PBR properties
    int amount = 4;
    auto collection = scene->CreateEntity("Spheres");
    auto spheres = scene->CreateEntities(amount * amount, "Instance");
    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            Transform transform;
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            transform.SetPosition(position * 5.0f);
            transform.SetScale(glm::vec3(5.0f));
            scene->SetDataComponent(sphere, transform);
            auto meshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(sphere).lock();
            meshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
            auto material = ProjectManager::CreateTemporaryAsset<Material>();
            meshRenderer->m_material.Set<Material>(material);
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            material->m_materialProperties.m_roughness = static_cast<float>(i) / (amount - 1);
            material->m_materialProperties.m_metallic = static_cast<float>(j) / (amount - 1);

            scene->SetParent(sphere, collection);
        }
    }
#pragma endregion
#pragma region Load models and display

    auto sponza = std::dynamic_pointer_cast<Prefab>(ProjectManager::GetOrCreateAsset("Models/Sponza_FBX/Sponza.fbx"));
    auto sponzaEntity = sponza->ToEntity(scene);
    Transform sponzaTransform;
    sponzaTransform.SetValue(glm::vec3(0, -14, -60), glm::radians(glm::vec3(0, -90, 0)), glm::vec3(0.1));
    scene->SetDataComponent(sponzaEntity, sponzaTransform);

    auto title = std::dynamic_pointer_cast<Prefab>(ProjectManager::GetOrCreateAsset("Models/UniEngine.obj"));
    auto titleEntity = title->ToEntity(scene);
    scene->SetEntityName(titleEntity, "Title");
    Transform titleTransform;
    titleTransform.SetValue(glm::vec3(3.5, 70, -160), glm::radians(glm::vec3(0, 0, 0)), glm::vec3(0.05));
    scene->SetDataComponent(titleEntity, titleTransform);
    
    auto titleMaterial =
        scene->GetOrSetPrivateComponent<MeshRenderer>(scene->GetChildren(scene->GetChildren(titleEntity)[0])[0])
            .lock()
            ->m_material.Get<Material>();
    titleMaterial->m_materialProperties.m_emission = 4;
    titleMaterial->m_materialProperties.m_albedoColor = glm::vec3(1, 0.2, 0.5);
    
#ifdef USE_ASSIMP

    auto dancingStormTrooper = std::dynamic_pointer_cast<Prefab>(
        ProjectManager::GetOrCreateAsset("Models/dancing-stormtrooper/silly_dancing.fbx"));
    auto dancingStormTrooperEntity = dancingStormTrooper->ToEntity(scene);
    scene->SetEntityName(dancingStormTrooperEntity, "StormTrooper");
    Transform dancingStormTrooperTransform;
    dancingStormTrooperTransform.SetValue(glm::vec3(12, -14, 0), glm::vec3(0), glm::vec3(4));
    scene->SetDataComponent(dancingStormTrooperEntity, dancingStormTrooperTransform);

    auto capoeira = std::dynamic_pointer_cast<Prefab>(ProjectManager::GetOrCreateAsset("Models/Capoeira.fbx"));
    auto capoeiraEntity = capoeira->ToEntity(scene);
    scene->SetEntityName(capoeiraEntity, "Capoeira");
    Transform capoeiraTransform;
    capoeiraTransform.SetValue(glm::vec3(5, 27, -180), glm::vec3(0), glm::vec3(0.2));
    scene->SetDataComponent(capoeiraEntity, capoeiraTransform);
    auto capoeiraBodyMaterial = scene
                                    ->GetOrSetPrivateComponent<SkinnedMeshRenderer>(
                                        scene->GetChildren(scene->GetChildren(capoeiraEntity)[1])[0])
                                    .lock()
                                    ->m_material.Get<Material>();
    capoeiraBodyMaterial->m_materialProperties.m_albedoColor = glm::vec3(0, 1, 1);
    capoeiraBodyMaterial->m_materialProperties.m_metallic = 1;
    capoeiraBodyMaterial->m_materialProperties.m_roughness = 0;
    auto capoeiraJointsMaterial = scene
                                      ->GetOrSetPrivateComponent<SkinnedMeshRenderer>(
                                          scene->GetChildren(scene->GetChildren(capoeiraEntity)[0])[0])
                                      .lock()
                                      ->m_material.Get<Material>();
    capoeiraJointsMaterial->m_materialProperties.m_albedoColor = glm::vec3(0.3, 1.0, 0.5);
    capoeiraJointsMaterial->m_materialProperties.m_metallic = 1;
    capoeiraJointsMaterial->m_materialProperties.m_roughness = 0;
    capoeiraJointsMaterial->m_materialProperties.m_emission = 6;

#endif
#pragma endregion

#pragma region Create ground
    auto ground = scene->CreateEntity("Ground");
    auto groundMeshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(ground).lock();
    auto groundMat = ProjectManager::CreateTemporaryAsset<Material>();
    groundMat->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer->m_material.Set<Material>(groundMat);
    groundMeshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Cube);
    Transform groundTransform;
    groundTransform.SetValue(glm::vec3(0, -16, -90), glm::vec3(0), glm::vec3(160, 1, 220));
    scene->SetDataComponent(ground, groundTransform);
#pragma endregion

#pragma region Lighting
    auto dirLightEntity = scene->CreateEntity("Directional Light");
    auto dirLight = scene->GetOrSetPrivateComponent<DirectionalLight>(dirLightEntity).lock();
    dirLight->m_diffuseBrightness = 3.0f;
    dirLight->m_lightSize = 0.2f;

    auto pointLightLeftEntity = scene->CreateEntity("Right Point Light");
    auto pointLightLeftRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(pointLightLeftEntity).lock();
    auto groundMaterial = ProjectManager::CreateTemporaryAsset<Material>();
    groundMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    pointLightLeftRenderer->m_material.Set<Material>(groundMaterial);
    groundMaterial->m_materialProperties.m_albedoColor = glm::vec3(0.0, 0.5, 1.0);
    groundMaterial->m_materialProperties.m_emission = 10.0f;
    pointLightLeftRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    auto pointLightLeft = scene->GetOrSetPrivateComponent<PointLight>(pointLightLeftEntity).lock();
    pointLightLeft->m_diffuseBrightness = 20;
    pointLightLeft->m_lightSize = 0.2f;
    pointLightLeft->m_linear = 0.02;
    pointLightLeft->m_quadratic = 0.0001;
    pointLightLeft->m_diffuse = glm::vec3(0.0, 0.5, 1.0);

    auto pointLightRightEntity = scene->CreateEntity("Left Point Light");
    auto pointLightRightRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(pointLightRightEntity).lock();
    auto pointLightRightMaterial = ProjectManager::CreateTemporaryAsset<Material>();
    pointLightRightMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    pointLightRightRenderer->m_material.Set<Material>(pointLightRightMaterial);
    pointLightRightMaterial->m_materialProperties.m_albedoColor = glm::vec3(1.0, 0.8, 0.0);
    pointLightRightMaterial->m_materialProperties.m_emission = 10.0f;
    pointLightRightRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    auto pointLightRight = scene->GetOrSetPrivateComponent<PointLight>(pointLightRightEntity).lock();
    pointLightRight->m_diffuseBrightness = 20;
    pointLightRight->m_lightSize = 0.2f;
    pointLightRight->m_linear = 0.02;
    pointLightRight->m_quadratic = 0.0001;
    pointLightRight->m_diffuse = glm::vec3(1.0, 0.8, 0.0);

    auto spotLightConeEntity = scene->CreateEntity("Top Spot Light");
    Transform spotLightConeTransform;
    spotLightConeTransform.SetPosition(glm::vec3(12, 14, 0));
    scene->SetDataComponent(spotLightConeEntity, spotLightConeTransform);

    auto spotLightRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(spotLightConeEntity).lock();
    spotLightRenderer->m_castShadow = false;
    auto spotLightMaterial = ProjectManager::CreateTemporaryAsset<Material>();
    spotLightMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    spotLightRenderer->m_material.Set<Material>(spotLightMaterial);
    spotLightMaterial->m_materialProperties.m_albedoColor = glm::vec3(1, 0.7, 0.7);
    spotLightMaterial->m_materialProperties.m_emission = 10.0f;
    spotLightRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Cone);

    auto spotLightEntity = scene->CreateEntity("Spot Light");
    Transform spotLightTransform;
    spotLightTransform.SetEulerRotation(glm::radians(glm::vec3(-90, 0, 0)));
    scene->SetDataComponent(spotLightEntity, spotLightTransform);
    scene->SetParent(spotLightEntity, spotLightConeEntity);
    auto spotLight = scene->GetOrSetPrivateComponent<SpotLight>(spotLightEntity).lock();
    spotLight->m_diffuse = glm::vec3(1, 0.7, 0.7);
    spotLight->m_diffuseBrightness = 40;
#pragma endregion
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100.0f, time * 10, 0.0f)));
    scene->SetDataComponent(dirLightEntity, dirLightTransform);

    Transform pointLightLeftTransform;
    pointLightLeftTransform.SetPosition(glm::vec3(-40, 12, sinTime * 50 - 50));
    scene->SetDataComponent(pointLightLeftEntity, pointLightLeftTransform);

    Transform pointLightRightTransform;
    pointLightRightTransform.SetPosition(glm::vec3(40, 12, cosTime * 50 - 50));
    scene->SetDataComponent(pointLightRightEntity, pointLightRightTransform);
    /*
    Application::RegisterUpdateFunction([=, &time]() {
        if (!Application::IsPlaying())
            return;
        Transform dirLightTransform;
        dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100.0f, time * 10, 0.0f)));
        dirLightEntity.SetDataComponent(dirLightTransform);

        Transform pointLightLeftTransform;
        pointLightLeftTransform.SetPosition(glm::vec3(-40, 12, sinTime * 50 - 50));

        Transform pointLightRightTransform;
        pointLightRightTransform.SetPosition(glm::vec3(40, 12, cosTime * 50 - 50));
        const float currentTime = Application::Time().CurrentTime();
        time += Application::Time().DeltaTime();
        const float sinTime = glm::sin(time / 5.0f);
        const float cosTime = glm::cos(time / 5.0f);
        dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100.0f, time * 10, 0.0f)));
        dirLightEntity.SetDataComponent(dirLightTransform);
        pointLightLeftTransform.SetPosition(glm::vec3(-40, 12, sinTime * 50 - 50));
        pointLightRightTransform.SetPosition(glm::vec3(40, 12, cosTime * 50 - 50));
        pointLightLeftEntity.SetDataComponent(pointLightLeftTransform);
        pointLightRightEntity.SetDataComponent(pointLightRightTransform);
    });
     */
}