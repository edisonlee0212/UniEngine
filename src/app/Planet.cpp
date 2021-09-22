// Planet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "AssetManager.hpp"
#include <Application.hpp>
#include <ClassRegistry.hpp>
#include <MeshRenderer.hpp>
#include <Planet/PlanetTerrainSystem.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;
using namespace Planet;
void LoadScene();
int main()
{
    ClassRegistry::RegisterPrivateComponent<PlanetTerrain>("PlanetTerrain");
    ClassRegistry::RegisterSystem<PlanetTerrainSystem>("PlanetTerrainSystem");

    const std::filesystem::path resourceFolderPath("../Resources");
    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Planet/Planet.ueproj";
    Application::Init(applicationConfigs);
    LoadScene();
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}
void LoadScene(){
#pragma region Preparations
    RenderManager::GetMainCamera().lock()->m_useClearColor = false;
    auto mainCameraEntity = RenderManager::GetMainCamera().lock()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    mainCameraEntity.GetOrSetPrivateComponent<PlayerController>();
    auto postProcessing = mainCameraEntity.GetOrSetPrivateComponent<PostProcessing>().lock();

    auto surfaceMaterial = AssetManager::LoadMaterial(DefaultResources::GLPrograms::StandardProgram);
    auto borderTexture = AssetManager::CreateAsset<Texture2D>("Border");
    borderTexture->SetPathAndLoad("Textures/border.png");
    surfaceMaterial->m_albedoTexture = borderTexture;

    auto pts = EntityManager::GetOrCreateSystem<PlanetTerrainSystem>(SystemGroup::SimulationSystemGroup);

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
    auto planet1 = EntityManager::CreateEntity();
    auto planetTerrain1 = planet1.GetOrSetPrivateComponent<PlanetTerrain>().lock();
    planetTerrain1->m_surfaceMaterial = surfaceMaterial;
    planetTerrain1->SetPlanetInfo(pi);
    planet1.SetDataComponent(planetTransform);
    planet1.SetName("Planet 1");
    planetTransform.SetPosition(glm::vec3(35.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 20;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 15.0;
    pi.m_index = 1;

    auto planet2 = EntityManager::CreateEntity();
    auto planetTerrain2 = planet2.GetOrSetPrivateComponent<PlanetTerrain>().lock();
    planetTerrain2->m_surfaceMaterial = surfaceMaterial;
    planetTerrain2->SetPlanetInfo(pi);
    planet2.SetDataComponent(planetTransform);
    planet2.SetName("Planet 2");
    planetTransform.SetPosition(glm::vec3(-20.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 4;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 5.0;
    pi.m_index = 2;

    auto planet3 = EntityManager::CreateEntity();
    auto planetTerrain3 = planet3.GetOrSetPrivateComponent<PlanetTerrain>().lock();
    planetTerrain3->m_surfaceMaterial = surfaceMaterial;
    planetTerrain3->SetPlanetInfo(pi);
    planet3.SetDataComponent(planetTransform);
    planet3.SetName("Planet 3");
#pragma endregion

#pragma region Lights
    auto sharedMat = AssetManager::LoadMaterial(DefaultResources::GLPrograms::StandardProgram);

    Transform ltw;

    Entity dle = EntityManager::CreateEntity("Directional Light");
    dle.SetName("Directional Light 1");
    auto dlc = dle.GetOrSetPrivateComponent<DirectionalLight>().lock();
    dlc->m_diffuse = glm::vec3(1.0f);
    ltw.SetScale(glm::vec3(0.5f));

    Entity ple = EntityManager::CreateEntity("Point Light 1");
    auto plmmc = ple.GetOrSetPrivateComponent<MeshRenderer>().lock();
    plmmc->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    plmmc->m_material.Set<Material>(sharedMat);
    auto plc = ple.GetOrSetPrivateComponent<PointLight>().lock();
    plc->m_constant = 1.0f;
    plc->m_linear = 0.09f;
    plc->m_quadratic = 0.032f;
    plc->m_diffuse = glm::vec3(1.0f);
    plc->m_diffuseBrightness = 5;

    ple.SetDataComponent(ltw);

    Entity ple2 = EntityManager::CreateEntity("Point Light 2");
    auto plc2 = ple2.GetOrSetPrivateComponent<PointLight>().lock();
    plc2->m_constant = 1.0f;
    plc2->m_linear = 0.09f;
    plc2->m_quadratic = 0.032f;
    plc2->m_diffuse = glm::vec3(1.0f);
    plc2->m_diffuseBrightness = 5;

    ple2.SetDataComponent(ltw);
    ple2.SetName("Point Light 2");
    auto plmmc2 = ple2.GetOrSetPrivateComponent<MeshRenderer>().lock();
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