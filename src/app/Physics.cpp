#include "ResourceManager.hpp"
#include <RigidBody.hpp>
#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <MeshRenderer.hpp>
#include <DistanceJoint.hpp>
#include <FixedJoint.hpp>
using namespace UniEngine;

Entity CreateDynamicCube(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string& name);

Entity CreateSolidCube(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string &name);

Entity CreateCube(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string &name);


Entity CreateDynamicSphere(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string& name);

Entity CreateSolidSphere(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string &name);

Entity CreateSphere(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string &name);



int main()
{
    Application::Init();
    auto &world = Application::GetCurrentWorld();
    world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);

    auto mainCameraEntity = RenderManager::GetMainCamera()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetComponentData<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    mainCameraEntity.SetComponentData(mainCameraTransform);

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
    CreateDynamicCube(glm::vec3(1.0f), glm::vec3(0, 30, 0), glm::vec3(0, 0, 0), glm::vec3(2, 1, 2), "Dropping box");

    const auto ground = CreateSolidCube(glm::vec3(1.0f), glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30), "Ground");

    CreateSolidCube(glm::vec3(1.0f), glm::vec3(30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "LeftWall");
    CreateSolidCube(glm::vec3(1.0f), glm::vec3(-30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "RightWall");
    CreateSolidCube(glm::vec3(1.0f), glm::vec3(0, -10, 30), glm::vec3(0), glm::vec3(30, 15, 1), "FrontWall");
    CreateSolidCube(glm::vec3(1.0f), glm::vec3(0, -10, -30), glm::vec3(0), glm::vec3(30, 15, 1), "BackWall");

    const auto b1 = CreateDynamicCube(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-5, -7.5, 0), glm::vec3(0, 0, 45), glm::vec3(0.5), "Block 1");
    const auto b2 = CreateDynamicCube(glm::vec3(1.0f), glm::vec3(0, -10, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 2");
    const auto b3 = CreateDynamicCube(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(5, -7.5, 0), glm::vec3(0,0,45), glm::vec3(1), "Block 3");

    b1.SetPrivateComponent(std::make_unique<FixedJoint>());
    b1.GetPrivateComponent<FixedJoint>()->m_linkedEntity = b2;
    b1.GetPrivateComponent<FixedJoint>()->Link();
    b3.SetPrivateComponent(std::make_unique<FixedJoint>());
    b3.GetPrivateComponent<FixedJoint>()->m_linkedEntity = b2;
    b3.GetPrivateComponent<FixedJoint>()->Link();
    b2.SetPrivateComponent(std::make_unique<FixedJoint>());
    b2.GetPrivateComponent<FixedJoint>()->m_linkedEntity = ground;
    b2.GetPrivateComponent<FixedJoint>()->Link();

    const auto anchor = CreateSolidCube(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.2f), "Anchor");
    const auto freeSphere = CreateDynamicCube(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-15, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.5), "Free Cube");
    //freeSphere.SetPrivateComponent(std::make_unique<DistanceJoint>());
    //freeSphere.GetPrivateComponent<DistanceJoint>()->m_linkedEntity = anchor;
    //freeSphere.GetPrivateComponent<DistanceJoint>()->Link();
#pragma endregion
    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
}

Entity CreateSolidCube(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string& name)
{
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = std::make_unique<RigidBody>();
    cube.SetPrivateComponent(std::move(rigidBody));
    cube.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Box);
    cube.GetPrivateComponent<RigidBody>()->SetStatic(true);
    cube.GetPrivateComponent<RigidBody>()->UpdateMass(0.1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    cube.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
    cube.GetPrivateComponent<RigidBody>()->SetEnabled(true);
    return cube;
}

Entity CreateDynamicCube(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string& name)
{
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = std::make_unique<RigidBody>();
    cube.SetPrivateComponent(std::move(rigidBody));
    cube.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Box);
    cube.GetPrivateComponent<RigidBody>()->SetStatic(false);
    cube.GetPrivateComponent<RigidBody>()->UpdateMass(0.1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    cube.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
    cube.GetPrivateComponent<RigidBody>()->SetEnabled(true);
    return cube;
}


Entity CreateCube(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const std::string& name){
    auto cube = EntityManager::CreateEntity(name);
    auto groundMeshRenderer = std::make_unique<MeshRenderer>();
    groundMeshRenderer->m_material = ResourceManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer->m_material->m_albedoColor = color;
    groundMeshRenderer->m_mesh = DefaultResources::Primitives::Cube;
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), scale);
    //groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    cube.SetComponentData(groundTransform);
    cube.SetPrivateComponent(std::move(groundMeshRenderer));
    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), scale);
    cube.SetComponentData(groundGlobalTransform);
    return cube;
}

Entity CreateDynamicSphere(const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string& name){
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = std::make_unique<RigidBody>();
    sphere.SetPrivateComponent(std::move(rigidBody));
    sphere.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Sphere);
    sphere.GetPrivateComponent<RigidBody>()->SetStatic(false);
    sphere.GetPrivateComponent<RigidBody>()->UpdateMass(0.1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    sphere.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
    sphere.GetPrivateComponent<RigidBody>()->SetEnabled(true);
    return sphere;
}

Entity CreateSolidSphere(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string &name){
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = std::make_unique<RigidBody>();
    sphere.SetPrivateComponent(std::move(rigidBody));
    sphere.GetPrivateComponent<RigidBody>()->SetShapeType(ShapeType::Sphere);
    sphere.GetPrivateComponent<RigidBody>()->SetStatic(true);
    sphere.GetPrivateComponent<RigidBody>()->UpdateMass(0.1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    sphere.GetPrivateComponent<RigidBody>()->ApplyMeshBound();
    sphere.GetPrivateComponent<RigidBody>()->SetEnabled(true);
    return sphere;
}

Entity CreateSphere(
    const glm::vec3 color, const glm::vec3 &position, const glm::vec3 &rotation, const float &scale, const std::string &name){
    auto sphere = EntityManager::CreateEntity(name);
    auto groundMeshRenderer = std::make_unique<MeshRenderer>();
    groundMeshRenderer->m_material = ResourceManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer->m_material->m_albedoColor = color;
    groundMeshRenderer->m_mesh = DefaultResources::Primitives::Sphere;
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    //groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    sphere.SetComponentData(groundTransform);
    sphere.SetPrivateComponent(std::move(groundMeshRenderer));
    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    sphere.SetComponentData(groundGlobalTransform);
    return sphere;
}