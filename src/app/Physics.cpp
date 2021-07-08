#include "ResourceManager.hpp"
#include <RigidBody.hpp>
#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
using namespace UniEngine;
void CreateSolidCube(
    const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string &name);

int main()
{
    Application::Init();
    auto &world = Application::GetCurrentWorld();
    world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
#pragma region Create 9 spheres in different PBR properties
    const int amount = 20;
    const float scaleFactor = 0.1f;
    const auto collection = EntityManager::CreateEntity("Spheres");
    auto spheres = EntityManager::CreateEntities(amount * amount, "Instance");
    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            auto transform = sphere.GetComponentData<Transform>();
            auto globalTransform = sphere.GetComponentData<GlobalTransform>();
            glm::vec3 position =
                glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            position += glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f)) * scaleFactor;
            transform.SetPosition(position * 5.0f * scaleFactor);
            globalTransform.SetPosition(position * 5.0f * scaleFactor);
            transform.SetScale(glm::vec3(2.0f * scaleFactor));
            globalTransform.SetScale(glm::vec3(2.0f * scaleFactor));
            sphere.SetComponentData(transform);
            sphere.SetComponentData(globalTransform);
            auto meshRenderer = std::make_unique<MeshRenderer>();
            meshRenderer->m_mesh = DefaultResources::Primitives::Sphere;
            meshRenderer->m_material = ResourceManager::CreateResource<Material>();
            meshRenderer->m_material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer->m_material->m_roughness = static_cast<float>(i) / (amount - 1);
            meshRenderer->m_material->m_metallic = static_cast<float>(j) / (amount - 1);
            sphere.SetPrivateComponent(std::move(meshRenderer));

            auto rigidBody = std::make_unique<RigidBody>();
            sphere.SetPrivateComponent(std::move(rigidBody));
            sphere.GetPrivateComponent<RigidBody>()->SetEnabled(true);
            sphere.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Sphere);
            sphere.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
            

            sphere.SetParent(collection);
        }
    }
#pragma endregion

#pragma region Lighting
    const auto dirLightEntity = EntityManager::CreateEntity("Dir Light");
    auto dirLight = std::make_unique<DirectionalLight>();
    dirLight->m_diffuseBrightness = 3.0f;
    dirLight->m_lightSize = 0.2f;
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100, 0, 0)));
    dirLightEntity.SetComponentData(dirLightTransform);
    dirLightEntity.SetPrivateComponent(std::move(dirLight));
#pragma endregion

#pragma region Create Boundaries
    CreateSolidCube(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30), "Ground");

    CreateSolidCube(glm::vec3(30, -10, 0), glm::vec3(0), glm::vec3(1, 5, 30), "LeftWall");
    CreateSolidCube(glm::vec3(-30, -10, 0), glm::vec3(0), glm::vec3(1, 5, 30), "RightWall");
    CreateSolidCube(glm::vec3(0, -10, 30), glm::vec3(0), glm::vec3(30, 5, 1), "FrontWall");
    CreateSolidCube(glm::vec3(0, -10, -30), glm::vec3(0), glm::vec3(30, 5, 1), "BackWall");

    CreateSolidCube(glm::vec3(-5, -7.5, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 1");
    CreateSolidCube(glm::vec3(0, -10, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 2");
    CreateSolidCube(glm::vec3(5, -7.5, 0), glm::vec3(0,0,45), glm::vec3(1), "Block 3");
#pragma endregion
    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}

void CreateSolidCube(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string& name)
{
    auto ground = EntityManager::CreateEntity(name);
    auto groundMeshRenderer = std::make_unique<MeshRenderer>();
    groundMeshRenderer->m_material = DefaultResources::Materials::StandardMaterial;
    groundMeshRenderer->m_mesh = DefaultResources::Primitives::Cube;
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), scale);
    //groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    ground.SetComponentData(groundTransform);
    ground.SetPrivateComponent(std::move(groundMeshRenderer));
    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), scale);
    ground.SetComponentData(groundGlobalTransform);

    auto rigidBody = std::make_unique<RigidBody>();
    ground.SetPrivateComponent(std::move(rigidBody));
    ground.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Box);
    ground.GetPrivateComponent<RigidBody>()->SetStatic(true);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    ground.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
    ground.GetPrivateComponent<RigidBody>()->SetEnabled(true);
}
