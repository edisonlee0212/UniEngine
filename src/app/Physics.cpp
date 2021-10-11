#include "AssetManager.hpp"
#include <Application.hpp>
#include <Collider.hpp>
#include <Joint.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;

#pragma region Helpers
Entity CreateDynamicCube(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name);

Entity CreateSolidCube(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name);

Entity CreateCube(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name);

Entity CreateDynamicSphere(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name);

Entity CreateSolidSphere(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name);

Entity CreateSphere(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name);
#pragma endregion
void LoadScene();
int main()
{
    ProjectManager::SetScenePostLoadActions([](){
        LoadScene();
    });

    const std::filesystem::path resourceFolderPath("../Resources");
    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Physics/Physics.ueproj";
    Application::Init(applicationConfigs);

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();
    Application::End();
#pragma endregion
    return 0;
}
void LoadScene(){
    auto mainCameraEntity = RenderManager::GetMainCamera().lock()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    mainCameraEntity.GetOrSetPrivateComponent<PlayerController>();
    auto postProcessing = mainCameraEntity.GetOrSetPrivateComponent<PostProcessing>().lock();

#pragma region Create 9 spheres in different PBR properties
    const int amount = 20;
    const float scaleFactor = 0.1f;
    const auto collection = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), "Spheres");
    auto spheres = EntityManager::CreateEntities(EntityManager::GetCurrentScene(), amount * amount, "Instance");

    for (int i = 0; i < amount; i++)
    {
        for (int j = 0; j < amount; j++)
        {
            auto &sphere = spheres[i * amount + j];
            Transform transform;
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            position += glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f)) * scaleFactor;
            transform.SetPosition(position * 5.0f * scaleFactor);
            transform.SetScale(glm::vec3(2.0f * scaleFactor));
            sphere.SetDataComponent(transform);
            auto meshRenderer = sphere.GetOrSetPrivateComponent<MeshRenderer>().lock();
            meshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
            auto material = AssetManager::CreateAsset<Material>();
            meshRenderer->m_material.Set<Material>(material);
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            material->m_roughness = static_cast<float>(i) / (amount - 1);
            material->m_metallic = static_cast<float>(j) / (amount - 1);

            auto rigidBody = sphere.GetOrSetPrivateComponent<RigidBody>().lock();
            rigidBody->SetEnabled(true);
            rigidBody->SetDensityAndMassCenter(0.1);
            auto sphereCollider = AssetManager::CreateAsset<Collider>();
            sphereCollider->SetShapeType(ShapeType::Sphere);
            sphereCollider->SetShapeParam(glm::vec3(2.0 * scaleFactor));
            rigidBody->AttachCollider(sphereCollider);
            sphere.SetParent(collection);
        }
    }
#pragma endregion

#pragma region Lighting
    const auto dirLightEntity = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), "Dir Light");
    auto dirLight = dirLightEntity.GetOrSetPrivateComponent<DirectionalLight>().lock();
    dirLight->m_diffuseBrightness = 3.0f;
    dirLight->m_lightSize = 0.2f;
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100, 0, 0)));
    dirLightEntity.SetDataComponent(dirLightTransform);

#pragma endregion

#pragma region Create Boundaries
    {

        const auto ground = CreateSolidCube(
            1.0, glm::vec3(1.0f), glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30), "Ground");

        CreateSolidCube(
            1.0, glm::vec3(1.0f), glm::vec3(30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "LeftWall");
        CreateSolidCube(
            1.0, glm::vec3(1.0f), glm::vec3(-30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "RightWall");
        CreateSolidCube(
            1.0, glm::vec3(1.0f), glm::vec3(0, -10, 30), glm::vec3(0), glm::vec3(30, 15, 1), "FrontWall");
        CreateSolidCube(
            1.0, glm::vec3(1.0f), glm::vec3(0, -10, -30), glm::vec3(0), glm::vec3(30, 15, 1), "BackWall");

        const auto b1 = CreateDynamicCube(
            1.0,
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(-5, -7.5, 0),
            glm::vec3(0, 0, 45),
            glm::vec3(0.5),
            "Block 1");
        const auto b2 = CreateDynamicCube(
            1.0, glm::vec3(1.0f), glm::vec3(0, -10, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 2");
        const auto b3 = CreateDynamicCube(
            1.0, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(5, -7.5, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 3");

        auto b1j = b1.GetOrSetPrivateComponent<Joint>().lock();
        b1j->SetType(JointType::Fixed);
        b1j->Link(b2);
        auto b3j = b3.GetOrSetPrivateComponent<Joint>().lock();
        b3j->SetType(JointType::Fixed);
        b3j->Link(b2);
        auto b2j = b2.GetOrSetPrivateComponent<Joint>().lock();
        b2j->SetType(JointType::Fixed);
        b2j->Link(ground);

        const auto anchor = CreateDynamicCube(
            1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.2f), "Anchor");
        anchor.GetOrSetPrivateComponent<RigidBody>().lock()->SetKinematic(true);
        auto lastLink = anchor;
        for (int i = 1; i < 10; i++)
        {
            const auto link = CreateDynamicSphere(
                1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10 - i, 0, 0), glm::vec3(0, 0, 45), 0.2f, "Link");
            auto joint = link.GetOrSetPrivateComponent<Joint>().lock();
            joint->SetType(JointType::D6);
            joint->Link(lastLink);
            // joint->SetMotion(MotionAxis::SwingY, MotionType::Limited);
            link.SetParent(anchor);
            lastLink = link;
        }

        const auto freeSphere = CreateDynamicCube(
            0.01,
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(-20, 0, 0),
            glm::vec3(0, 0, 45),
            glm::vec3(0.5),
            "Free Cube");
        auto joint = freeSphere.GetOrSetPrivateComponent<Joint>().lock();
        joint->SetType(JointType::D6);
        joint->Link(lastLink);
        // joint->SetMotion(MotionAxis::TwistX, MotionType::Free);
        joint->SetMotion(MotionAxis::SwingY, MotionType::Free);
        joint->SetMotion(MotionAxis::SwingZ, MotionType::Free);
    }
#pragma endregion
}
Entity CreateSolidCube(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name)
{
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = cube.GetOrSetPrivateComponent<RigidBody>().lock();
    rigidBody->SetStatic(true);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);

    auto collider = AssetManager::CreateAsset<Collider>();
    collider->SetShapeType(ShapeType::Box);
    collider->SetShapeParam(scale);
    rigidBody->AttachCollider(collider);
    return cube;
}

Entity CreateDynamicCube(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name)
{
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = cube.GetOrSetPrivateComponent<RigidBody>().lock();
    rigidBody->SetStatic(false);
    rigidBody->SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);
    rigidBody->SetDensityAndMassCenter(mass / scale.x / scale.y / scale.z);

    auto collider = AssetManager::CreateAsset<Collider>();
    collider->SetShapeType(ShapeType::Box);
    collider->SetShapeParam(scale);
    rigidBody->AttachCollider(collider);
    return cube;
}

Entity CreateCube(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name)
{
    auto cube = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), name);
    auto groundMeshRenderer = cube.GetOrSetPrivateComponent<MeshRenderer>().lock();
    groundMeshRenderer->m_material.Set<Material>(
        AssetManager::LoadMaterial(DefaultResources::GLPrograms::StandardProgram));
    groundMeshRenderer->m_material.Get<Material>()->m_albedoColor = color;
    groundMeshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Cube);
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), scale);
    // groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    cube.SetDataComponent(groundTransform);

    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), scale);
    cube.SetDataComponent(groundGlobalTransform);
    return cube;
}

Entity CreateDynamicSphere(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name)
{
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = sphere.GetOrSetPrivateComponent<RigidBody>().lock();
    rigidBody->SetStatic(false);
    rigidBody->SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);
    rigidBody->SetDensityAndMassCenter(mass / scale / scale / scale);

    auto collider = AssetManager::CreateAsset<Collider>();
    collider->SetShapeType(ShapeType::Sphere);
    collider->SetShapeParam(glm::vec3(scale));
    rigidBody->AttachCollider(collider);
    return sphere;
}

Entity CreateSolidSphere(
    const float &mass,
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name)
{
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = sphere.GetOrSetPrivateComponent<RigidBody>().lock();
    rigidBody->SetStatic(true);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);

    auto collider = AssetManager::CreateAsset<Collider>();
    collider->SetShapeType(ShapeType::Sphere);
    collider->SetShapeParam(glm::vec3(scale));
    rigidBody->AttachCollider(collider);
    return sphere;
}

Entity CreateSphere(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name)
{
    auto sphere = EntityManager::CreateEntity(EntityManager::GetCurrentScene(), name);
    auto groundMeshRenderer = sphere.GetOrSetPrivateComponent<MeshRenderer>().lock();
    groundMeshRenderer->m_material.Set<Material>(
        AssetManager::LoadMaterial(DefaultResources::GLPrograms::StandardProgram));
    groundMeshRenderer->m_material.Get<Material>()->m_albedoColor = color;
    groundMeshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    // groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    sphere.SetDataComponent(groundTransform);

    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    sphere.SetDataComponent(groundGlobalTransform);
    return sphere;
}