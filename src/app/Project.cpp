#include "AssetManager.hpp"

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
                curve.CurveEditor(
                    "Curve",
                    ImVec2(-1, -1),
                    (unsigned)CurveEditorFlags::ALLOW_RESIZE | (unsigned)CurveEditorFlags::SHOW_GRID |
                        (unsigned)CurveEditorFlags::SHOW_DEBUG);
            }
            ImGui::End();
        });
    });
    ApplicationConfigs applicationConfigs;
    Application::Create(applicationConfigs);

    auto mesh = AssetManager::CreateAsset<Mesh>();
    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> triangles;
    SphereMeshGenerator::Icosahedron(positions, triangles);
    std::vector<Vertex> vertices;
    vertices.resize(positions.size());
    for(int i = 0; i < vertices.size(); i++){
        vertices[i].m_position = positions[i];
    }
    mesh->SetVertices(17, vertices, triangles);
    AssetManager::Share(mesh);
    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Start();
    Application::End();
#pragma endregion
    return 0;
}
