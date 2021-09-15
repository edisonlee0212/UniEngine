#include "AssetManager.hpp"

#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    std::vector<glm::vec2> points;
    ProjectManager::SetScenePostLoadActions([&]() {
        Application::RegisterLateUpdateFunction([&](){
            ImGui::ShowDemoWindow();

            ImGui::Begin("Test");
            CurveEditor("Test", points, ImVec2(-1, -1), (unsigned)CurveEditorFlags::ALLOW_RESIZE | (unsigned)CurveEditorFlags::SHOW_GRID); //
            ImGui::End();
        });
    });

    Application::Init();
    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}
