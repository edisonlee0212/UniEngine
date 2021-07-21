// Planet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ResourceManager.hpp"

#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
#include <Planet/PlanetTerrainSystem.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;
using namespace Planet;
int main()
{
    ComponentFactory::RegisterSerializable<PlanetTerrain>();
    Application::Init();
#pragma region Preparations
    auto &world = EntityManager::GetCurrentWorld();
    EntityArchetype archetype = EntityManager::CreateEntityArchetype("General", Transform(), GlobalTransform());

    auto ccs = world->CreateSystem<CameraControlSystem>("CameraControlSystem", SystemGroup::SimulationSystemGroup);
    ccs->SetSensitivity(0.1f);
    ccs->SetVelocity(15.0f);
    ccs->Enable();

    RenderManager::GetMainCamera()->m_useClearColor = false;

    auto pts = world->CreateSystem<PlanetTerrainSystem>("PlanetTerrainSystem", SystemGroup::SimulationSystemGroup);
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
    auto planet1 = EntityManager::CreateEntity(archetype);
    auto &planetTerrain1 = planet1.SetPrivateComponent<PlanetTerrain>();
    planetTerrain1.Init(pi);
    planet1.SetDataComponent(planetTransform);
    planet1.SetName("Planet 1");
    planetTransform.SetPosition(glm::vec3(35.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 20;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 15.0;
    pi.m_index = 1;

    auto planet2 = EntityManager::CreateEntity(archetype);
    auto &planetTerrain2 = planet2.SetPrivateComponent<PlanetTerrain>();
    planetTerrain2.Init(pi);
    planet2.SetDataComponent(planetTransform);
    planet2.SetName("Planet 2");
    planetTransform.SetPosition(glm::vec3(-20.0f, 0.0f, 0.0f));
    pi.m_maxLodLevel = 4;
    pi.m_lodDistance = 7.0;
    pi.m_radius = 5.0;
    pi.m_index = 2;

    auto planet3 = EntityManager::CreateEntity(archetype);
    auto &planetTerrain3 = planet3.SetPrivateComponent<PlanetTerrain>();
    planetTerrain3.Init(pi);
    planet3.SetDataComponent(planetTransform);
    planet3.SetName("Planet 3");
#pragma endregion

#pragma region Lights
    auto sharedMat = ResourceManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);

    Transform ltw;

    Entity dle = EntityManager::CreateEntity("Directional Light");
    dle.SetName("Directional Light 1");
    auto &dlc = dle.SetPrivateComponent<DirectionalLight>();
    dlc.m_diffuse = glm::vec3(1.0f);
    ltw.SetScale(glm::vec3(0.5f));

    Entity ple = EntityManager::CreateEntity("Point Light 1");
    auto &plmmc = ple.SetPrivateComponent<MeshRenderer>();
    plmmc.m_mesh = DefaultResources::Primitives::Sphere;
    plmmc.m_material = sharedMat;
    auto &plc = ple.SetPrivateComponent<PointLight>();
    plc.m_constant = 1.0f;
    plc.m_linear = 0.09f;
    plc.m_quadratic = 0.032f;
    plc.m_diffuse = glm::vec3(1.0f);
    plc.m_diffuseBrightness = 5;

    ple.SetDataComponent(ltw);

    Entity ple2 = EntityManager::CreateEntity("Point Light 2");
    auto &plc2 = ple2.SetPrivateComponent<PointLight>();
    plc2.m_constant = 1.0f;
    plc2.m_linear = 0.09f;
    plc2.m_quadratic = 0.032f;
    plc2.m_diffuse = glm::vec3(1.0f);
    plc2.m_diffuseBrightness = 5;

    ple2.SetDataComponent(ltw);
    ple2.SetName("Point Light 2");
    auto &plmmc2 = ple2.SetPrivateComponent<MeshRenderer>();
    plmmc2.m_mesh = DefaultResources::Primitives::Sphere;
    plmmc2.m_material = sharedMat;

#pragma endregion

#pragma region EngineLoop
    Application::RegisterPreUpdateFunction([&]() {
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
    });
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}
