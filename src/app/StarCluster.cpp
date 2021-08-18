// Galaxy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Application.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <StarCluster/StarClusterSystem.hpp>
using namespace UniEngine;
using namespace Galaxy;

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

    ProjectManager::SetScenePostLoadActions([]() {
        auto mainCameraEntity = RenderManager::GetMainCamera().lock()->GetOwner();
        mainCameraEntity.GetOrSetPrivateComponent<PlayerController>();
#pragma region Star System
        auto starClusterSystem =
            EntityManager::GetOrCreateSystem<StarClusterSystem>(SystemGroup::SimulationSystemGroup);
#pragma endregion
        auto postProcessing =
            RenderManager::GetMainCamera().lock()->GetOwner().GetOrSetPrivateComponent<PostProcessing>().lock();
        auto bloom = postProcessing->GetLayer<Bloom>().lock();
        bloom->m_intensity = 0.1f;
        bloom->m_diffusion = 8;
        bloom->m_enabled = true;

        postProcessing->GetLayer<SSAO>().lock()->m_enabled = false;
        RenderManager::GetMainCamera().lock()->m_useClearColor = true;
    });
    Application::Init();
#pragma region EngineLoop
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}