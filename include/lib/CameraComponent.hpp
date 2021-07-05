#pragma once
#include <EntityManager.hpp>
#include <RenderTarget.hpp>
#include <EnvironmentalMap.hpp>
namespace UniEngine
{
class CameraComponent;
struct GlobalTransform;

enum class CameraLayer
{
    None = 0,
    MainCamera = 1 << 0,
    DebugCamera = 1 << 1,
};

struct UNIENGINE_API CameraLayerMask : ComponentDataBase
{
    size_t m_value;
    bool operator==(const CameraLayerMask &other) const
    {
        return other.m_value == m_value;
    }
    CameraLayerMask();
};
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
    glm::vec4 m_reservedParameters;
    glm::vec4 m_position;
    void UpdateMatrices(const CameraComponent *camera, glm::vec3 position, glm::quat rotation);
    void UploadMatrices(const CameraComponent *camera) const;
};
struct Ray;

class Cubemap;
class UNIENGINE_API CameraComponent final : public PrivateComponentBase, public RenderTarget
{
    friend class RenderManager;
    friend class EditorManager;
    friend struct CameraInfoBlock;
    friend class PostProcessing;
    friend class Bloom;
    friend class SSAO;
    friend class GreyScale;
    std::shared_ptr<Texture2D> m_colorTexture;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_depthStencilBuffer;
    std::unique_ptr<RenderTarget> m_gBuffer;
    std::unique_ptr<OpenGLUtils::GLRenderBuffer> m_gDepthBuffer;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gPositionBuffer;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gNormalBuffer;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gColorSpecularBuffer;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_gMetallicRoughnessAo;

    static std::unique_ptr<OpenGLUtils::GLUBO> m_cameraUniformBufferBlock;
    bool m_isMainCamera = false;

  public:
    static CameraInfoBlock m_cameraInfoBlock;
    bool m_allowAutoResize = true;
    float m_nearDistance = 0.1f;
    float m_farDistance = 500.0f;
    float m_fov = 90;
    void StoreToJpg(const std::string &path, int resizeX = -1, int resizeY = -1) const;
    void StoreToPng(const std::string &path, int resizeX = -1, int resizeY = -1, bool alphaChannel = false) const;

    static void CalculatePlanes(std::vector<Plane> &planes, glm::mat4 projection, glm::mat4 view);
    static void CalculateFrustumPoints(
        CameraComponent *cameraComponrnt,
        float nearPlane, float farPlane, glm::vec3 cameraPos, glm::quat cameraRot, glm::vec3 *points);
    static glm::quat ProcessMouseMovement(float yawAngle, float pitchAngle, bool constrainPitch = true);
    static void ReverseAngle(
        const glm::quat &rotation, float &pitchAngle, float &yawAngle, const bool &constrainPitch = true);
    [[nodiscard]] std::shared_ptr<Texture2D> GetTexture() const;
    [[nodiscard]] glm::mat4 GetProjection() const;
    static glm::vec3 Project(GlobalTransform &ltw, glm::vec3 position);

    glm::vec3 UnProject(GlobalTransform &ltw, glm::vec3 position) const;

    glm::vec3 GetMouseWorldPoint(GlobalTransform &ltw, glm::vec2 mousePosition) const;
    void SetClearColor(glm::vec3 color) const;
    Ray ScreenPointToRay(GlobalTransform &ltw, glm::vec2 mousePosition) const;
    static void GenerateMatrices();
    void ResizeResolution(int x, int y);
    CameraComponent();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    ~CameraComponent() override;
    bool m_useClearColor = false;
    glm::vec3 m_clearColor = glm::vec3(0.0f);
    std::shared_ptr<EnvironmentalMap> m_environmentalMap;
    void OnGui() override;
};

} // namespace UniEngine
