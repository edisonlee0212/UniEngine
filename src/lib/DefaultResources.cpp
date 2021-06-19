#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include <FileIO.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <ResourceManager.hpp>
using namespace UniEngine;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::SkyboxProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::BackGroundProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::ScreenProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::StandardProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::StandardInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::GizmoProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::GizmoInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Default::GLPrograms::GizmoInstancedColoredProgram;
OpenGLUtils::GLVAO *Default::GLPrograms::ScreenVAO;
std::shared_ptr<OpenGLUtils::GLVAO> Default::GLPrograms::SkyboxVAO;
std::string *Default::ShaderIncludes::Uniform;

std::shared_ptr<Texture2D> Default::Textures::MissingTexture;
std::shared_ptr<Texture2D> Default::Textures::UV;
std::shared_ptr<Texture2D> Default::Textures::ObjectIcon;
std::shared_ptr<Texture2D> Default::Textures::Border;
std::shared_ptr<Texture2D> Default::Textures::StandardTexture;
std::shared_ptr<Cubemap> Default::Textures::DefaultSkybox;

std::shared_ptr<Mesh> Default::Primitives::Sphere;
std::shared_ptr<Mesh> Default::Primitives::Cube;
std::shared_ptr<Mesh> Default::Primitives::Quad;
std::shared_ptr<Mesh> Default::Primitives::Cone;
std::shared_ptr<Mesh> Default::Primitives::Ring;
std::shared_ptr<Mesh> Default::Primitives::Cylinder;
std::shared_ptr<Mesh> Default::Primitives::Monkey;

std::shared_ptr<Material> Default::Materials::StandardMaterial;
std::shared_ptr<Material> Default::Materials::StandardInstancedMaterial;

void Default::Load(World *world)
{
#pragma region Shader Includes
    std::string add = std::string("#extension GL_ARB_bindless_texture : require\n") + 
                      "#define MAX_TEXTURES_AMOUNT " + std::to_string(ShaderIncludes::MaxMaterialsAmount) +
                      "\n#define DIRECTIONAL_LIGHTS_AMOUNT " +
                      std::to_string(ShaderIncludes::MaxDirectionalLightAmount) + "\n#define POINT_LIGHTS_AMOUNT " +
                      std::to_string(ShaderIncludes::MaxPointLightAmount) + "\n#define SHADOW_CASCADE_AMOUNT " +
                      std::to_string(ShaderIncludes::ShadowCascadeAmount) + "\n#define MAX_KERNEL_AMOUNT " +
                      std::to_string(ShaderIncludes::MaxKernelAmount) + "\n#define SPOT_LIGHTS_AMOUNT " +
                      std::to_string(ShaderIncludes::MaxSpotLightAmount) + "\n";

    ShaderIncludes::Uniform =
        new std::string(add + FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Include/Uniform.inc")));

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

    const std::vector<std::string> facesPath{
        FileIO::GetResourcePath("Textures/Skyboxes/Default/posx.jpg"),
        FileIO::GetResourcePath("Textures/Skyboxes/Default/negx.jpg"),
        FileIO::GetResourcePath("Textures/Skyboxes/Default/posy.jpg"),
        FileIO::GetResourcePath("Textures/Skyboxes/Default/negy.jpg"),
        FileIO::GetResourcePath("Textures/Skyboxes/Default/posz.jpg"),
        FileIO::GetResourcePath("Textures/Skyboxes/Default/negz.jpg"),
    };

    Textures::DefaultSkybox = ResourceManager::LoadCubemap(true, facesPath);
    Textures::DefaultSkybox->m_name = "Default";

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
    GLPrograms::SkyboxProgram = ResourceManager::LoadProgram(false, skyboxvert, skyboxfrag);
    GLPrograms::SkyboxProgram->SetInt("skybox", 0);
    GLPrograms::SkyboxProgram->m_name = "Skybox";
    skyboxfrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     std::string(FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BackGround.frag")));
    skyboxfrag->Compile(fragShaderCode);
    GLPrograms::BackGroundProgram = ResourceManager::LoadProgram(false, skyboxvert, skyboxfrag);
    GLPrograms::BackGroundProgram->m_name = "Background";
#pragma endregion

#pragma region Screen Shader
    float quadVertices[] = {
        // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

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
    GLPrograms::ScreenProgram = ResourceManager::LoadProgram(false, screenvert, screenfrag);
    GLPrograms::ScreenProgram->m_name = "Screen";
#pragma endregion

#pragma region Textures
    Textures::MissingTexture =
        ResourceManager::LoadTexture(false, FileIO::GetResourcePath("Textures/texture-missing.png"));
    Textures::MissingTexture->m_name = "Missing";
    Textures::UV = ResourceManager::LoadTexture(true, FileIO::GetResourcePath("Textures/uv-test.png"));
    Textures::UV->m_name = "UV";
    Textures::StandardTexture = ResourceManager::LoadTexture(true, FileIO::GetResourcePath("Textures/white.png"));
    Textures::StandardTexture->m_name = "Default";
    Textures::ObjectIcon = ResourceManager::LoadTexture(false, FileIO::GetResourcePath("Textures/object.png"));
    Textures::ObjectIcon->m_name = "Icon";
    Textures::Border = ResourceManager::LoadTexture(false, FileIO::GetResourcePath("Textures/border.png"));
    Textures::Border->m_name = "Border";
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
    GLPrograms::StandardProgram = ResourceManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::StandardProgram->m_name = "Standard";
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/StandardInstanced.vert"));

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::StandardInstancedProgram = ResourceManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::StandardInstancedProgram->m_name = "Standard Instanced";
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
    GLPrograms::GizmoProgram = ResourceManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::GizmoProgram->m_name = "Gizmo";
    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/GizmoInstanced.vert"));

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::GizmoInstancedProgram = ResourceManager::LoadProgram(false, standardVert, standardFrag);
    GLPrograms::GizmoInstancedProgram->m_name = "Gizmo Instanced";

    vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath() + "Shaders/Vertex/ColoredGizmos.vert");
    fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath() + "Shaders/Fragment/ColoredGizmos.frag");

    standardVert = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    standardVert->Compile(vertShaderCode);
    standardFrag = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    standardFrag->Compile(fragShaderCode);
    GLPrograms::GizmoInstancedColoredProgram = std::make_unique<OpenGLUtils::GLProgram>(standardVert, standardFrag);
    GLPrograms::GizmoInstancedColoredProgram->m_name = "Gizmo Instanced Colored";
#pragma endregion

#pragma region Models &Primitives
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/quad.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Quad = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Quad);
        Primitives::Quad->m_name = "Quad";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/sphere.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Sphere = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Sphere);
        Primitives::Sphere->m_name = "Sphere";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/cube.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Cube = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Cube);
        Primitives::Cube->m_name = "Cube";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/cone.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Cone = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Cone);
        Primitives::Cone->m_name = "Cone";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/cylinder.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Cylinder = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Cylinder);
        Primitives::Cylinder->m_name = "Cylinder";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/ring.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Ring = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Ring);
        Primitives::Ring->m_name = "Ring";
    }
    {
        auto model = ResourceManager::LoadModel(
            false,
            FileIO::GetResourcePath("Primitives/monkey.obj"),
            GLPrograms::StandardProgram,
            false,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
        Primitives::Monkey = model->RootNode()->m_children[0]->m_meshMaterials[0].second;
        ResourceManager::Push(Primitives::Monkey);
        Primitives::Monkey->m_name = "Monkey";
    }
#pragma endregion

    Materials::StandardMaterial = ResourceManager::LoadMaterial(true, GLPrograms::StandardProgram);
    Materials::StandardMaterial->m_name = "Standard";
    Materials::StandardMaterial->SetTexture(Textures::StandardTexture);

    Materials::StandardInstancedMaterial = ResourceManager::LoadMaterial(true, GLPrograms::StandardInstancedProgram);
    Materials::StandardInstancedMaterial->SetTexture(Textures::StandardTexture);
    Materials::StandardInstancedMaterial->m_name = "Standard Instanced";
}
