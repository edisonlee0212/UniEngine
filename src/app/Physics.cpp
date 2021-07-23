#include "AssetManager.hpp"
#include <Application.hpp>
#include <CameraControlSystem.hpp>
#include <Collider.hpp>
#include <Joint.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;

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

int main()
{
    Application::Init();
    auto ccs = EntityManager::GetOrCreateSystem<CameraControlSystem>("CameraControlSystem", SystemGroup::SimulationSystemGroup);

    auto mainCameraEntity = RenderManager::GetMainCamera()->GetOwner();
    auto mainCameraTransform = mainCameraEntity.GetDataComponent<Transform>();
    mainCameraTransform.SetPosition(glm::vec3(0, -4, 25));
    mainCameraEntity.SetDataComponent(mainCameraTransform);
    auto &postProcessing = mainCameraEntity.SetPrivateComponent<PostProcessing>();
    postProcessing.GetLayer<Bloom>()->m_intensity = 0.03;
    postProcessing.GetLayer<Bloom>()->m_diffusion = 16;

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
            Transform transform;
            glm::vec3 position = glm::vec3(i - amount / 2.0f, j - amount / 2.0f, 0);
            position += glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f)) * scaleFactor;
            transform.SetPosition(position * 5.0f * scaleFactor);
            transform.SetScale(glm::vec3(2.0f * scaleFactor));
            sphere.SetDataComponent(transform);
            auto &meshRenderer = sphere.SetPrivateComponent<MeshRenderer>();
            meshRenderer.m_mesh = DefaultResources::Primitives::Sphere;
            meshRenderer.m_material = AssetManager::CreateResource<Material>();
            meshRenderer.m_material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer.m_material->m_roughness = static_cast<float>(i) / (amount - 1);
            meshRenderer.m_material->m_metallic = static_cast<float>(j) / (amount - 1);

            auto &rigidBody = sphere.SetPrivateComponent<RigidBody>();
            rigidBody.SetEnabled(true);
            rigidBody.SetDensityAndMassCenter(0.1);
            auto sphereCollider = AssetManager::CreateResource<Collider>();
            sphereCollider->SetShapeType(ShapeType::Sphere);
            sphereCollider->SetShapeParam(glm::vec3(2.0 * scaleFactor));
            rigidBody.AttachCollider(sphereCollider);
            sphere.SetParent(collection);
        }
    }
#pragma endregion

#pragma region Lighting
    const auto dirLightEntity = EntityManager::CreateEntity("Dir Light");
    auto &dirLight = dirLightEntity.SetPrivateComponent<DirectionalLight>();
    dirLight.m_diffuseBrightness = 3.0f;
    dirLight.m_lightSize = 0.2f;
    Transform dirLightTransform;
    dirLightTransform.SetEulerRotation(glm::radians(glm::vec3(100, 0, 0)));
    dirLightEntity.SetDataComponent(dirLightTransform);

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

        auto &b1j = b1.SetPrivateComponent<Joint>();
        b1j.SetType(JointType::Fixed);
        b1j.Link(b2);
        auto &b3j = b3.SetPrivateComponent<Joint>();
        b3j.SetType(JointType::Fixed);
        b3j.Link(b2);
        auto &b2j = b2.SetPrivateComponent<Joint>();
        b2j.SetType(JointType::Fixed);
        b2j.Link(ground);

        const auto anchor = CreateDynamicCube(
            1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.2f), "Anchor");
        anchor.GetPrivateComponent<RigidBody>().SetKinematic(true);
        auto lastLink = anchor;
        for (int i = 1; i < 10; i++)
        {
            const auto link = CreateDynamicSphere(
                1.0, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-10 - i, 0, 0), glm::vec3(0, 0, 45), 0.2f, "Link");
            auto &joint = link.SetPrivateComponent<Joint>();
            joint.SetType(JointType::D6);
            joint.Link(lastLink);
            //joint.SetMotion(MotionAxis::SwingY, MotionType::Limited);
            link.SetParent(anchor);
            lastLink = link;
        }

        const auto freeSphere = CreateDynamicCube(
            0.01, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-20, 0, 0), glm::vec3(0, 0, 45), glm::vec3(0.5), "Free Cube");
        auto &joint = freeSphere.SetPrivateComponent<Joint>();
        joint.SetType(JointType::Spherical);
        joint.Link(lastLink);
    }
#pragma endregion

#pragma region Heart shaped rings
    if (false)
    {
        const auto height = 0;
        const auto leftSphere = CreateSolidSphere(
            1.0, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-2.7, height + 2, -10), glm::vec3(0, 0, 45), 1.7, "Block 1");
        const auto rightSphere = CreateSolidSphere(
            1.0, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(2.7, height + 2, -10), glm::vec3(0, 0, 45), 1.7, "Block 3");

        float radius = 3.0f;
        float factor = 1.5f;
        const auto anchor = CreateDynamicSphere(
            8.0, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, height, -10), glm::vec3(0, 0, 45), 0.28, "Start");
        auto lastLink = anchor;
        int amount = 72;
        for (int i = 1; i < amount; i++)
        {
            float mass = i == amount / 2 ? 1 : 1;
            const auto position = glm::vec3(
                glm::sin(glm::radians(i * 360.0f / amount)) * radius * factor,
                -glm::cos(glm::radians(i * 360.0f / amount)) * radius + height + radius,
                -10);
            const auto link =
                CreateDynamicSphere(mass, glm::vec3(1.0f, 1.0f, 1.0f), position, glm::vec3(0, 0, 45), 0.1f, "Link");
            auto &joint = link.SetPrivateComponent<Joint>();
            joint.SetType(JointType::Spherical);
            joint.Link(lastLink);
            link.GetPrivateComponent<MeshRenderer>().m_material->m_ambient = 2.0f;
            link.GetPrivateComponent<MeshRenderer>().m_material->m_roughness = 0.0f;
            link.GetPrivateComponent<MeshRenderer>().m_material->m_metallic = 0.0f;
            link.SetParent(anchor);
            lastLink = link;
        }
        auto &joint = anchor.SetPrivateComponent<Joint>();
        joint.SetType(JointType::Spherical);
        joint.Link(lastLink);
        anchor.GetPrivateComponent<MeshRenderer>().m_material->m_ambient = 3.0f;
        anchor.GetPrivateComponent<MeshRenderer>().m_material->m_roughness = 0.0f;
        anchor.GetPrivateComponent<MeshRenderer>().m_material->m_metallic = 0.0f;
    }
#pragma endregion

    // Start engine. Here since we need to inject procedures to the main engine loop we need to manually loop by our
    // self. Another way to run engine is to simply execute:
    Application::Run();

    Application::End();
#pragma endregion
    return 0;
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
    auto &rigidBody = cube.SetPrivateComponent<RigidBody>();
    rigidBody.SetStatic(true);
    rigidBody.SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody.SetEnabled(true);
    rigidBody.SetDensityAndMassCenter(mass / scale.x / scale.y / scale.z);

    auto collider = AssetManager::CreateResource<Collider>();
    collider->SetShapeType(ShapeType::Box);
    collider->SetShapeParam(scale);
    rigidBody.AttachCollider(collider);
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
    auto &rigidBody = cube.SetPrivateComponent<RigidBody>();
    rigidBody.SetStatic(false);
    rigidBody.SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody.SetEnabled(true);
    rigidBody.SetDensityAndMassCenter(mass / scale.x / scale.y / scale.z);

    auto collider = AssetManager::CreateResource<Collider>();
    collider->SetShapeType(ShapeType::Box);
    collider->SetShapeParam(scale);
    rigidBody.AttachCollider(collider);
    return cube;
}

Entity CreateCube(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const glm::vec3 &scale,
    const std::string &name)
{
    auto cube = EntityManager::CreateEntity(name);
    auto &groundMeshRenderer = cube.SetPrivateComponent<MeshRenderer>();
    groundMeshRenderer.m_material = AssetManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer.m_material->m_albedoColor = color;
    groundMeshRenderer.m_mesh = DefaultResources::Primitives::Cube;
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
    auto &rigidBody = sphere.SetPrivateComponent<RigidBody>();
    rigidBody.SetStatic(false);
    rigidBody.SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody.SetEnabled(true);
    rigidBody.SetDensityAndMassCenter(mass / scale / scale / scale);

    auto collider = AssetManager::CreateResource<Collider>();
    collider->SetShapeType(ShapeType::Sphere);
    collider->SetShapeParam(glm::vec3(scale));
    rigidBody.AttachCollider(collider);
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
    auto &rigidBody = sphere.SetPrivateComponent<RigidBody>();
    rigidBody.SetStatic(true);
    rigidBody.SetDensityAndMassCenter(1);
    // The rigidbody can only apply mesh bound after it's attached to an entity with mesh renderer.
    rigidBody.SetEnabled(true);
    rigidBody.SetDensityAndMassCenter(mass / scale / scale / scale);

    auto collider = AssetManager::CreateResource<Collider>();
    collider->SetShapeType(ShapeType::Sphere);
    collider->SetShapeParam(glm::vec3(scale));
    rigidBody.AttachCollider(collider);
    return sphere;
}

Entity CreateSphere(
    const glm::vec3 color,
    const glm::vec3 &position,
    const glm::vec3 &rotation,
    const float &scale,
    const std::string &name)
{
    auto sphere = EntityManager::CreateEntity(name);
    auto &groundMeshRenderer = sphere.SetPrivateComponent<MeshRenderer>();
    groundMeshRenderer.m_material = AssetManager::LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
    groundMeshRenderer.m_material->m_albedoColor = color;
    groundMeshRenderer.m_mesh = DefaultResources::Primitives::Sphere;
    Transform groundTransform;
    groundTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    // groundTransform.SetValue(glm::vec3(0, -15, 0), glm::vec3(0), glm::vec3(30, 1, 30));
    sphere.SetDataComponent(groundTransform);

    GlobalTransform groundGlobalTransform;
    groundGlobalTransform.SetValue(position, glm::radians(rotation), glm::vec3(scale));
    sphere.SetDataComponent(groundGlobalTransform);
    return sphere;
}