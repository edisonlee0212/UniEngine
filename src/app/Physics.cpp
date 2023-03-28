#include "ProjectManager.hpp"
#include <Application.hpp>
#include <Collider.hpp>
#include <Joint.hpp>
#include <MeshRenderer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <RigidBody.hpp>
#include "DefaultResources.hpp"
#include "Graphics.hpp"
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
    ProjectManager::SetScenePostLoadActions([]() { LoadScene(); });

    const std::filesystem::path resourceFolderPath("../../../Resources");

    for(const auto i : std::filesystem::recursive_directory_iterator(resourceFolderPath))
    {
        if(i.is_directory()) continue;
	    if(i.path().extension().string() == ".uescene" || i.path().extension().string() == ".umeta" || i.path().extension().string() == ".ueproj" || i.path().extension().string() == ".ufmeta")
	    {
            std::filesystem::remove(i.path());
	    }
    }

    ApplicationConfigs applicationConfigs;
    applicationConfigs.m_projectPath = resourceFolderPath / "Example Projects/Physics/Physics.ueproj";
    Application::Create(applicationConfigs);

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Start();
    Application::End();
#pragma endregion
    return 0;
}
void LoadScene()
{
    auto scene = Application::GetActiveScene();
    const auto mainCamera = scene->m_mainCamera.Get<Camera>();
    auto mainCameraEntity = mainCamera->GetOwner();
    auto mainCameraTransform = scene->GetDataComponent<Transform>(mainCameraEntity);
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    scene->SetDataComponent(mainCameraEntity, mainCameraTransform);
    scene->GetOrSetPrivateComponent<PlayerController>(mainCameraEntity);
    auto postProcessing = scene->GetOrSetPrivateComponent<PostProcessing>(mainCameraEntity).lock();

#pragma region Create 9 spheres in different PBR properties
    const int amount = 20;
    const float scaleFactor = 0.1f;
    const auto collection = scene->CreateEntity("Spheres");
    auto spheres = scene->CreateEntities(amount * amount, "Instance");

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
            scene->SetDataComponent(sphere, transform);
            auto meshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(sphere).lock();
            meshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
            auto material = ProjectManager::CreateTemporaryAsset<Material>();
            meshRenderer->m_material.Set<Material>(material);
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            material->m_materialProperties.m_roughness = static_cast<float>(i) / (amount - 1);
            material->m_materialProperties.m_metallic = static_cast<float>(j) / (amount - 1);

            auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(sphere).lock();
            rigidBody->SetEnabled(true);
            rigidBody->SetDensityAndMassCenter(0.1);
            auto sphereCollider = ProjectManager::CreateTemporaryAsset<Collider>();
            sphereCollider->SetShapeType(ShapeType::Sphere);
            sphereCollider->SetShapeParam(glm::vec3(2.0 * scaleFactor));
            rigidBody->AttachCollider(sphereCollider);
            scene->SetParent(sphere, collection);
        }
    }
#pragma endregion

#pragma region Lighting
    const auto dirLightEntity = scene->CreateEntity("Dir Light");
    auto dirLight = scene->GetOrSetPrivateComponent<DirectionalLight>(dirLightEntity).lock();
    dirLight->m_diffuseBrightness = 3.0f;
    dirLight->m_lightSize = 0.2f;
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100, 0, 0)));
    scene->SetDataComponent(dirLightEntity, dirLightTransform);

#pragma endregion

#pragma region Create Boundaries
    {

        const auto ground =
            CreateSolidCube(1.0, glm::vec3(1.0f), glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30), "Ground");

        CreateSolidCube(1.0, glm::vec3(1.0f), glm::vec3(30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "LeftWall");
        CreateSolidCube(1.0, glm::vec3(1.0f), glm::vec3(-30, -10, 0), glm::vec3(0), glm::vec3(1, 15, 30), "RightWall");
        CreateSolidCube(1.0, glm::vec3(1.0f), glm::vec3(0, -10, 30), glm::vec3(0), glm::vec3(30, 15, 1), "FrontWall");
        CreateSolidCube(1.0, glm::vec3(1.0f), glm::vec3(0, -10, -30), glm::vec3(0), glm::vec3(30, 15, 1), "BackWall");

        const auto b1 = CreateDynamicCube(
            1.0, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-5, -7.5, 0), glm::vec3(0, 0, 45), glm::vec3(0.5), "Block 1");
        const auto b2 =
            CreateDynamicCube(1.0, glm::vec3(1.0f), glm::vec3(0, -10, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 2");
        const auto b3 = CreateDynamicCube(
            1.0, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(5, -7.5, 0), glm::vec3(0, 0, 45), glm::vec3(1), "Block 3");

        auto b1j = scene->GetOrSetPrivateComponent<Joint>(b1).lock();
        b1j->SetType(JointType::Fixed);
        b1j->Link(b2);
        auto b3j = scene->GetOrSetPrivateComponent<Joint>(b3).lock();
        b3j->SetType(JointType::Fixed);
        b3j->Link(b2);
        auto b2j = scene->GetOrSetPrivateComponent<Joint>(b2).lock();
        b2j->SetType(JointType::Fixed);
        b2j->Link(ground);

        const auto anchor = CreateDynamicCube(
            1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.2f), "Anchor");
        scene->GetOrSetPrivateComponent<RigidBody>(anchor).lock()->SetKinematic(true);
        auto lastLink = anchor;
        for (int i = 1; i < 10; i++)
        {
            const auto link = CreateDynamicSphere(
                1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10 - i, 0, 0), glm::vec3(0, 0, 45), 0.2f, "Link");
            auto joint = scene->GetOrSetPrivateComponent<Joint>(link).lock();
            joint->SetType(JointType::D6);
            joint->Link(lastLink);
            // joint->SetMotion(MotionAxis::SwingY, MotionType::Limited);
            scene->SetParent(link, anchor);
            lastLink = link;
        }

        const auto freeSphere = CreateDynamicCube(
            0.01, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-20, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.5), "Free Cube");
        auto joint = scene->GetOrSetPrivateComponent<Joint>(freeSphere).lock();
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
    auto scene = Application::GetActiveScene();
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(cube).lock();
    rigidBody->SetStatic(true);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);

    auto collider = ProjectManager::CreateTemporaryAsset<Collider>();
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
    auto scene = Application::GetActiveScene();
    auto cube = CreateCube(color, position, rotation, scale, name);
    auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(cube).lock();
    rigidBody->SetStatic(false);
    rigidBody->SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);
    rigidBody->SetDensityAndMassCenter(mass / scale.x / scale.y / scale.z);

    auto collider = ProjectManager::CreateTemporaryAsset<Collider>();
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
    auto scene = Application::GetActiveScene();
    auto cube = scene->CreateEntity(name);
    auto groundMeshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(cube).lock();
    auto material = ProjectManager::CreateTemporaryAsset<Material>();
    material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer->m_material.Set<Material>(material);
    material->m_materialProperties.m_albedoColor = color;
    groundMeshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Cube);
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), scale);
    // groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    scene->SetDataComponent(cube, groundTransform);

    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), scale);
    scene->SetDataComponent(cube, groundGlobalTransform);
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
    auto scene = Application::GetActiveScene();
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(sphere).lock();
    rigidBody->SetStatic(false);
    rigidBody->SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);
    rigidBody->SetDensityAndMassCenter(mass / scale / scale / scale);

    auto collider = ProjectManager::CreateTemporaryAsset<Collider>();
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
    auto scene = Application::GetActiveScene();
    auto sphere = CreateSphere(color, position, rotation, scale, name);
    auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(sphere).lock();
    rigidBody->SetStatic(true);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody->SetEnabled(true);

    auto collider = ProjectManager::CreateTemporaryAsset<Collider>();
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
    auto scene = Application::GetActiveScene();
    auto sphere = scene->CreateEntity(name);
    auto groundMeshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(sphere).lock();
    auto material = ProjectManager::CreateTemporaryAsset<Material>();
    material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer->m_material.Set<Material>(material);
    material->m_materialProperties.m_albedoColor = color;
    groundMeshRenderer->m_mesh.Set<Mesh>(DefaultResources::Primitives::Sphere);
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    // groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    scene->SetDataComponent(sphere, groundTransform);

    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    scene->SetDataComponent(sphere, groundGlobalTransform);
    return sphere;
}