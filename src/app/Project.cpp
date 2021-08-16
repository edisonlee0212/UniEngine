#include "AssetManager.hpp"

#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
using namespace UniEngine;

int main()
{
    Application::Init();
    RenderManager::GetInstance().m_lightSettings.m_ambientLight = 0.5f;


#pragma region Set main camera to correct position and rotation
    auto mainCameraEntity = RenderManager::GetMainCamera().lock()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, 0, 40));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    mainCameraEntity.GetOrSetPrivateComponent<PostProcessing>();
    auto camera = mainCameraEntity.GetOrSetPrivateComponent<Camera>().lock();
    mainCameraEntity.GetOrSetPrivateComponent<PlayerController>();
#pragma endregion


#pragma region Create 9 spheres in different PBR properties
    int amount = 4;
    auto collection = EntityManager::CreateEntity("Spheres");
    auto spheres = EntityManager::CreateEntities(amount * amount, "Instance");
    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            Transform transform;
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            transform.SetPosition(position * 5.0f);
            transform.SetScale(glm::vec3(2.0f));
            sphere.SetDataComponent(transform);
            auto meshRenderer = sphere.GetOrSetPrivateComponent<MeshRenderer>().lock();
            meshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
            auto material = AssetManager::CreateAsset<Material>();
            meshRenderer->m_material.Set<Material>(material);
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            material->m_roughness = static_cast<float>(i) / (amount - 1);
            material->m_metallic = static_cast<float>(j) / (amount - 1);

            sphere.SetParent(collection);
        }
    }
#pragma endregion

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}
