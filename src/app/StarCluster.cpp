// Galaxy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Application.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <StarCluster/StarClusterSystem.hpp>
#include "ClassRegistry.hpp"
using namespace UniEngine;
using namespace Galaxy;
void LoadScene();
int main()
{
    ClassRegistry::RegisterDataComponent<StarPosition>("StarPosition");
    ClassRegistry::RegisterDataComponent<SelectionStatus>("SelectionStatus");
    ClassRegistry::RegisterDataComponent<StarInfo>("StarInfo");
    ClassRegistry::RegisterDataComponent<SurfaceColor>("SurfaceColor");
    ClassRegistry::RegisterDataComponent<DisplayColor>("DisplayColor");
    ClassRegistry::RegisterDataComponent<OriginalColor>("OriginalColor");
    ClassRegistry::RegisterDataComponent<StarOrbitOffset>("StarOrbitOffset");
    ClassRegistry::RegisterDataComponent<StarOrbitProportion>("StarOrbitProportion");
    ClassRegistry::RegisterDataComponent<StarOrbit>("StarOrbit");
    ClassRegistry::RegisterDataComponent<StarClusterIndex>("StarClusterIndex");

    ClassRegistry::RegisterSystem<StarClusterSystem>("StarClusterSystem");

    const std::filesystem::path resourceFolderPath("../../../Resources");

    for (const auto i : std::filesystem::recursive_directory_iterator(resourceFolderPath))
    {
        if (i.is_directory()) continue;
        if (i.path().extension().string() == ".uescene" || i.path().extension().string() == ".umeta" || i.path().extension().string() == ".ueproj" || i.path().extension().string() == ".ufmeta")
        {
            std::filesystem::remove(i.path());
        }
    }

    ProjectManager::SetScenePostLoadActions([](){
        LoadScene();
    });
    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Star Cluster/Star Cluster.ueproj";
    Application::Create(applicationConfigs);

#pragma region EngineLoop
    Application::Start();
    Application::End();
#pragma endregion
    return 0;
}

void LoadScene(){
    auto scene =Application::GetActiveScene();
    const auto mainCamera = scene->m_mainCamera.Get<Camera>();
    auto mainCameraEntity = mainCamera->GetOwner();
    scene->GetOrSetPrivateComponent<PlayerController>(mainCameraEntity);
#pragma region Star System
    auto starClusterSystem =
        scene->GetOrCreateSystem<StarClusterSystem>(SystemGroup::SimulationSystemGroup);
#pragma endregion
    auto postProcessing =
        scene->GetOrSetPrivateComponent<PostProcessing>(mainCamera->GetOwner()).lock();
    auto bloom = postProcessing->GetLayer<Bloom>().lock();
    bloom->m_intensity = 0.1f;
    bloom->m_diffusion = 8;
    bloom->m_enabled = true;

    postProcessing->GetLayer<SSAO>().lock()->m_enabled = false;
    mainCamera->m_useClearColor = true;
}