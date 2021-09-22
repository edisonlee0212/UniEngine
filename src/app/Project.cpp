#include "AssetManager.hpp"

#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    Curve curve = Curve({0, 0}, {1, 1});
    Curve curve1 = Curve({0, 0}, {1, 1});
    Curve curve2 = Curve({0, 0}, {1, 1});
    Curve curve3 = Curve({0, 0}, {1, 1});
    Curve curve4 = Curve({0, 0}, {1, 1});
    Curve curve5 = Curve({0, 0}, {1, 1});
    Curve curve6 = Curve({0, 0}, {1, 1});

    ProjectManager::SetScenePostLoadActions([&]() {
        Application::RegisterLateUpdateFunction([&](){
            ImGui::ShowDemoWindow();
        });
    });
    ApplicationConfigs applicationConfigs;
    Application::Init(applicationConfigs);
    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}
