#include "AssetManager.hpp"

#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    Application::Init();
    SerializableFactory::RegisterSerializable<CameraControlSystem>("CameraControlSystem");
    auto ccs = EntityManager::GetOrCreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    RenderManager::GetInstance().m_lightSettings.m_ambientLight = 0.5f;
#pragma region Set main camera to correct position and rotation
    auto mainCameraEntity = RenderManager::GetMainCamera()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, 0, 40));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    mainCameraEntity.SetPrivateComponent<PostProcessing>();
    auto& camera = mainCameraEntity.GetPrivateComponent<Camera>();
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
            Transform transform;
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            transform.SetPosition(position * 5.0f);
            transform.SetScale(glm::vec3(2.0f));
            sphere.SetDataComponent(transform);
            auto &meshRenderer = sphere.SetPrivateComponent<MeshRenderer>();
            meshRenderer.m_mesh = DefaultResources::Primitives::Sphere;
            meshRenderer.m_material = AssetManager::CreateResource<Material>();
            meshRenderer.m_material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer.m_material->m_roughness = static_cast<float>(i) / (amount - 1);
            meshRenderer.m_material->m_metallic = static_cast<float>(j) / (amount - 1);

            sphere.SetParent(collection);
        }
    }
#pragma endregion
#pragma region Load models and display
    auto sponza = AssetManager::LoadModel(true, FileIO::GetResourcePath("Models/Sponza_FBX/Sponza.fbx"));
    auto sponzaEntity = AssetManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), sponza);
    Transform sponzaTransform;
    sponzaTransform.SetValue(glm::vec3(0, -14, -60), glm::radians(glm::vec3(0, -90, 0)), glm::vec3(0.1));
    sponzaEntity.SetDataComponent(sponzaTransform);

    auto title = AssetManager::LoadModel(true, FileIO::GetResourcePath("Models/UniEngine.obj"));
    auto titleEntity = AssetManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), title);
    titleEntity.SetName("Title");
    Transform titleTransform;
    titleTransform.SetValue(glm::vec3(3.5, 70, -160), glm::radians(glm::vec3(0, 0, 0)), glm::vec3(0.05));
    titleEntity.SetDataComponent(titleTransform);
    auto& titleMaterial = titleEntity.GetChildren()[0].GetPrivateComponent<MeshRenderer>().m_material;
    titleMaterial->m_emission = 4;
    titleMaterial->m_albedoColor = glm::vec3(1, 0.2, 0.5);
#ifdef USE_ASSIMP
    auto dancingStormTrooper =
        AssetManager::LoadModel(true, FileIO::GetResourcePath("Models/dancing-stormtrooper/silly_dancing.fbx"));
    auto dancingStormTrooperEntity =
        AssetManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), dancingStormTrooper);
    dancingStormTrooperEntity.SetName("StormTrooper");
    Transform dancingStormTrooperTransform;
    dancingStormTrooperTransform.SetValue(glm::vec3(12, -14, 0), glm::vec3(0), glm::vec3(4));
    dancingStormTrooperEntity.SetDataComponent(dancingStormTrooperTransform);
    auto capoeira = AssetManager::LoadModel(true, FileIO::GetResourcePath("Models/Capoeira.fbx"));
    auto capoeiraEntity = AssetManager::ToEntity(EntityManager::GetDefaultEntityArchetype(), capoeira);
    capoeiraEntity.SetName("Capoeira");
    Transform capoeiraTransform;
    capoeiraTransform.SetValue(glm::vec3(5, 27, -180), glm::vec3(0), glm::vec3(0.2));
    capoeiraEntity.SetDataComponent(capoeiraTransform);
    auto& capoeiraBodyMaterial = capoeiraEntity.GetChildren()[1].GetChildren()[0].GetPrivateComponent<SkinnedMeshRenderer>().m_material;
    capoeiraBodyMaterial->m_albedoColor = glm::vec3(0, 1, 1);
    capoeiraBodyMaterial->m_metallic = 1;
    capoeiraBodyMaterial->m_roughness = 0;
    auto& capoeiraJointsMaterial = capoeiraEntity.GetChildren()[0].GetChildren()[0].GetPrivateComponent<SkinnedMeshRenderer>().m_material;
    capoeiraJointsMaterial->m_albedoColor = glm::vec3(0.3, 1.0, 0.5);
    capoeiraJointsMaterial->m_metallic = 1;
    capoeiraJointsMaterial->m_roughness = 0;
    capoeiraJointsMaterial->m_emission = 6;
#endif
#pragma endregion

#pragma region Create ground
    auto ground = EntityManager::CreateEntity("Ground");
    auto& groundMeshRenderer = ground.SetPrivateComponent<MeshRenderer>();
    groundMeshRenderer.m_material = DefaultResources::Materials::StandardMaterial;
    groundMeshRenderer.m_mesh = DefaultResources::Primitives::Cube;
    Transform groundTransform;
    groundTransform.SetValue(glm::vec3(0, -16, -90), glm::vec3(0), glm::vec3(160, 1, 220));
    ground.SetDataComponent(groundTransform);
#pragma endregion

#pragma region Lighting

    auto dirLightEntity = EntityManager::CreateEntity("Directional Light");
    auto &dirLight = dirLightEntity.SetPrivateComponent<DirectionalLight>();
    dirLight.m_diffuseBrightness = 3.0f;
    dirLight.m_lightSize = 0.2f;

    auto pointLightLeftEntity = EntityManager::CreateEntity("Right Point Light");
    auto &pointLightLeftRenderer = pointLightLeftEntity.SetPrivateComponent<MeshRenderer>();
    pointLightLeftRenderer.m_material =
        AssetManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    pointLightLeftRenderer.m_material->m_albedoColor = glm::vec3(0.0, 0.5, 1.0);
    pointLightLeftRenderer.m_material->m_emission = 10.0f;
    pointLightLeftRenderer.m_mesh = DefaultResources::Primitives::Sphere;
    auto &pointLightLeft = pointLightLeftEntity.SetPrivateComponent<PointLight>();
    pointLightLeft.m_diffuseBrightness = 20;
    pointLightLeft.m_lightSize = 0.2f;
    pointLightLeft.m_linear = 0.02;
    pointLightLeft.m_quadratic = 0.0001;
    pointLightLeft.m_diffuse = glm::vec3(0.0, 0.5, 1.0);

    auto pointLightRightEntity = EntityManager::CreateEntity("Left Point Light");
    auto &pointLightRightRenderer = pointLightRightEntity.SetPrivateComponent<MeshRenderer>();
    pointLightRightRenderer.m_material =
        AssetManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    pointLightRightRenderer.m_material->m_albedoColor = glm::vec3(1.0, 0.8, 0.0);
    pointLightRightRenderer.m_material->m_emission = 10.0f;
    pointLightRightRenderer.m_mesh = DefaultResources::Primitives::Sphere;
    auto &pointLightRight = pointLightRightEntity.SetPrivateComponent<PointLight>();
    pointLightRight.m_diffuseBrightness = 20;
    pointLightRight.m_lightSize = 0.2f;
    pointLightRight.m_linear = 0.02;
    pointLightRight.m_quadratic = 0.0001;
    pointLightRight.m_diffuse = glm::vec3(1.0, 0.8, 0.0);

    auto spotLightConeEntity = EntityManager::CreateEntity("Top Spot Light");
    Transform spotLightConeTransform;
    spotLightConeTransform.SetPosition(glm::vec3(12, 14, 0));
    spotLightConeEntity.SetDataComponent(spotLightConeTransform);

    auto &spotLightRenderer = spotLightConeEntity.SetPrivateComponent<MeshRenderer>();
    spotLightRenderer.m_material = AssetManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    spotLightRenderer.m_material->m_albedoColor = glm::vec3(1, 0.7, 0.7);
    spotLightRenderer.m_material->m_emission = 10.0f;
    spotLightRenderer.m_mesh = DefaultResources::Primitives::Cone;

    auto spotLightEntity = EntityManager::CreateEntity("Spot Light");
    Transform spotLightTransform;
    spotLightTransform.SetEulerRotation(glm::radians(glm::vec3(-90, 0, 0)));
    spotLightEntity.SetDataComponent(spotLightTransform);
    spotLightEntity.SetParent(spotLightConeEntity);
    auto &spotLight = spotLightEntity.SetPrivateComponent<SpotLight>();
    spotLight.m_diffuse = glm::vec3(1, 0.7, 0.7);
    spotLight.m_diffuseBrightness = 40;
#pragma endregion

    double time = 0;
    const float sinTime = glm::sin(time / 5.0f);
    const float cosTime = glm::cos(time / 5.0f);

    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100.0f, time * 10, 0.0f)));
    dirLightEntity.SetDataComponent(dirLightTransform);

    Transform pointLightLeftTransform;
    pointLightLeftTransform.SetPosition(glm::vec3(-40, 12, sinTime * 50 - 50));
    pointLightLeftEntity.SetDataComponent(pointLightLeftTransform);

    Transform pointLightRightTransform;
    pointLightRightTransform.SetPosition(glm::vec3(40, 12, cosTime * 50 - 50));
    pointLightRightEntity.SetDataComponent(pointLightRightTransform);

    Application::RegisterUpdateFunction([&]() {
        if(!Application::IsPlaying()) return;
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

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}
