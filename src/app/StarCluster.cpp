// Galaxy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Application.hpp>
#include <PostProcessing.hpp>
#include <StarCluster/StarClusterSystem.hpp>
#include <PlayerController.hpp>
using namespace UniEngine;
using namespace Galaxy;

DataComponentRegistration<StarPosition> StarPositionReg("StarPosition");
DataComponentRegistration<SelectionStatus> SelectionStatusReg("SelectionStatus");
DataComponentRegistration<StarInfo> StarInfoReg("StarInfo");
DataComponentRegistration<SurfaceColor> SurfaceColorReg("SurfaceColor");
DataComponentRegistration<DisplayColor> DisplayColorReg("DisplayColor");
DataComponentRegistration<OriginalColor> OriginalColorReg("OriginalColor");
DataComponentRegistration<StarOrbitOffset> StarOrbitOffsetReg("StarOrbitOffset");
DataComponentRegistration<StarOrbitProportion> StarOrbitProportionReg("StarOrbitProportion");
DataComponentRegistration<StarOrbit> StarOrbitReg("StarOrbit");
DataComponentRegistration<StarClusterIndex> StarClusterIndexReg("StarClusterIndex");

SystemRegistration<StarClusterSystem> StarClusterSystemReg("StarClusterSystem");
int main()
{

#pragma region Application Preparations
    Application::Init();

    auto mainCameraEntity = RenderManager::GetMainCamera().lock()->GetOwner();
    mainCameraEntity.GetOrSetPrivateComponent<PlayerController>();
#pragma endregion
#pragma region Star System
    auto starClusterSystem = EntityManager::GetOrCreateSystem<StarClusterSystem>(SystemGroup::SimulationSystemGroup);
#pragma endregion
    auto postProcessing =
        RenderManager::GetMainCamera().lock()->GetOwner().GetOrSetPrivateComponent<PostProcessing>().lock();
    auto bloom = postProcessing->GetLayer<Bloom>().lock();
    bloom->m_intensity = 0.1f;
    bloom->m_diffusion = 8;
    bloom->m_enabled = true;

    postProcessing->GetLayer<SSAO>().lock()->m_enabled = false;
    RenderManager::GetMainCamera().lock()->m_useClearColor = true;
#pragma region EngineLoop
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}