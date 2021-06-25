#include "ResourceManager.hpp"

#include <Application.hpp>
#include <MeshRenderer.hpp>
#include <CameraControlSystem.hpp>
using namespace UniEngine;

int main()
{
    Application::Init();
    auto &world = Application::GetCurrentWorld();
    CameraControlSystem *ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
    ccs->Enable();

    int amount = 6;
    auto collection = EntityManager::CreateEntity("Spheres");
    auto spheres = EntityManager::CreateEntities(amount * amount, "Instance");
    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            auto transform = sphere.GetComponentData<Transform>();
            auto globalTransform = sphere.GetComponentData<GlobalTransform>();
            glm::vec3 position = glm::vec3(i + 0.5f - amount / 2.0f, j + 0.5f - amount / 2.0f, 0);
            transform.SetPosition(position * 3.0f);
            globalTransform.SetPosition(position * 3.0f);
            transform.SetScale(glm::vec3(1.0f));
            globalTransform.SetScale(glm::vec3(1.0f));
            sphere.SetComponentData(transform);
            sphere.SetComponentData(globalTransform);
            auto meshRenderer = std::make_unique<MeshRenderer>();
            meshRenderer->m_mesh = DefaultResources::Primitives::Sphere;
            meshRenderer->m_material = ResourceManager::CreateResource<Material>();
            meshRenderer->m_material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer->m_material->m_roughness = static_cast<float>(i) / (amount - 1);
            meshRenderer->m_material->m_metallic = static_cast<float>(j) / (amount - 1);
            sphere.SetPrivateComponent(std::move(meshRenderer));

            sphere.SetParent(collection);
        }
    }

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}
