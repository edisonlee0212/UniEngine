#pragma once
#include "Engine/ECS/Entities.hpp"
#include <EnvironmentalMap.hpp>
#include <RenderTarget.hpp>
namespace UniEngine
{
class Camera;
struct GlobalTransform;

struct UNIENGINE_API Plane
{
    float m_a, m_b, m_c, m_d;
    Plane();
    void Normalize();
};

struct UNIENGINE_API CameraInfoBlock
{
    glm::mat4 m_projection;
    glm::mat4 m_view;
    glm::mat4 m_projectionView;
    glm::mat4 m_inverseProjection;
    glm::mat4 m_inverseView;
    glm::mat4 m_inverseProjectionView;
    glm::vec4 m_clearColor;
    glm::vec4 m_reservedParameters1;
    glm::vec4 m_reservedParameters2;
    std::unique_ptr<OpenGLUtils::GLBuffer>& GetBuffer();
    void UploadMatrices(const std::shared_ptr<Camera> &camera, const glm::vec3& position, const glm::quat& rotation);
    void UploadMatrices(const std::shared_ptr<Camera> &camera, const GlobalTransform& globalTransform);
};
struct Ray;

class Cubemap;
class UNIENGINE_API Camera final : public IPrivateComponent, public RenderTarget
{
    friend class Graphics;
    friend class RenderLayer;
    friend class EditorLayer;
    friend struct CameraInfoBlock;
    friend class PostProcessing;
    friend class Bloom;
    friend class SSAO;
    friend class SSR;
    std::shared_ptr<Texture2D> m_colorTexture;
    std::shared_ptr<Texture2D> m_depthStencilTexture;

    //Deferred shading GBuffer
    std::unique_ptr<RenderTarget> m_gBuffer;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gBufferDepth;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gBufferNormal;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gBufferAlbedo;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gBufferMetallicRoughnessEmissionAmbient;


    size_t m_frameCount = 0;
    bool m_rendered = false;
    bool m_requireRendering = false;
  public:
    [[nodiscard]] bool Rendered() const;
    void SetRequireRendering(bool value);
    Camera& operator=(const Camera& source);
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;

    static CameraInfoBlock m_cameraInfoBlock;
    
    float m_nearDistance = 0.1f;
    float m_farDistance = 500.0f;
    float m_fov = 120;

    static void CalculatePlanes(std::vector<Plane> &planes, glm::mat4 projection, glm::mat4 view);
    static void CalculateFrustumPoints(
        const std::shared_ptr<Camera> &cameraComponent,
        float nearPlane,
        float farPlane,
        glm::vec3 cameraPos,
        glm::quat cameraRot,
        glm::vec3 *points);
    static glm::quat ProcessMouseMovement(float yawAngle, float pitchAngle, bool constrainPitch = true);
    static void ReverseAngle(
        const glm::quat &rotation, float &pitchAngle, float &yawAngle, const bool &constrainPitch = true);
    [[nodiscard]] std::shared_ptr<Texture2D> GetTexture() const;
    [[nodiscard]] std::shared_ptr<Texture2D> GetDepthStencil() const;
    [[nodiscard]] glm::mat4 GetProjection() const;
    static glm::vec3 Project(GlobalTransform &ltw, glm::vec3 position);

    glm::vec3 UnProject(GlobalTransform &ltw, glm::vec3 position) const;

    glm::vec3 GetMouseWorldPoint(GlobalTransform &ltw, glm::vec2 mousePosition) const;
    void SetClearColor(glm::vec3 color) const;
    Ray ScreenPointToRay(GlobalTransform &ltw, glm::vec2 mousePosition) const;

    void ResizeResolution(int x, int y);
    void OnCreate() override;
    void Start() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void OnDestroy() override;
    bool m_useClearColor = false;
    glm::vec3 m_clearColor = glm::vec3(0.0f);
    float m_backgroundIntensity = 1.0f;
    AssetRef m_skybox;
    void OnInspect() override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;

    [[nodiscard]] std::unique_ptr<RenderTarget>& UnsafeGetGBuffer();
    [[nodiscard]] std::unique_ptr<OpenGLUtils::GLTexture2D>& UnsafeGetGBufferDepth();
    [[nodiscard]] std::unique_ptr<OpenGLUtils::GLTexture2D>& UnsafeGetGBufferNormal();
    [[nodiscard]] std::unique_ptr<OpenGLUtils::GLTexture2D>& UnsafeGetGBufferAlbedo();
    [[nodiscard]] std::unique_ptr<OpenGLUtils::GLTexture2D>& UnsafeGetGBufferMetallicRoughnessEmissionAmbient();
};

} // namespace UniEngine
