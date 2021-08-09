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
    SerializableFactory::RegisterSerializable<CameraControlSystem>("CameraControlSystem");
    SerializableFactory::RegisterSerializable<StarClusterSystem>("StarClusterSystem");
    auto ccs = EntityManager::GetOrCreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);

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