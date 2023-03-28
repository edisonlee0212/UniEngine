// Planet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "ProjectManager.hpp"
#include "Application.hpp"
#include "ClassRegistry.hpp"
#include "MeshRenderer.hpp"
#include "Planet/PlanetTerrainSystem.hpp"
#include "PlayerController.hpp"
#include "PostProcessing.hpp"
#include "Engine/Core/Serialization.hpp"
#include "DefaultResources.hpp"
#include "Graphics.hpp"
using namespace UniEngine;
using namespace Planet;
void LoadScene();
int main()
{
    ClassRegistry::RegisterPrivateComponent<PlanetTerrain>("PlanetTerrain");
    ClassRegistry::RegisterSystem<PlanetTerrainSystem>("PlanetTerrainSystem");

    const std::filesystem::path resourceFolderPath("../../../Resources");

    for (const auto i : std::filesystem::recursive_directory_iterator(resourceFolderPath))
    {
        if (i.is_directory()) continue;
        if (i.path().extension().string() == ".uescene" || i.path().extension().string() == ".umeta" || i.path().extension().string() == ".ueproj" || i.path().extension().string() == ".ufmeta")
        {
            std::filesystem::remove(i.path());
        }
    }

    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Planet/Planet.ueproj";

    ProjectManager::SetScenePostLoadActions([](){
        LoadScene();
    });

    Application::Create(applicationConfigs);
    Application::Start();
    Application::End();

#pragma endregion
    return 0;
}
void LoadScene(){
    auto scene = Application::GetActiveScene();
#pragma region Preparations
    const auto mainCamera = scene->m_mainCamera.Get<Camera>();
    auto mainCameraEntity = mainCamera->GetOwner();
    auto mainCameraTransform = scene->GetDataComponent<Transform>(mainCameraEntity);
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    scene->SetDataComponent(mainCameraEntity, mainCameraTransform);
    scene->GetOrSetPrivateComponent<PlayerController>(mainCameraEntity);
    auto postProcessing = scene->GetOrSetPrivateComponent<PostProcessing>(mainCameraEntity).lock();

    auto surfaceMaterial = ProjectManager::CreateTemporaryAsset<Material>();
    surfaceMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    auto borderTexture = std::dynamic_pointer_cast<Texture2D>(ProjectManager::GetOrCreateAsset("Textures/border.png"));
    surfaceMaterial->m_albedoTexture = borderTexture;

    auto pts = scene->GetOrCreateSystem<PlanetTerrainSystem>(SystemGroup::SimulationSystemGroup);

    pts->Enable();

    PlanetInfo pi;
    Transform planetTransform;

    planetTransform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    planetTransform.SetEulerRotation(glm::vec3(0.0f));
    pi.m_maxLodLevel = 8;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 10.0;
    pi.m_index = 0;
    pi.m_resolution = 8;

    // Serialization not implemented.
    // planetTerrain1->TerrainConstructionStages.push_back(std::make_shared<PerlinNoiseStage>());
    auto planet1 = scene->CreateEntity("Planet 1");
    auto planetTerrain1 = scene->GetOrSetPrivateComponent<PlanetTerrain>(planet1).lock();
    planetTerrain1->m_surfaceMaterial = surfaceMaterial;
    planetTerrain1->SetPlanetInfo(pi);
    scene->SetDataComponent(planet1, planetTransform);
    planetTransform.SetPosition(glm::vec3(35.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 20;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 15.0;
    pi.m_index = 1;

    auto planet2 = scene->CreateEntity("Planet 2");
    auto planetTerrain2 = scene->GetOrSetPrivateComponent<PlanetTerrain>(planet2).lock();
    planetTerrain2->m_surfaceMaterial = surfaceMaterial;
    planetTerrain2->SetPlanetInfo(pi);
    scene->SetDataComponent(planet2, planetTransform);
    planetTransform.SetPosition(glm::vec3(-20.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 4;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 5.0;
    pi.m_index = 2;

    auto planet3 = scene->CreateEntity("Planet 3");
    auto planetTerrain3 = scene->GetOrSetPrivateComponent<PlanetTerrain>(planet3).lock();
    planetTerrain3->m_surfaceMaterial = surfaceMaterial;
    planetTerrain3->SetPlanetInfo(pi);
    scene->SetDataComponent(planet3, planetTransform);
#pragma endregion

#pragma region Lights
    auto sharedMat = ProjectManager::CreateTemporaryAsset<Material>();
    surfaceMaterial->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    Transform ltw;

    Entity dle = scene->CreateEntity("Directional Light");
    auto dlc = scene->GetOrSetPrivateComponent<DirectionalLight>(dle).lock();
    dlc->m_diffuse = glm::vec3(1.0f);
    ltw.SetScale(glm::vec3(0.5f));

    Entity ple = scene->CreateEntity("Point Light 1");
    auto plmmc = scene->GetOrSetPrivateComponent<MeshRenderer>(ple).lock();
    plmmc->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    plmmc->m_material.Set<Material>(sharedMat);
    auto plc = scene->GetOrSetPrivateComponent<PointLight>(ple).lock();
    plc->m_constant = 1.0f;
    plc->m_linear = 0.09f;
    plc->m_quadratic = 0.032f;
    plc->m_diffuse = glm::vec3(1.0f);
    plc->m_diffuseBrightness = 5;

    scene->SetDataComponent(ple, ltw);

    Entity ple2 = scene->CreateEntity("Point Light 2");
    auto plc2 = scene->GetOrSetPrivateComponent<PointLight>(ple2).lock();
    plc2->m_constant = 1.0f;
    plc2->m_linear = 0.09f;
    plc2->m_quadratic = 0.032f;
    plc2->m_diffuse = glm::vec3(1.0f);
    plc2->m_diffuseBrightness = 5;

    scene->SetDataComponent(ple2, ltw);
    scene->SetEntityName(ple2, "Point Light 2");
    auto plmmc2 = scene->GetOrSetPrivateComponent<MeshRenderer>(ple2).lock();
    plmmc2->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    plmmc2->m_material.Set<Material>(sharedMat);

#pragma endregion
    /*
    #pragma region EngineLoop
            Application::RegisterPreUpdateFunction([=]() {
                Transform ltw;
                ltw.SetScale(glm::vec3(0.5f));
    #pragma region LightsPosition
                ltw.SetPosition(glm::vec4(
                    glm::vec3(
                        0.0f,
                        20.0f * glm::sin(Application::Time().CurrentTime() / 2.0f),
                        -20.0f * glm::cos(Application::Time().CurrentTime() / 2.0f)),
                    0.0f));
                dle.SetDataComponent(ltw);
                ltw.SetPosition(glm::vec4(
                    glm::vec3(
                        -20.0f * glm::cos(Application::Time().CurrentTime() / 2.0f),
                        20.0f * glm::sin(Application::Time().CurrentTime() / 2.0f),
                        0.0f),
                    0.0f));
                ple.SetDataComponent(ltw);
                ltw.SetPosition(glm::vec4(
                    glm::vec3(
                        20.0f * glm::cos(Application::Time().CurrentTime() / 2.0f),
                        15.0f,
                        20.0f * glm::sin(Application::Time().CurrentTime() / 2.0f)),
                    0.0f));
                ple2.SetDataComponent(ltw);
    #pragma endregion
     */
}