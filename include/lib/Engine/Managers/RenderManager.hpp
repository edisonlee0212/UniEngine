#pragma once
#include <Camera.hpp>
#include <DefaultResources.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <Particles.hpp>
#include <SkinnedMeshRenderer.hpp>
namespace UniEngine
{
struct UNIENGINE_API LightSettingsBlock
{
    float m_splitDistance[4];
    int m_pcfSampleAmount = 64;
    float m_scaleFactor = 1.0f;
    int m_blockerSearchAmount = 1;
    float m_seamFixRatio = 0.05f;
    float m_vsmMaxVariance = 0.001f;
    float m_lightBleedFactor = 0.5f;
    float m_gamma = 2.2f;
    float m_ambientLight = 0.8f;
};

struct MaterialSettingsBlock
{
    GLuint64 m_directionalShadowMap = 0;
    GLuint64 m_pointShadowMap = 0;
    GLuint64 m_spotShadowMap = 0;

    GLuint64 m_albedoMap = 0;
    GLuint64 m_normalMap = 0;
    GLuint64 m_metallicMap = 0;
    GLuint64 m_roughnessMap = 0;
    GLuint64 m_aoMap = 0;

    int m_albedoEnabled = 0;
    int m_normalEnabled = 0;
    int m_metallicEnabled = 0;
    int m_roughnessEnabled = 0;
    int m_aoEnabled = 0;
    int m_alphaDiscardEnabled = true;
    int m_receiveShadow = true;
    int m_enableShadow = true;

    glm::vec4 m_albedoColorVal = glm::vec4(1.0f);
    float m_metallicVal = 0.5f;
    float m_roughnessVal = 0.5f;
    float m_aoVal = 1.0f;
    float m_emissionVal = 0.0f;
    float m_alphaDiscardOffset = 0.1f;
};
struct EnvironmentalMapSettingsBlock
{
    GLuint64 m_skybox = 0;
    GLuint64 m_environmentalIrradiance = 0;
    GLuint64 m_environmentalPrefiltered = 0;
    GLuint64 m_environmentalBrdfLut = 0;
    glm::vec4 m_backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_skyboxGamma = 1.0f;
    float m_environmentalLightingGamma = 1.0f;
    float m_environmentalPadding1 = 1.0f;
    float m_environmentalPadding2 = 0.0f;
};

enum class RenderCommandType
{
    FromRenderer,
    FromAPI,
    FromAPIInstanced
};

enum class RenderCommandMeshType
{
    Default,
    Skinned
};
struct RenderCommand
{
    RenderCommandType m_commandType = RenderCommandType::FromRenderer;
    RenderCommandMeshType m_meshType = RenderCommandMeshType::Default;
    Entity m_owner = Entity();
    std::weak_ptr<Mesh> m_mesh;
    std::weak_ptr<SkinnedMesh> m_skinnedMesh;
    bool m_castShadow = true;
    bool m_receiveShadow = true;
    std::weak_ptr<ParticleMatrices> m_matrices;
    std::weak_ptr<BoneMatrices> m_boneMatrices; // We require the skinned mesh renderer to provide bones.
    GlobalTransform m_globalTransform;
};

struct RenderGeometryGroup
{
    std::map<std::weak_ptr<OpenGLUtils::GLVAO>, std::vector<RenderCommand>, std::owner_less<>>
        m_meshes;
    std::map<std::weak_ptr<OpenGLUtils::GLVAO>, std::vector<RenderCommand>, std::owner_less<>>
        m_skinnedMeshes;
};

struct RenderCommands
{
    std::map<std::weak_ptr<Material>, RenderGeometryGroup, std::owner_less<>> m_value;
};

class UNIENGINE_API RenderManager : public ISingleton<RenderManager>
{
    std::weak_ptr<Camera> m_mainCameraComponent;
#pragma region Global Var
#pragma region GUI
    bool m_enableRenderMenu = true;
    bool m_enableInfoWindow = true;
#pragma endregion
#pragma region Render

    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_deferredRenderInstances;
    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_deferredInstancedRenderInstances;
    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_forwardRenderInstances;
    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_forwardInstancedRenderInstances;
    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_transparentRenderInstances;
    std::map<std::weak_ptr<Camera>, RenderCommands, std::owner_less<>> m_instancedTransparentRenderInstances;

    std::unique_ptr<OpenGLUtils::GLUBO> m_kernelBlock;

    bool m_mainCameraViewable = true;
    int m_mainCameraResolutionX = 1;
    int m_mainCameraResolutionY = 1;
    friend class RenderTarget;
    friend class DefaultResources;
    friend class EditorManager;
    friend class LightProbe;
    friend class ReflectionProbe;
    friend class EnvironmentalMap;
    friend class Cubemap;
    friend class AssetManager;
    size_t m_triangles = 0;
    size_t m_drawCall = 0;
    std::unique_ptr<OpenGLUtils::GLUBO> m_materialSettingsBuffer;
    std::unique_ptr<OpenGLUtils::GLUBO> m_environmentalMapSettingsBuffer;
#pragma endregion
#pragma region Lightings
    AssetRef m_environmentalMap;

    OpenGLUtils::GLUBO m_directionalLightBlock;
    OpenGLUtils::GLUBO m_pointLightBlock;
    OpenGLUtils::GLUBO m_spotLightBlock;

    size_t m_directionalLightShadowMapResolution = 4096;
    size_t m_pointLightShadowMapResolution = 2048;
    size_t m_spotLightShadowMapResolution = 2048;
    OpenGLUtils::GLUBO m_shadowCascadeInfoBlock;

    DirectionalLightInfo m_directionalLights[DefaultResources::ShaderIncludes::MaxDirectionalLightAmount];
    PointLightInfo m_pointLights[DefaultResources::ShaderIncludes::MaxPointLightAmount];
    SpotLightInfo m_spotLights[DefaultResources::ShaderIncludes::MaxSpotLightAmount];

    std::unique_ptr<DirectionalLightShadowMap> m_directionalLightShadowMap;
    std::unique_ptr<PointLightShadowMap> m_pointLightShadowMap;
    std::unique_ptr<SpotLightShadowMap> m_spotLightShadowMap;

#pragma endregion
#pragma endregion
    static void PrepareBrdfLut();
    static void DeferredPrepassInternal(const std::shared_ptr<Mesh> &mesh);
    static void DeferredPrepassInstancedInternal(
        const std::shared_ptr<Mesh> &mesh, const std::vector<glm::mat4> &matrices);
    static void DeferredPrepassInternal(const std::shared_ptr<SkinnedMesh> &skinnedMesh);
    static void DeferredPrepassInstancedInternal(
        const std::shared_ptr<SkinnedMesh> &skinnedMesh, const std::vector<glm::mat4> &matrices);

    static void DrawMesh(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Material> &material,
        const glm::mat4 &model,
        const bool &receiveShadow);
    static void DrawMeshInternal(const std::shared_ptr<Mesh> &mesh);
    static void DrawMeshInternal(const std::shared_ptr<SkinnedMesh> &mesh);
    static void DrawMeshInstanced(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Material> &material,
        const glm::mat4 &model,
        const std::vector<glm::mat4> &matrices,
        const bool &receiveShadow);
    static void DrawMeshInstancedInternal(const std::shared_ptr<Mesh> &mesh, const std::vector<glm::mat4> &matrices);
    static void DrawMeshInstancedInternal(
        const std::shared_ptr<SkinnedMesh> &mesh, const std::vector<glm::mat4> &matrices);
    static void DrawGizmoMesh(
        const std::shared_ptr<Mesh> &mesh,
        const glm::vec4 &color,
        const glm::mat4 &model,
        const glm::mat4 &scaleMatrix);
    static void DrawGizmoMeshInstanced(
        const std::shared_ptr<Mesh> &mesh,
        const glm::vec4 &color,
        const glm::mat4 &model,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &scaleMatrix);
    static void DrawGizmoMeshInstancedColored(
        const std::shared_ptr<Mesh> &mesh,
        const std::vector<glm::vec4> &colors,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &model,
        const glm::mat4 &scaleMatrix);

    static float Lerp(const float &a, const float &b, const float &f);


    static void RenderCube();
    static void RenderQuad();

  public:
    static void RenderManager::DispatchRenderCommands(
        const RenderCommands &renderCommands,
        const std::function<void(const std::shared_ptr<Material> &, const RenderCommand &renderCommand)> &func,
        const bool &setMaterial,
        const bool &bindProgram);

    bool m_stableFit = true;
    float m_maxShadowDistance = 300;
    float m_shadowCascadeSplit[DefaultResources::ShaderIncludes::ShadowCascadeAmount] = {0.075f, 0.15f, 0.3f, 1.0f};
    LightSettingsBlock m_lightSettings;
    MaterialSettingsBlock m_materialSettings;
    EnvironmentalMapSettingsBlock m_environmentalMapSettings;
    static void ApplyShadowMapSettings();
    static void ApplyEnvironmentalSettings(const std::shared_ptr<Camera> &cameraComponent);
    static void MaterialPropertySetter(const std::shared_ptr<Material> &material, const bool &disableBlending = false);
    static void ApplyMaterialSettings(const std::shared_ptr<Material> &material);
    static void ApplyProgramSettings(const std::shared_ptr<OpenGLUtils::GLProgram> &program);
    static void ReleaseMaterialSettings(const std::shared_ptr<Material> &material);
    static void RenderToCamera(const std::shared_ptr<Camera> &cameraComponent);
    static void ShadowMapPrePass(
        const int &enabledSize,
        std::shared_ptr<OpenGLUtils::GLProgram> &defaultProgram,
        std::shared_ptr<OpenGLUtils::GLProgram> &defaultInstancedProgram,
        std::shared_ptr<OpenGLUtils::GLProgram> &skinnedProgram,
        std::shared_ptr<OpenGLUtils::GLProgram> &instancedSkinnedProgram);
    static void RenderShadows(Bound &worldBound, const std::shared_ptr<Camera> &cameraComponent, const Entity &mainCameraEntity);
    static void Init();
    // Main rendering happens here.

    static void CollectRenderInstances(
        const std::shared_ptr<Camera> &camera, const glm::vec3 &position, Bound &worldBound, const bool &calculateBound = false);
#pragma region Shadow
    static void SetSplitRatio(const float &r1, const float &r2, const float &r3, const float &r4);
    static void SetDirectionalLightShadowMapResolution(const size_t &value);
    static void SetPointLightShadowMapResolution(const size_t &value);
    static void SetSpotLightShadowMapResolution(const size_t &value);
    static glm::vec3 ClosestPointOnLine(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b);
#pragma endregion
#pragma region RenderAPI
    static void OnGui();
    static void PreUpdate();
    static void LateUpdate();
    static size_t Triangles();
    static size_t DrawCall();

    static void SetMainCamera(const std::shared_ptr<Camera> &value);
    static std::weak_ptr<Camera> GetMainCamera();
#pragma region Gizmos
    static void DrawGizmoMesh(
        const std::shared_ptr<Mesh> &mesh,
        const glm::vec4 &color = glm::vec4(1.0f),
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstanced(
        const std::shared_ptr<Mesh> &mesh,
        const glm::vec4 &color,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstancedColored(
        const std::shared_ptr<Mesh> &mesh,
        const std::vector<glm::vec4> &colors,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);

    static void DrawGizmoMesh(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color = glm::vec4(1.0f),
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstanced(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstancedColored(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const std::vector<glm::vec4> &colors,
        const std::vector<glm::mat4> &matrices,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoRay(
        const glm::vec4 &color, const glm::vec3 &start, const glm::vec3 &end, const float &width = 0.01f);
    static void DrawGizmoRays(
        const glm::vec4 &color,
        const std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
        const float &width = 0.01f);
    static void DrawGizmoRays(const glm::vec4 &color, const std::vector<Ray> &rays, const float &width = 0.01f);
    static void DrawGizmoRay(const glm::vec4 &color, const Ray &ray, const float &width = 0.01f);
    static void DrawGizmoRay(
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const glm::vec3 &start,
        const glm::vec3 &end,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const std::vector<Ray> &rays,
        const float &width = 0.01f);
    static void DrawGizmoRay(
        const std::shared_ptr<Camera> &cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const Ray &ray,
        const float &width = 0.01f);
#pragma endregion
    static void DrawMesh(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Material> &material,
        const glm::mat4 &model,
        const std::shared_ptr<Camera> &cameraComponent,
        const bool &receiveShadow = true,
        const bool &castShadow = true);
    static void DrawMeshInstanced(
        const std::shared_ptr<Mesh> &mesh,
        const std::shared_ptr<Material> &material,
        const glm::mat4 &model,
        const std::shared_ptr<ParticleMatrices> &matrices,
        const std::shared_ptr<Camera> &cameraComponent,
        const bool &receiveShadow = true,
        const bool &castShadow = true);

    /*
    static void DrawTexture2D(
        const OpenGLUtils::GLTexture2D *texture, const float &depth, const glm::vec2 &center, const glm::vec2 &size);
    static void DrawTexture2D(
        const OpenGLUtils::GLTexture2D *texture,
        const float &depth,
        const glm::vec2 &center,
        const glm::vec2 &size,
        const RenderTarget *target);
    static void DrawTexture2D(
        const Texture2D *texture,
        const float &depth,
        const glm::vec2 &center,
        const glm::vec2 &size,
        const RenderTarget *target);
    static void DrawTexture2D(
        const Texture2D *texture,
        const float &depth,
        const glm::vec2 &center,
        const glm::vec2 &size,
        const Camera &cameraComponent);
    */
#pragma endregion
};
} // namespace UniEngine
