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

std::unique_ptr<std::string> DefaultResources::ShaderIncludes::Uniform;

std::shared_ptr<Texture2D> DefaultResources::Textures::MissingTexture;

std::shared_ptr<Mesh> DefaultResources::Primitives::Sphere;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cube;
std::shared_ptr<Mesh> DefaultResources::Primitives::Quad;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cone;
std::shared_ptr<Mesh> DefaultResources::Primitives::Ring;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cylinder;
std::shared_ptr<Mesh> DefaultResources::Primitives::Monkey;

std::shared_ptr<Material> DefaultResources::Materials::StandardMaterial;
std::shared_ptr<Material> DefaultResources::Materials::StandardInstancedMaterial;

std::shared_ptr<Cubemap> DefaultResources::Environmental::DefaultSkybox;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::DefaultEnvironmentalMap;
void DefaultResources::LoadShaders()
{
    int numberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);
    for (int i = 0; i < numberOfExtensions; i++)
    {
        const GLubyte *ccc = glGetStringi(GL_EXTENSIONS, i);
        if (strcmp((char *)ccc, "GL_ARB_bindless_texture") == 0)
        {
            OpenGLUtils::GetInstance().m_enableBindlessTexture = true;
        }
    }
#pragma region Shaders
#pragma region Shader Includes
    std::string add;
    if (OpenGLUtils::GetInstance().m_enableBindlessTexture)
    {
        add += FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Include/Uniform_BT.glsl");
    }
    else
    {
        add +=
            FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Include/Uniform_LEGACY.glsl");
    }

    add += "\n#define MAX_BONES_AMOUNT " + std::to_string(ShaderIncludes::MaxBonesAmount) +
           "\n#define MAX_TEXTURES_AMOUNT " + std::to_string(ShaderIncludes::MaxMaterialsAmount) +
           "\n#define DIRECTIONAL_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxDirectionalLightAmount) +
           "\n#define POINT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxPointLightAmount) +
           "\n#define SHADOW_CASCADE_AMOUNT " + std::to_string(ShaderIncludes::ShadowCascadeAmount) +
           "\n#define MAX_KERNEL_AMOUNT " + std::to_string(ShaderIncludes::MaxKernelAmount) +
           "\n#define SPOT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxSpotLightAmount) + "\n";

    ShaderIncludes::Uniform = std::make_unique<std::string>(
        add + FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Include/Uniform.glsl"));

#pragma endregion

#pragma region Standard Shader

    auto vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Standard.vert");
    auto fragShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/StandardForward.frag");

    auto standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    auto standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>(Handle(0), "Standard");
    GLPrograms::StandardProgram->Link(standardVert, standardFrag);

    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardSkinned.vert");
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>(Handle(0), "Standard Skinned");
    GLPrograms::StandardSkinnedProgram->Link(standardVert, standardFrag);

    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardInstanced.vert");
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardInstancedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>(Handle(0), "Standard Instanced");
    GLPrograms::StandardInstancedProgram->Link(standardVert, standardFrag);

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardInstancedSkinned.vert");
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardInstancedSkinnedProgram =
        AssetManager::CreateAsset<OpenGLUtils::GLProgram>(Handle(0), "Standard Instanced Skinned");
    GLPrograms::StandardInstancedSkinnedProgram->Link(standardVert, standardFrag);
#pragma endregion

#pragma region Post - Processing

    {
        Bloom::m_separateProgram = std::make_shared<OpenGLUtils::GLProgram>();
        Bloom::m_separateProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert")),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Fragment/BloomSeparator.frag")));

        Bloom::m_filterProgram = std::make_shared<OpenGLUtils::GLProgram>();
        Bloom::m_filterProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert")),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Fragment/BlurFilter.frag")));
        Bloom::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
        Bloom::m_combineProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert")),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                    FileUtils::LoadFileAsString(
                        AssetManager::GetResourceFolderPath() / "Shaders/Fragment/BloomCombine.frag")));

        vertShaderCode = std::string("#version 460 core\n") +
                         FileUtils::LoadFileAsString(
                             AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert");

        fragShaderCode =
            std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
            FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/SSAOGeometry.frag");

        SSAO::m_geometryProgram = std::make_shared<OpenGLUtils::GLProgram>();
        SSAO::m_geometryProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));

        vertShaderCode = std::string("#version 460 core\n") +
                         FileUtils::LoadFileAsString(
                             AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert");

        fragShaderCode =
            std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
            FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/BlurFilter.frag");

        SSAO::m_blurProgram = std::make_shared<OpenGLUtils::GLProgram>();
        SSAO::m_blurProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));

        fragShaderCode =
            std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
            FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/SSAOCombine.frag");

        SSAO::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
        SSAO::m_combineProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));
    }
#pragma endregion
#pragma endregion
}

void DefaultResources::LoadPrimitives()
{
#pragma region Models &Primitives

    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/quad.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
            ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
            : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Quad = AssetManager::CreateAsset<Mesh>(Handle(0), "Quad");
        Primitives::Quad->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/sphere.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                                 ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                                 : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Sphere = AssetManager::CreateAsset<Mesh>(Handle(0), "Sphere");
        Primitives::Sphere->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/cube.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                               ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                               : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Cube = AssetManager::CreateAsset<Mesh>(Handle(0), "Cube");
        Primitives::Cube->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/cone.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                               ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                               : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Cone = AssetManager::CreateAsset<Mesh>(Handle(0), "Cone");
        Primitives::Cone->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/cylinder.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                                   ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                                   : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Cylinder = AssetManager::CreateAsset<Mesh>(Handle(0), "Cylinder");
        Primitives::Cylinder->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/ring.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                               ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                               : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Ring = AssetManager::CreateAsset<Mesh>(Handle(0), "Ring");
        Primitives::Ring->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }
    {
        auto model = AssetManager::Load<Prefab>(AssetManager::GetResourceFolderPath() / "Primitives/monkey.obj");
        auto mesh = model->m_children[0]->GetPrivateComponent<MeshRenderer>().get()
                                 ? model->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh
                                 : model->m_children[0]->m_children[0]->GetPrivateComponent<MeshRenderer>()->m_mesh;
        Primitives::Monkey = AssetManager::CreateAsset<Mesh>(Handle(0), "Monkey");
        Primitives::Monkey->SetVertices(19, mesh->UnsafeGetVertices(), mesh->UnsafeGetTriangles());
    }

#pragma endregion
}
void DefaultResources::Load()
{
    AssetManager::RegisterAssetType<Material>("Material");
    AssetManager::RegisterAssetType<Mesh>("Mesh");
    AssetManager::RegisterAssetType<Texture2D>("Texture2D");
    AssetManager::RegisterAssetType<Cubemap>("Cubemap");
    AssetManager::RegisterAssetType<LightProbe>("LightProbe");
    AssetManager::RegisterAssetType<ReflectionProbe>("ReflectionProbe");
    AssetManager::RegisterAssetType<OpenGLUtils::GLProgram>("GLProgram");
    AssetManager::RegisterAssetType<EnvironmentalMap>("EnvironmentalMap");
    AssetManager::RegisterAssetType<Animation>("Animation");
    AssetManager::RegisterAssetType<SkinnedMesh>("SkinnedMesh");
    AssetManager::RegisterAssetType<PhysicsMaterial>("PhysicsMaterial");
    AssetManager::RegisterAssetType<Collider>("Collider");
    AssetManager::RegisterAssetType<Prefab>("Prefab");
    AssetManager::RegisterAssetType<Scene>("Scene");

    LoadShaders();
    LoadPrimitives();

    LoadRenderManagerResources();
    LoadEditorManagerResources();

#pragma region Physics
    Physics::DefaultPhysicsMaterial = AssetManager::CreateAsset<PhysicsMaterial>(Handle(0), "Default");
#pragma endregion

#pragma region Environmental
    Materials::StandardMaterial = AssetManager::CreateAsset<Material>(Handle(0), "Standard");
    Materials::StandardMaterial->m_program = GLPrograms::StandardProgram;

    Materials::StandardInstancedMaterial = AssetManager::CreateAsset<Material>(Handle(0), "Standard Instanced");
    Materials::StandardInstancedMaterial->m_program = GLPrograms::StandardInstancedProgram;

    Environmental::DefaultSkybox = AssetManager::CreateAsset<Cubemap>(Handle(0), "Default");
    Environmental::DefaultSkybox->Load(
        AssetManager::GetResourceFolderPath() / "Textures/Cubemaps/Walk_Of_Fame/Mans_Outside_Env.hdr");
    Environmental::DefaultSkybox->m_name = "Default";


    Textures::MissingTexture = AssetManager::CreateAsset<Texture2D>(Handle(0), "Missing");
    Textures::MissingTexture->Load(AssetManager::GetResourceFolderPath() / "Textures/texture-missing.png");
    Textures::MissingTexture->m_name = "Missing";

    Environmental::DefaultEnvironmentalMap = AssetManager::CreateAsset<EnvironmentalMap>(Handle(0), "Default");
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

    auto skyboxvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    std::string vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        std::string(FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Skybox.vert"));
    skyboxvert->Compile(vertShaderCode);
    auto skyboxfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    std::string fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                                 std::string(FileUtils::LoadFileAsString(
                                     AssetManager::GetResourceFolderPath() / "Shaders/Fragment/Skybox.frag"));
    skyboxfrag->Compile(fragShaderCode);
    SkyboxProgram = std::make_shared<OpenGLUtils::GLProgram>();
    SkyboxProgram->Link(skyboxvert, skyboxfrag);
    SkyboxProgram->SetInt("skybox", 0);
    SkyboxProgram->m_name = "Skybox";
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Fragment/EquirectangularMapToCubemap.frag"));
        convertCubemapfrag->Compile(fragShaderCode);
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

    auto screenvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShaderCode =
        std::string("#version 450 core\n") +
        std::string(FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Screen.vert"));
    screenvert->Compile(vertShaderCode);
    auto screenfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShaderCode = std::string("#version 450 core\n") +
                     std::string(FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Fragment/Screen.frag"));
    screenfrag->Compile(fragShaderCode);
    ScreenProgram = std::make_shared<OpenGLUtils::GLProgram>();
    ScreenProgram->Link(screenvert, screenfrag);
    ScreenProgram->m_name = "Screen";
#pragma endregion

    vertShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/DirectionalLightShadowMap.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Fragment/DirectionalLightShadowMap.frag");
    std::string geomShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            AssetManager::GetResourceFolderPath() / "Shaders/Geometry/DirectionalLightShadowMap.geom");

    auto vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    auto fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    auto geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);
    m_directionalLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
                                                              AssetManager::GetResourceFolderPath() /
                                                              "Shaders/Vertex/DirectionalLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_directionalLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            AssetManager::GetResourceFolderPath() / "Shaders/Vertex/DirectionalLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_directionalLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            AssetManager::GetResourceFolderPath() / "Shaders/Vertex/DirectionalLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_directionalLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_directionalLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/PointLightShadowMap.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Fragment/PointLightShadowMap.frag");
    geomShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Geometry/PointLightShadowMap.geom");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);

    m_pointLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/PointLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_pointLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/PointLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_pointLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            AssetManager::GetResourceFolderPath() / "Shaders/Vertex/PointLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_pointLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_pointLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/SpotLightShadowMap.vert");
    fragShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/SpotLightShadowMap.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    m_spotLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/SpotLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_spotLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/SpotLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_spotLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightSkinnedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(
            AssetManager::GetResourceFolderPath() / "Shaders/Vertex/SpotLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_spotLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_spotLightInstancedSkinnedProgram->Link(vertShader, fragShader);

#pragma region GBuffer
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Fragment/StandardDeferredLighting.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    m_gBufferLightingPass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferLightingPass->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Standard.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/StandardDeferred.frag");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    m_gBufferPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferPrepass->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_gBufferSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferSkinnedPrepass->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_gBufferInstancedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferInstancedPrepass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/StandardInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_gBufferInstancedSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
    m_gBufferInstancedSkinnedPrepass->Link(vertShader, fragShader);
#pragma endregion
#pragma region SSAO
    vertShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/TexturePassThrough.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/SSAOGeometry.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

#pragma endregion

    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Fragment/EnvironmentalMapIrradianceConvolution.frag"));
        convertCubemapfrag->Compile(fragShaderCode);
        ConvolutionProgram = std::make_shared<OpenGLUtils::GLProgram>();
        ConvolutionProgram->Link(convertCubemapvert, convertCubemapfrag);
        ConvolutionProgram->SetInt("environmentMap", 0);
        ConvolutionProgram->m_name = "EnvironmentalMapIrradianceConvolution";
    }
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileUtils::LoadFileAsString(
                AssetManager::GetResourceFolderPath() / "Shaders/Fragment/EnvironmentalMapPrefilter.frag"));
        convertCubemapfrag->Compile(fragShaderCode);
        PrefilterProgram = std::make_shared<OpenGLUtils::GLProgram>();
        PrefilterProgram->Link(convertCubemapvert, convertCubemapfrag);
        PrefilterProgram->SetInt("environmentMap", 0);
        PrefilterProgram->m_name = "EnvironmentalMapPrefilter";
    }
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode = std::string("#version 450 core\n") +
                         std::string(FileUtils::LoadFileAsString(
                             AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EnvironmentalMapBrdf.vert"));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode = std::string("#version 450 core\n") +
                         std::string(FileUtils::LoadFileAsString(
                             AssetManager::GetResourceFolderPath() / "Shaders/Fragment/EnvironmentalMapBrdf.frag"));
        convertCubemapfrag->Compile(fragShaderCode);
        BrdfProgram = std::make_shared<OpenGLUtils::GLProgram>();
        BrdfProgram->Link(convertCubemapvert, convertCubemapfrag);
        BrdfProgram->m_name = "EnvironmentalMapBrdf";
    }

#pragma region Gizmo Shader
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/Gizmo.frag");

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Gizmo.vert");

    auto standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    auto standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GizmoProgram = std::make_shared<OpenGLUtils::GLProgram>();
    GizmoProgram->Attach(standardVert);
    GizmoProgram->Attach(standardFrag);
    GizmoProgram->Link();
    GizmoProgram->m_name = "Gizmo";
    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/GizmoInstanced.vert");

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GizmoInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    GizmoInstancedProgram->Attach(standardVert);
    GizmoInstancedProgram->Attach(standardFrag);
    GizmoInstancedProgram->Link();
    GizmoInstancedProgram->m_name = "Gizmo Instanced";

    vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/ColoredGizmos.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/ColoredGizmos.frag");

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
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
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Empty.vert");
    std::string fragShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/EntityRecorder.frag");

    auto vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    auto fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    m_sceneCameraEntityRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptyInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneCameraEntityInstancedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityInstancedRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptySkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneCameraEntitySkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntitySkinnedRecorderProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptyInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneCameraEntityInstancedSkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneCameraEntityInstancedSkinnedRecorderProgram->Link(vertShader, fragShader);
#pragma endregion
#pragma region Highlight Prepass
    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Empty.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);

    fragShaderCode =
        std::string("#version 450 core\n") +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Fragment/Highlight.frag");

    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    m_sceneHighlightPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptyInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightPrePassInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptySkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightSkinnedPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightSkinnedPrePassProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/EmptyInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightPrePassInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightPrePassInstancedSkinnedProgram->Link(vertShader, fragShader);
#pragma endregion
#pragma region Highlight
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/Highlight.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/HighlightInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileUtils::LoadFileAsString(AssetManager::GetResourceFolderPath() / "Shaders/Vertex/HighlightSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightSkinnedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileUtils::LoadFileAsString(
                         AssetManager::GetResourceFolderPath() / "Shaders/Vertex/HighlightInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    m_sceneHighlightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
    m_sceneHighlightInstancedSkinnedProgram->Link(vertShader, fragShader);
#pragma endregion
}