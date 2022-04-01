#include "ProjectManager.hpp"

#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    Curve curve = Curve({0, 0}, {1, 1});
    ProjectManager::SetScenePostLoadActions([&]() {
        Application::RegisterLateUpdateFunction([&](){
            ImGui::ShowDemoWindow();
            if(ImGui::Begin("Test"))
            {
                curve.OnInspect(
                    "Curve",
                    ImVec2(-1, -1),
                    (unsigned)CurveEditorFlags::ALLOW_RESIZE | (unsigned)CurveEditorFlags::SHOW_GRID |
                        (unsigned)CurveEditorFlags::SHOW_DEBUG);

                FileUtils::OpenFolder("Test", [](const std::filesystem::path &path){
                    UNIENGINE_LOG(path.string());
                }, false);
            }

            ImGui::End();
        });
    });
    ApplicationConfigs applicationConfigs;
    Application::Create(applicationConfigs);

    return 0;
}
