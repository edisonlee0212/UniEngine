#include <AssetManager.hpp>
#include <Collider.hpp>
#include <Core/FileIO.hpp>
#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include <LightProbe.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <PhysicsMaterial.hpp>
#include <PostProcessing.hpp>
#include <Prefab.hpp>
#include <ReflectionProbe.hpp>
#include <RenderManager.hpp>
#include <SkinnedMesh.hpp>
using namespace UniEngine;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::ConvolutionProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::PrefilterProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::BrdfProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::SkyboxProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::ScreenProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::GizmoProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::GizmoInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::GizmoInstancedColoredProgram;
OpenGLUtils::GLVAO *DefaultResources::GLPrograms::ScreenVAO;
std::shared_ptr<OpenGLUtils::GLVAO> DefaultResources::GLPrograms::SkyboxVAO;
std::string *DefaultResources::ShaderIncludes::Uniform;

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
std::shared_ptr<Cubemap> DefaultResources::Environmental::MilkyWaySkybox;
std::shared_ptr<Cubemap> DefaultResources::Environmental::CircusSkybox;
std::shared_ptr<Cubemap> DefaultResources::Environmental::MilkyWayHDRSkybox;
std::shared_ptr<Cubemap> DefaultResources::Environmental::CircusHDRSkybox;

std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::DefaultEnvironmentalMap;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::MilkyWayEnvironmentalMap;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::CircusEnvironmentalMap;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::MilkyWayHDREnvironmentalMap;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::CircusHDREnvironmentalMap;

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
        add += FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Include/Uniform_BT.glsl"));
    }
    else
    {
        add += FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Include/Uniform_LEGACY.glsl"));
    }

    add += "\n#define MAX_BONES_AMOUNT " + std::to_string(ShaderIncludes::MaxBonesAmount) +
           "\n#define MAX_TEXTURES_AMOUNT " + std::to_string(ShaderIncludes::MaxMaterialsAmount) +
           "\n#define DIRECTIONAL_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxDirectionalLightAmount) +
           "\n#define POINT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxPointLightAmount) +
           "\n#define SHADOW_CASCADE_AMOUNT " + std::to_string(ShaderIncludes::ShadowCascadeAmount) +
           "\n#define MAX_KERNEL_AMOUNT " + std::to_string(ShaderIncludes::MaxKernelAmount) +
           "\n#define SPOT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxSpotLightAmount) + "\n";

    ShaderIncludes::Uniform =
        new std::string(add + FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Include/Uniform.glsl")));

#pragma endregion
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
    GLPrograms::SkyboxVAO = std::make_shared<OpenGLUtils::GLVAO>();
    GLPrograms::SkyboxVAO->SetData(sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    GLPrograms::SkyboxVAO->EnableAttributeArray(0);
    GLPrograms::SkyboxVAO->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);


    auto skyboxvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    std::string vertShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
        std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/Skybox.vert")));
    skyboxvert->Compile(vertShaderCode);
    auto skyboxfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    std::string fragShaderCode =
        std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
        std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/Skybox.frag")));
    skyboxfrag->Compile(fragShaderCode);
    GLPrograms::SkyboxProgram = AssetManager::LoadProgram(false, skyboxvert, skyboxfrag);
    GLPrograms::SkyboxProgram->SetInt("skybox", 0);
    GLPrograms::SkyboxProgram->m_name = "Skybox";
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Vertex/EquirectangularMapToCubemap.vert")));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Fragment/EquirectangularMapToCubemap.frag")));
        convertCubemapfrag->Compile(fragShaderCode);
        AssetManager::GetInstance().m_2DToCubemapProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        AssetManager::GetInstance().m_2DToCubemapProgram->Link(convertCubemapvert, convertCubemapfrag);
        AssetManager::GetInstance().m_2DToCubemapProgram->SetInt("equirectangularMap", 0);
        AssetManager::GetInstance().m_2DToCubemapProgram->m_name = "EquirectangularMapToCubemap";
    }
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Vertex/EquirectangularMapToCubemap.vert")));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Fragment/EnvironmentalMapIrradianceConvolution.frag")));
        convertCubemapfrag->Compile(fragShaderCode);
        GLPrograms::ConvolutionProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        GLPrograms::ConvolutionProgram->Link(convertCubemapvert, convertCubemapfrag);
        GLPrograms::ConvolutionProgram->SetInt("environmentMap", 0);
        GLPrograms::ConvolutionProgram->m_name = "EnvironmentalMapIrradianceConvolution";
    }
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Vertex/EquirectangularMapToCubemap.vert")));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Fragment/EnvironmentalMapPrefilter.frag")));
        convertCubemapfrag->Compile(fragShaderCode);
        GLPrograms::PrefilterProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        GLPrograms::PrefilterProgram->Link(convertCubemapvert, convertCubemapfrag);
        GLPrograms::PrefilterProgram->SetInt("environmentMap", 0);
        GLPrograms::PrefilterProgram->m_name = "EnvironmentalMapPrefilter";
    }
    {
        auto convertCubemapvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
        vertShaderCode =
            std::string("#version 450 core\n") +
            std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/EnvironmentalMapBrdf.vert")));
        convertCubemapvert->Compile(vertShaderCode);
        auto convertCubemapfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
        fragShaderCode = std::string("#version 450 core\n") +
                         std::string(FileIO::LoadFileAsString(
                             FileIO::GetResourcePath("Shaders/Fragment/EnvironmentalMapBrdf.frag")));
        convertCubemapfrag->Compile(fragShaderCode);
        GLPrograms::BrdfProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        GLPrograms::BrdfProgram->Link(convertCubemapvert, convertCubemapfrag);
        GLPrograms::BrdfProgram->m_name = "EnvironmentalMapBrdf";
    }
#pragma endregion
#pragma region Screen Shader
    float quadVertices[] = {
        // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f,  -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f};

    GLPrograms::ScreenVAO = new OpenGLUtils::GLVAO();
    GLPrograms::ScreenVAO->SetData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    GLPrograms::ScreenVAO->EnableAttributeArray(0);
    GLPrograms::ScreenVAO->SetAttributePointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    GLPrograms::ScreenVAO->EnableAttributeArray(1);
    GLPrograms::ScreenVAO->SetAttributePointer(
        1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    auto screenvert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShaderCode = std::string("#version 450 core\n") +
                     std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/Screen.vert")));
    screenvert->Compile(vertShaderCode);
    auto screenfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShaderCode = std::string("#version 450 core\n") +
                     std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/Screen.frag")));
    screenfrag->Compile(fragShaderCode);
    GLPrograms::ScreenProgram = AssetManager::LoadProgram(false, screenvert, screenfrag);
    GLPrograms::ScreenProgram->m_name = "Screen";
#pragma endregion
#pragma region Standard Shader

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/Standard.vert"));
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/StandardForward.frag"));

    auto standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    auto standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardProgram = AssetManager::LoadProgram(true, standardVert, standardFrag);
    GLPrograms::StandardProgram->m_name = "Standard";

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/StandardSkinned.vert"));
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardSkinnedProgram = AssetManager::LoadProgram(true, standardVert, standardFrag);
    GLPrograms::StandardSkinnedProgram->m_name = "Standard Skinned";

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/StandardInstanced.vert"));
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardInstancedProgram = AssetManager::LoadProgram(true, standardVert, standardFrag);
    GLPrograms::StandardInstancedProgram->m_name = "Standard Instanced";

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/StandardInstancedSkinned.vert"));
    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardInstancedSkinnedProgram = AssetManager::LoadProgram(true, standardVert, standardFrag);
    GLPrograms::StandardInstancedSkinnedProgram->m_name = "Standard Instanced Skinned";
#pragma endregion
#pragma region Gizmo Shader
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/Gizmo.frag"));

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/Gizmo.vert"));

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::GizmoProgram = AssetManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::GizmoProgram->m_name = "Gizmo";
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/GizmoInstanced.vert"));

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::GizmoInstancedProgram = AssetManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::GizmoInstancedProgram->m_name = "Gizmo Instanced";

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath() + "Shaders/Vertex/ColoredGizmos.vert");
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath() + "Shaders/Fragment/ColoredGizmos.frag");

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::GizmoInstancedColoredProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
    GLPrograms::GizmoInstancedColoredProgram->Attach(standardVert);
    GLPrograms::GizmoInstancedColoredProgram->Attach(standardFrag);
    GLPrograms::GizmoInstancedColoredProgram->Link();
    GLPrograms::GizmoInstancedColoredProgram->m_name = "Gizmo Instanced Colored";
#pragma endregion
#pragma region Post-Processing

    {
        Bloom::m_separateProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        Bloom::m_separateProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BloomSeparator.frag"))));

        Bloom::m_filterProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        Bloom::m_filterProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BlurFilter.frag"))));
        Bloom::m_combineProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        Bloom::m_combineProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Vertex,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
            std::make_shared<OpenGLUtils::GLShader>(
                OpenGLUtils::ShaderType::Fragment,
                std::string("#version 450 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BloomCombine.frag"))));

        vertShaderCode = std::string("#version 460 core\n") +
                         FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"));

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/SSAOGeometry.frag"));

        SSAO::m_geometryProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        SSAO::m_geometryProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));

        vertShaderCode = std::string("#version 460 core\n") +
                         FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"));

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BlurFilter.frag"));

        SSAO::m_blurProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        SSAO::m_blurProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));

        fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                         FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/SSAOCombine.frag"));

        SSAO::m_combineProgram = AssetManager::CreateResource<OpenGLUtils::GLProgram>();
        SSAO::m_combineProgram->Link(
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex, vertShaderCode),
            std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment, fragShaderCode));

    }
#pragma endregion
#pragma endregion
}

void DefaultResources::LoadTextures()
{
#pragma region Textures
    Textures::MissingTexture =
        AssetManager::LoadTexture(false, FileIO::GetResourcePath("Textures/texture-missing.png"));
    Textures::MissingTexture->m_name = "Missing";

#pragma endregion
}

void DefaultResources::LoadPrimitives()
{
#pragma region Models &Primitives
    if (true)
    {
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/quad.obj"));
            Primitives::Quad = model->RootNode()->m_children[0]->m_mesh
                                   ? model->RootNode()->m_children[0]->m_mesh
                                   : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Quad);
            Primitives::Quad->m_name = "Quad";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/sphere.obj"));
            Primitives::Sphere = model->RootNode()->m_children[0]->m_mesh
                                     ? model->RootNode()->m_children[0]->m_mesh
                                     : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Sphere);
            Primitives::Sphere->m_name = "Sphere";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/cube.obj"));
            Primitives::Cube = model->RootNode()->m_children[0]->m_mesh
                                   ? model->RootNode()->m_children[0]->m_mesh
                                   : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Cube);
            Primitives::Cube->m_name = "Cube";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/cone.obj"));
            Primitives::Cone = model->RootNode()->m_children[0]->m_mesh
                                   ? model->RootNode()->m_children[0]->m_mesh
                                   : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Cone);
            Primitives::Cone->m_name = "Cone";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/cylinder.obj"));
            Primitives::Cylinder = model->RootNode()->m_children[0]->m_mesh
                                       ? model->RootNode()->m_children[0]->m_mesh
                                       : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Cylinder);
            Primitives::Cylinder->m_name = "Cylinder";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/ring.obj"));
            Primitives::Ring = model->RootNode()->m_children[0]->m_mesh
                                   ? model->RootNode()->m_children[0]->m_mesh
                                   : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Ring);
            Primitives::Ring->m_name = "Ring";
        }
        {
            auto model = AssetManager::LoadModel(false, FileIO::GetResourcePath("Primitives/monkey.obj"));
            Primitives::Monkey = model->RootNode()->m_children[0]->m_mesh
                                     ? model->RootNode()->m_children[0]->m_mesh
                                     : model->RootNode()->m_children[0]->m_children[0]->m_mesh;
            AssetManager::Push(Primitives::Monkey);
            Primitives::Monkey->m_name = "Monkey";
        }
    }
#pragma endregion
}
void DefaultResources::Load()
{
    AssetManager::RegisterResourceType<Material>("Material");
    AssetManager::RegisterResourceType<Mesh>("Mesh");
    AssetManager::RegisterResourceType<Texture2D>("Texture2D");
    AssetManager::RegisterResourceType<Cubemap>("Cubemap");
    AssetManager::RegisterResourceType<Model>("Model");
    AssetManager::RegisterResourceType<LightProbe>("LightProbe");
    AssetManager::RegisterResourceType<ReflectionProbe>("ReflectionProbe");
    AssetManager::RegisterResourceType<OpenGLUtils::GLProgram>("GLProgram");
    AssetManager::RegisterResourceType<EnvironmentalMap>("EnvironmentalMap");
    AssetManager::RegisterResourceType<Animation>("Animation");
    AssetManager::RegisterResourceType<SkinnedMesh>("SkinnedMesh");
    AssetManager::RegisterResourceType<PhysicsMaterial>("PhysicsMaterial");
    AssetManager::RegisterResourceType<Collider>("Collider");
    AssetManager::RegisterResourceType<Prefab>("Prefab");
    LoadShaders();
    LoadTextures();
    LoadPrimitives();
#pragma region Environmental
    Materials::StandardMaterial = AssetManager::LoadMaterial(true, GLPrograms::StandardProgram);
    Materials::StandardMaterial->m_name = "Standard";

    Materials::StandardInstancedMaterial = AssetManager::LoadMaterial(true, GLPrograms::StandardInstancedProgram);
    Materials::StandardInstancedMaterial->m_name = "Standard Instanced";

    Environmental::DefaultSkybox =
        AssetManager::LoadCubemap(
        true, FileIO::GetResourcePath("Textures/Cubemaps/Walk_Of_Fame/Mans_Outside_Env.hdr"));
    Environmental::DefaultSkybox->m_name = "Default";
    Environmental::DefaultEnvironmentalMap = AssetManager::CreateResource<EnvironmentalMap>(true, "Default");
    Environmental::DefaultEnvironmentalMap->Construct(Environmental::DefaultSkybox);

    /*
    Environmental::MilkyWaySkybox =
        AssetManager::LoadCubemap(true, FileIO::GetResourcePath("Textures/Cubemaps/Milkyway/Milkyway_BG.jpg"));
    Environmental::MilkyWaySkybox->m_name = "Milky Way";
    Environmental::MilkyWayHDRSkybox =
        AssetManager::LoadCubemap(true, FileIO::GetResourcePath("Textures/Cubemaps/Milkyway/Milkyway_small.hdr"));
    Environmental::MilkyWayHDRSkybox->m_name = "Milky Way HDR";
    Environmental::MilkyWayEnvironmentalMap = AssetManager::CreateResource<EnvironmentalMap>(true, "Milky Way");
    Environmental::MilkyWayEnvironmentalMap->Construct(Environmental::MilkyWaySkybox);
    Environmental::MilkyWayHDREnvironmentalMap = AssetManager::CreateResource<EnvironmentalMap>(true, "Milky Way
    HDR"); Environmental::MilkyWayHDREnvironmentalMap->Construct(Environmental::MilkyWayHDRSkybox);
    Environmental::CircusEnvironmentalMap = AssetManager::LoadEnvironmentalMap(
        true, FileIO::GetResourcePath("Textures/Cubemaps/Circus/Circus_Backstage_8k.jpg"));
    Environmental::CircusEnvironmentalMap->m_name = "Circus";
    Environmental::CircusHDREnvironmentalMap = AssetManager::LoadEnvironmentalMap(
        true, FileIO::GetResourcePath("Textures/Cubemaps/Circus/Circus_Backstage_3k.hdr"));
    Environmental::CircusHDREnvironmentalMap->m_name = "Circus HDR";
    */
#pragma endregion
}
