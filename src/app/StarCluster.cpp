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
    auto &world = Application::GetCurrentWorld();
    CameraControlSystem *ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    ccs->Enable();

#pragma endregion
#pragma region Star System
    auto *starClusterSystem = world->CreateSystem<StarClusterSystem>(SystemGroup::SimulationSystemGroup);
#pragma endregion
    auto postProcessing = std::make_unique<PostProcessing>();
    postProcessing->PushLayer(std::make_unique<Bloom>());
    Bloom *bloom = postProcessing->GetLayer<Bloom>();
    if (bloom != nullptr)
    {
        bloom->m_intensity = 0.03f;
        bloom->m_diffusion = 20;
        bloom->m_enabled = true;
    }
    RenderManager::GetMainCamera()->GetOwner().SetPrivateComponent<PostProcessing>();
    RenderManager::GetMainCamera()->m_useClearColor = true;
#pragma region EngineLoop
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}