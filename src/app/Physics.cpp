#include "ResourceManager.hpp"

#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
using namespace UniEngine;

int main()
{
    Application::Init();
    auto &world = Application::GetCurrentWorld();
    CameraControlSystem *ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    ccs->Enable();

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}
