#include <AssetManager.hpp>
#include <Collider.hpp>
#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include <LightProbe.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsMaterial.hpp>
#include <PostProcessing.hpp>
#include <Prefab.hpp>
#include <ReflectionProbe.hpp>
#include <RenderManager.hpp>
#include <Scene.hpp>
#include <SkinnedMesh.hpp>
#include <Utilities.hpp>
using namespace UniEngine;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_2DToCubemapProgram;

std::unique_ptr<Texture2D> DefaultResources::m_brdfLut;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferInstancedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferInstancedSkinnedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferSkinnedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferLightingPass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightInstancedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::ConvolutionProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::PrefilterProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::BrdfProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoInstancedColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::SkyboxProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::ScreenProgram;
std::shared_ptr<OpenGLUtils::GLVAO> DefaultResources::ScreenVAO;
std::shared_ptr<OpenGLUtils::GLVAO> DefaultResources::SkyboxVAO;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightSkinnedPrePassProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityInstancedSkinnedRecorderProgram;

std::shared_ptr<PhysicsMaterial> DefaultResources::Physics::DefaultPhysicsMaterial;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLShader> DefaultResources::GLShaders::TexturePassThrough;

std::unique_ptr<std::string> DefaultResources::ShaderIncludes::Uniform;

std::shared_ptr<Texture2D> DefaultResources::Textures::MissingTexture;

std::shared_ptr<Mesh> DefaultResources::Primitives::Sphere;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cube;
std::shared_ptr<Mesh> DefaultResources::Primitives::Quad;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cone;
std::shared_ptr<Mesh> DefaultResources::Primitives::Ring;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cylinder;
std::shared_ptr<Mesh> DefaultResources::Primitives::Monkey;

std::shared_ptr<Cubemap> DefaultResources::Environmental::DefaultSkybox;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::DefaultEnvironmentalMap;

Handle DefaultResources::GenerateNewHandle()
{
    return GetInstance().m_currentMaxHandle.m_value++;
}

Handle DefaultResources::GetMaxHandle()
{
    return GetInstance().m_currentMaxHandle;
}

void DefaultResources::LoadShaders()
{
    int numberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);
    for (int i = 0; i < numberOfExtensions; i++)
    {
        const GLubyte *ccc = glGetStringi(GL_EXTENSIONS, i);
    }
#pragma region Shaders
#pragma region Shader Includes
    std::string add;

    add += "\n#define MAX_BONES_AMOUNT " + std::to_string(ShaderIncludes::MaxBonesAmount) +
           "\n#define MAX_TEXTURES_AMOUNT " + std::to_string(ShaderIncludes::MaxMaterialsAmount) +
           "\n#define DIRECTIONAL_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxDirectionalLightAmount) +
           "\n#define POINT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxPointLightAmount) +
           "\n#define SHADOW_CASCADE_AMOUNT " + std::to_string(ShaderIncludes::ShadowCascadeAmount) +
           "\n#define MAX_KERNEL_AMOUNT " + std::to_string(ShaderIncludes::MaxKernelAmount) +
           "\n#define SPOT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxSpotLightAmount) + "\n";

    ShaderIncludes::Uniform = std::make_unique<std::string>(
        add +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Include/Uniform.glsl"));

#pragma endregion

#pragma region Standard Shader

    auto vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Standard.vert");
    auto fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                          FileUtils::LoadFileAsString(
                              std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardForward.frag");

    auto standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "Standard.vert");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardForward.frag");
    standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    GLPrograms::StandardProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard");
    GLPrograms::StandardProgram->Link(standardVert, standardFrag);
    AssetManager::Share(GLPrograms::StandardProgram);
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardSkinned.vert");
    standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardSkinned.vert");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    GLPrograms::StandardSkinnedProgram =
        AssetManager::CreateAsset<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Skinned");
    GLPrograms::StandardSkinnedProgram->Link(standardVert, standardFrag);
    AssetManager::Share(GLPrograms::StandardSkinnedProgram);
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstanced.vert");
    standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardInstanced.vert");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    GLPrograms::StandardInstancedProgram =
        AssetManager::CreateAsset<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Instanced");
    GLPrograms::StandardInstancedProgram->Link(standardVert, standardFrag);
    AssetManager::Share(GLPrograms::StandardInstancedProgram);
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedSkinned.vert");
    standardVert =
        AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardInstancedSkinned.vert");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    GLPrograms::StandardInstancedSkinnedProgram =
        AssetManager::CreateAsset<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Instanced Skinned");
    GLPrograms::StandardInstancedSkinnedProgram->Link(standardVert, standardFrag);
    AssetManager::Share(GLPrograms::StandardInstancedSkinnedProgram);
#pragma endregion
    vertShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/TexturePassThrough.vert");
    GLShaders::TexturePassThrough =
        AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "TexturePassThrough");
    GLShaders::TexturePassThrough->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
#pragma region Post - Processing

    {
        Bloom::m_separateProgram = std::make_shared<OpenGLUtils::GLProgram>();
        fragShaderCode = std::string("#version 450 core\n") +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BloomSeparator.frag");
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "BloomSeparator.frag");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        Bloom::m_separateProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        Bloom::m_filterProgram = std::make_shared<OpenGLUtils::GLProgram>();
        fragShaderCode = std::string("#version 450 core\n") +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BlurFilter.frag");
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        Bloom::m_filterProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        Bloom::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
        fragShaderCode = std::string("#version 450 core\n") +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BloomCombine.frag");
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        Bloom::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSAOGeometry.frag");

        SSAO::m_geometryProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSAO::m_geometryProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BlurFilter.frag");

        SSAO::m_blurProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSAO::m_blurProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSAOCombine.frag");

        SSAO::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSAO::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);


        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("../src/Internals/DefaultResources") / "Shaders/Fragment/SSRReflect.frag");

        SSR::m_reflectProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSR::m_reflectProgram->Link(GLShaders::TexturePassThrough, standardFrag);
        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("../src/Internals/DefaultResources") / "Shaders/Fragment/BlurFilter.frag");

        SSR::m_blurProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSR::m_blurProgram->Link(GLShaders::TexturePassThrough, standardFrag);

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileUtils::LoadFileAsString(
                             std::filesystem::path("../src/Internals/DefaultResources") / "Shaders/Fragment/SSRCombine.frag");

        SSR::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
        standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        SSR::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);
    }
#pragma endregion
#pragma endregion
}

void DefaultResources::LoadPrimitives()
{
#pragma region Models &Primitives

    {
        auto model = AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/quad.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Quad = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Quad");
        Primitives::Quad->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Quad);
    }
    {
        auto model =
            AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/sphere.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Sphere = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Sphere");
        Primitives::Sphere->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Sphere);
    }
    {
        auto model = AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/cube.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Cube = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Cube");
        Primitives::Cube->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Cube);
    }
    {
        auto model = AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/cone.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Cone = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Cone");
        Primitives::Cone->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Cone);
    }
    {
        auto model =
            AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/cylinder.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Cylinder = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Cylinder");
        Primitives::Cylinder->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Cylinder);
    }
    {
        auto model = AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/ring.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Ring = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Ring");
        Primitives::Ring->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Ring);
    }
    {
        auto model =
            AssetManager::Import<Prefab>(std::filesystem::path("./DefaultResources") / "Primitives/monkey.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                        ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>()
                        : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh.Get<Mesh>();
        Primitives::Monkey = AssetManager::CreateAsset<Mesh>(GenerateNewHandle(), "Monkey");
        Primitives::Monkey->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
        AssetManager::Share(Primitives::Monkey);
    }

#pragma endregion
}
void DefaultResources::Load()
{
    LoadShaders();
    LoadPrimitives();

    LoadRenderManagerResources();
    LoadEditorManagerResources();

#pragma region Physics
    Physics::DefaultPhysicsMaterial = AssetManager::CreateAsset<PhysicsMaterial>(GenerateNewHandle(), "Default");
    AssetManager::Share(Physics::DefaultPhysicsMaterial);
#pragma endregion

#pragma region Environmental
    Environmental::DefaultSkybox = AssetManager::CreateAsset<Cubemap>(GenerateNewHandle(), "Default");
    Environmental::DefaultSkybox->Load(
        std::filesystem::path("./DefaultResources") / "Textures/Cubemaps/Walk_Of_Fame/Mans_Outside_Env.hdr");
    Environmental::DefaultSkybox->m_name = "Default";

    Textures::MissingTexture = AssetManager::CreateAsset<Texture2D>(GenerateNewHandle(), "Missing");
    Textures::MissingTexture->Load(std::filesystem::path("./DefaultResources") / "Textures/texture-missing.png");
    Textures::MissingTexture->m_name = "Missing";

    Environmental::DefaultEnvironmentalMap =
        AssetManager::CreateAsset<EnvironmentalMap>(GenerateNewHandle(), "Default");
    Environmental::DefaultEnvironmentalMap->Construct(Environmental::DefaultSkybox);
#pragma endregion
}

void DefaultResources::LoadRenderManagerResources()
{
#pragma region Skybox
    float skyboxVertices[] = {// positions
                              -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
    SkyboxVAO = std::make_shared<OpenGLUtils::GLVAO>();
    SkyboxVAO->SetData(sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    SkyboxVAO->EnableAttributeArray(0);
    SkyboxVAO->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    auto skyboxvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    std::string vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                                 std::string(FileUtils::LoadFileAsString(
                                     std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Skybox.vert"));
    skyboxvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto skyboxfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    std::string fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                                 std::string(FileUtils::LoadFileAsString(
                                     std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Skybox.frag"));
    skyboxfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    SkyboxProgram = std::make_shared<OpenGLUtils::GLProgram>();
    SkyboxProgram->Link(skyboxvert, skyboxfrag);
    SkyboxProgram->SetInt("skybox", 0);
    SkyboxProgram->m_name = "Skybox";
    {
        auto convertCubemapvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
        auto convertCubemapfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EquirectangularMapToCubemap.frag"));
        convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        m_2DToCubemapProgram = std::make_shared<OpenGLUtils::GLProgram>();
        m_2DToCubemapProgram->Link(convertCubemapvert, convertCubemapfrag);
        m_2DToCubemapProgram->SetInt("equirectangularMap", 0);
        m_2DToCubemapProgram->m_name = "EquirectangularMapToCubemap";
    }

#pragma endregion
#pragma region Screen Shader
    float quadVertices[] = {
        // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    ScreenVAO = std::make_shared<OpenGLUtils::GLVAO>();
    ScreenVAO->SetData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    ScreenVAO->EnableAttributeArray(0);
    ScreenVAO->SetAttributePointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    ScreenVAO->EnableAttributeArray(1);
    ScreenVAO->SetAttributePointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    auto screenvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShaderCode = std::string("#version 450 core\n") +
                     std::string(FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Screen.vert"));
    screenvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto screenfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShaderCode = std::string("#version 450 core\n") +
                     std::string(FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Screen.frag"));
    screenfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    ScreenProgram = std::make_shared<OpenGLUtils::GLProgram>();
    ScreenProgram->Link(screenvert, screenfrag);
    ScreenProgram->m_name = "Screen";
#pragma endregion

    vertShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/DirectionalLightShadowMap.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Fragment/DirectionalLightShadowMap.frag");
    std::string geomShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Geometry/DirectionalLightShadowMap.geom");

    auto vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    auto geomShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);
    m_directionalLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
                                                              std::filesystem::path("./DefaultResources") /
                                                              "Shaders/Vertex/DirectionalLightShadowMapInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_directionalLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Vertex/DirectionalLightShadowMapSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_directionalLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") /
                         "Shaders/Vertex/DirectionalLightShadowMapInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_directionalLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/PointLightShadowMap.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Fragment/PointLightShadowMap.frag");
    geomShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Geometry/PointLightShadowMap.geom");

    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    geomShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

    m_pointLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
                                                              std::filesystem::path("./DefaultResources") /
                                                              "Shaders/Vertex/PointLightShadowMapInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_pointLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Vertex/PointLightShadowMapSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_pointLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Vertex/PointLightShadowMapInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_pointLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMap.vert");
    fragShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SpotLightShadowMap.frag");

    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    m_spotLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_spotLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_spotLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightSkinnedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_spotLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightInstancedSkinnedProgram->Link(vertShader, fragShader);

#pragma region GBuffer
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardDeferredLighting.frag");
    fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

    m_gBufferLightingPass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferLightingPass->Link(GLShaders::TexturePassThrough, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Standard.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardDeferred.frag");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    m_gBufferPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferPrepass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_gBufferSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferSkinnedPrepass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_gBufferInstancedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferInstancedPrepass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_gBufferInstancedSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferInstancedSkinnedPrepass->Link(vertShader, fragShader);
#pragma endregion

    {
        auto convertCubemapvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
        auto convertCubemapfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        fragShaderCode =
            std::string("#version 450 core\n") + std::string(FileUtils::LoadFileAsString(
                                                     std::filesystem::path("./DefaultResources") /
                                                     "Shaders/Fragment/EnvironmentalMapIrradianceConvolution.frag"));
        convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        ConvolutionProgram = std::make_shared<OpenGLUtils::GLProgram>();
        ConvolutionProgram->Link(convertCubemapvert, convertCubemapfrag);
        ConvolutionProgram->SetInt("environmentMap", 0);
        ConvolutionProgram->m_name = "EnvironmentalMapIrradianceConvolution";
    }
    {
        auto convertCubemapvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
        auto convertCubemapfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EnvironmentalMapPrefilter.frag"));
        convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        PrefilterProgram = std::make_shared<OpenGLUtils::GLProgram>();
        PrefilterProgram->Link(convertCubemapvert, convertCubemapfrag);
        PrefilterProgram->SetInt("environmentMap", 0);
        PrefilterProgram->m_name = "EnvironmentalMapPrefilter";
    }
    {
        auto convertCubemapvert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        vertShaderCode = std::string("#version 450 core\n") +
                         std::string(FileUtils::LoadFileAsString(
                             std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EnvironmentalMapBrdf.vert"));
        convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
        auto convertCubemapfrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EnvironmentalMapBrdf.frag"));
        convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
        BrdfProgram = std::make_shared<OpenGLUtils::GLProgram>();
        BrdfProgram->Link(convertCubemapvert, convertCubemapfrag);
        BrdfProgram->m_name = "EnvironmentalMapBrdf";
    }

#pragma region Gizmo Shader
    fragShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Gizmo.frag");

    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Gizmo.vert");

    auto standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    GizmoProgram = std::make_shared<OpenGLUtils::GLProgram>();
    GizmoProgram->Attach(standardVert);
    GizmoProgram->Attach(standardFrag);
    GizmoProgram->Link();
    GizmoProgram->m_name = "Gizmo";
    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstanced.vert");

    standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    GizmoInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    GizmoInstancedProgram->Attach(standardVert);
    GizmoInstancedProgram->Attach(standardFrag);
    GizmoInstancedProgram->Link();
    GizmoInstancedProgram->m_name = "Gizmo Instanced";

    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/ColoredGizmos.vert");
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Fragment/ColoredGizmos.frag");

    standardVert = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    standardFrag = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
    GizmoInstancedColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
    GizmoInstancedColoredProgram->Attach(standardVert);
    GizmoInstancedColoredProgram->Attach(standardFrag);
    GizmoInstancedColoredProgram->Link();
    GizmoInstancedColoredProgram->m_name = "Gizmo Instanced Colored";
#pragma endregion

    PrepareBrdfLut();
}

void DefaultResources::PrepareBrdfLut()
{
    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    auto brdfLut = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RG16F, 512, 512, true);
    m_brdfLut = std::make_unique<Texture2D>();
    m_brdfLut->m_texture = std::move(brdfLut);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    size_t resolution = 512;
    auto renderTarget = std::make_shared<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_shared<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
    renderTarget->AttachTexture(m_brdfLut->m_texture.get(), GL_COLOR_ATTACHMENT0);
    renderTarget->GetFrameBuffer()->ViewPort(resolution, resolution);
    DefaultResources::BrdfProgram->Bind();
    renderTarget->Clear();
    RenderManager::RenderQuad();
    OpenGLUtils::GLFrameBuffer::BindDefault();
}
void DefaultResources::LoadEditorManagerResources()
{
#pragma region Recorder
    std::string vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Empty.vert");
    std::string fragShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
                                                                          std::filesystem::path("./DefaultResources") /
                                                                          "Shaders/Fragment/EntityRecorder.frag");

    auto vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    auto fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

    m_sceneCameraEntityRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneCameraEntityInstancedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityInstancedRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptySkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneCameraEntitySkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntitySkinnedRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneCameraEntityInstancedSkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityInstancedSkinnedRecorderProgram->Link(vertShader, fragShader);
#pragma endregion
#pragma region Highlight Prepass
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Empty.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

    fragShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Highlight.frag");

    fragShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

    m_sceneHighlightPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightPrePassInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptySkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightSkinnedPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightSkinnedPrePassProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightPrePassInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassInstancedSkinnedProgram->Link(vertShader, fragShader);
#pragma endregion
#pragma region Highlight
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Highlight.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightInstanced.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightSkinnedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightInstancedSkinned.vert");
    vertShader = AssetManager::CreateAsset<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
    vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
    m_sceneHighlightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightInstancedSkinnedProgram->Link(vertShader, fragShader);
#pragma endregion
}