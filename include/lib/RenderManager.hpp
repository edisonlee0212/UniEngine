#pragma once
#include <CameraComponent.hpp>
#include <DefaultResources.hpp>
#include <Lights.hpp>
#include <Particles.hpp>
namespace UniEngine
{
struct UNIENGINE_API LightSettingsBlock
{
    float m_splitDistance[4];
    int m_pcfSampleAmount = 16;
    float m_scaleFactor = 1.0f;
    int m_blockerSearchAmount = 2;
    float m_seamFixRatio = 0.05f;
    float m_vsmMaxVariance = 0.001f;
    float m_lightBleedFactor = 0.5f;
    float m_evsmExponent = 40.0f;
    float m_ambientLight = 1.0f;
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
    float m_alphaDiscardOffset = 0.1f;
};
struct EnvironmentalMapSettingsBlock
{
    GLuint64 m_skybox = 0;
    GLuint64 m_environmentalIrradiance = 0;
    GLuint64 m_environmentalPrefiltered = 0;
    GLuint64 m_environmentalBrdfLut = 0;
    glm::vec4 m_backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

class UNIENGINE_API RenderManager : public ISingleton<RenderManager>
{
    CameraComponent *m_mainCameraComponent = nullptr;
#pragma region Global Var
#pragma region GUI
    bool m_enableRenderMenu = true;
    bool m_enableInfoWindow = true;
#pragma endregion
#pragma region Render
    std::unique_ptr<Texture2D> m_brdfLut;
    std::unique_ptr<OpenGLUtils::GLUBO> m_kernelBlock;
    std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferInstancedPrepass;
    std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferPrepass;
    std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferLightingPass;
    bool m_mainCameraViewable = true;
    int m_mainCameraResolutionX = 1;
    int m_mainCameraResolutionY = 1;
    friend class RenderTarget;
    size_t m_triangles = 0;
    size_t m_drawCall = 0;
    friend class DefaultResources;
    

    std::unique_ptr<OpenGLUtils::GLUBO> m_materialSettingsBuffer;
    std::unique_ptr<OpenGLUtils::GLUBO> m_environmentalMapSettingsBuffer;
#pragma endregion
#pragma region Shadow
    OpenGLUtils::GLUBO m_directionalLightBlock;
    OpenGLUtils::GLUBO m_pointLightBlock;
    OpenGLUtils::GLUBO m_spotLightBlock;

    size_t m_shadowMapResolution = 4096;
    OpenGLUtils::GLUBO m_shadowCascadeInfoBlock;

    DirectionalLightInfo m_directionalLights[DefaultResources::ShaderIncludes::MaxDirectionalLightAmount];
    PointLightInfo m_pointLights[DefaultResources::ShaderIncludes::MaxPointLightAmount];
    SpotLightInfo m_spotLights[DefaultResources::ShaderIncludes::MaxSpotLightAmount];

    std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightInstancedProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightInstancedProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightInstancedProgram;

    friend class EditorManager;

    std::unique_ptr<DirectionalLightShadowMap> m_directionalLightShadowMap;
    std::unique_ptr<PointLightShadowMap> m_pointLightShadowMap;
    std::unique_ptr<SpotLightShadowMap> m_spotLightShadowMap;

#pragma endregion
#pragma endregion
    static void PrepareBrdfLut();
    static void DeferredPrepass(const Mesh *mesh, const Material *material, const glm::mat4 &model);
    static void DeferredPrepassInternal(const Mesh *mesh, const glm::mat4 &model);
    static void DeferredPrepassInstanced(
        const Mesh *mesh, const Material *material, const glm::mat4 &model, glm::mat4 *matrices, const size_t &count);

    static void DrawMesh(const Mesh *mesh, const Material *material, const glm::mat4 &model, const bool &receiveShadow);
    static void DrawMeshInternal(const Mesh *mesh, const glm::mat4 &model);
    static void DrawMeshInstanced(
        const Mesh *mesh,
        const Material *material,
        const glm::mat4 &model,
        const glm::mat4 *matrices,
        const size_t &count,
        const bool &receiveShadow);

    static void DrawGizmoMesh(
        const Mesh *mesh, const glm::vec4 &color, const glm::mat4 &model, const glm::mat4 &scaleMatrix);
    static void DrawGizmoMeshInstanced(
        const Mesh *mesh,
        const glm::vec4 &color,
        const glm::mat4 &model,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &scaleMatrix);
    static void DrawGizmoMeshInstancedColored(
        const Mesh *mesh,
        const glm::vec4 *colors,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &model,
        const glm::mat4 &scaleMatrix);

    static float Lerp(const float &a, const float &b, const float &f);

    friend class ReflectionProbe;
    friend class LightProbe;
    friend class ResourceManager;
    static void RenderCube();
    static void RenderQuad();
  public:
    bool m_stableFit = true;
    float m_maxShadowDistance = 500;
    float m_shadowCascadeSplit[DefaultResources::ShaderIncludes::ShadowCascadeAmount] = {0.15f, 0.3f, 0.5f, 1.0f};
    LightSettingsBlock m_lightSettings;
    MaterialSettingsBlock m_materialSettings;
    EnvironmentalMapSettingsBlock m_environmentalMapSettings;
    static void ShadowEnvironmentPreset(const CameraComponent *cameraComponent);
    static void ApplyEnvironmentalSettings(const CameraComponent *cameraComponent);
    static void MaterialPropertySetter(const Material *material, const bool &disableBlending = false);
    static void ApplyMaterialSettings(const Material *material, const OpenGLUtils::GLProgram *program);
    static void ReleaseTextureHandles(const Material *material);
    static void RenderToCameraDeferred(
        const std::unique_ptr<CameraComponent> &cameraComponent,
        const GlobalTransform &cameraTransform,
        glm::vec3 &minBound,
        glm::vec3 &maxBound,
        bool calculateBounds = false);
    static void RenderBackGround(const std::unique_ptr<CameraComponent> &cameraComponent);
    static void RenderToCameraForward(
        const std::unique_ptr<CameraComponent> &cameraComponent,
        const GlobalTransform &cameraTransform,
        glm::vec3 &minBound,
        glm::vec3 &maxBound,
        bool calculateBounds = false);
    static void Init();
    // Main rendering happens here.
    static void PreUpdate();
#pragma region Shadow
    static void SetSplitRatio(const float &r1, const float &r2, const float &r3, const float &r4);
    static void SetShadowMapResolution(const size_t &value);

    static glm::vec3 ClosestPointOnLine(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b);
#pragma endregion
#pragma region RenderAPI
    static void OnGui();
    static size_t Triangles();
    static size_t DrawCall();

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
        const CameraComponent *cameraComponent);

    static void SetMainCamera(CameraComponent *value);
    static CameraComponent *GetMainCamera();
#pragma region Gizmos
    static void DrawGizmoMesh(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec4 &color = glm::vec4(1.0f),
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstanced(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec4 &color,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstancedColored(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec4 *colors,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);

    static void DrawGizmoMesh(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color = glm::vec4(1.0f),
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstanced(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);
    static void DrawGizmoMeshInstancedColored(
        const Mesh *mesh,
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 *colors,
        const glm::mat4 *matrices,
        const size_t &count,
        const glm::mat4 &model = glm::mat4(1.0f),
        const float &size = 1.0f);

    static void DrawGizmoRay(
        const CameraComponent *cameraComponent,
        const glm::vec4 &color,
        const glm::vec3 &start,
        const glm::vec3 &end,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const CameraComponent *cameraComponent,
        const glm::vec4 &color,
        std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const CameraComponent *cameraComponent,
        const glm::vec4 &color,
        std::vector<Ray> &rays,
        const float &width = 0.01f);
    static void DrawGizmoRay(
        const CameraComponent *cameraComponent, const glm::vec4 &color, Ray &ray, const float &width = 0.01f);

    static void DrawGizmoRay(
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        const glm::vec3 &start,
        const glm::vec3 &end,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
        const float &width = 0.01f);
    static void DrawGizmoRays(
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        std::vector<Ray> &rays,
        const float &width = 0.01f);
    static void DrawGizmoRay(
        const CameraComponent *cameraComponent,
        const glm::vec3 &cameraPosition,
        const glm::quat &cameraRotation,
        const glm::vec4 &color,
        Ray &ray,
        const float &width = 0.01f);
#pragma endregion

    static void DrawMesh(
        const Mesh *mesh,
        const Material *material,
        const glm::mat4 &model,
        const CameraComponent *cameraComponent,
        const bool &receiveShadow = true);
    static void DrawMeshInstanced(
        const Mesh *mesh,
        const Material *material,
        const glm::mat4 &model,
        const glm::mat4 *matrices,
        const size_t &count,
        const CameraComponent *cameraComponent,
        const bool &receiveShadow = true);

#pragma endregion
};
} // namespace UniEngine
