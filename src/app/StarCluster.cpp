// Galaxy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <PostProcessing.hpp>
#include <StarCluster/StarClusterSystem.hpp>
using namespace UniEngine;
using namespace Galaxy;
int main()
{
#pragma region Application Preparations
    Application::Init();
    auto &world = EntityManager::GetCurrentWorld();
    CameraControlSystem *ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    ccs->Enable();

#pragma endregion
#pragma region Star System
    auto *starClusterSystem = world->CreateSystem<StarClusterSystem>(SystemGroup::SimulationSystemGroup);
#pragma endregion
    auto& postProcessing = RenderManager::GetMainCamera()->GetOwner().SetPrivateComponent<PostProcessing>();
    Bloom *bloom = postProcessing.GetLayer<Bloom>();
    if (bloom != nullptr)
    {
        bloom->m_intensity = 0.1f;
        bloom->m_diffusion = 8;
        bloom->m_enabled = true;
    }
    postProcessing.GetLayer<SSAO>()->m_enabled = false;
    RenderManager::GetMainCamera()->m_useClearColor = true;
#pragma region EngineLoop
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}