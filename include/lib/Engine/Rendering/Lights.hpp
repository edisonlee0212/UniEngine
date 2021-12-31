#pragma once
#include "Engine/ECS/Entities.hpp"
#include <RenderTarget.hpp>
namespace UniEngine
{
#pragma region Lights
struct UNIENGINE_API DirectionalLightInfo
{
    glm::vec4 m_direction;
    glm::vec4 m_diffuse;
    glm::vec4 m_specular;
    glm::mat4 m_lightSpaceMatrix[4];
    glm::vec4 m_lightFrustumWidth;
    glm::vec4 m_lightFrustumDistance;
    glm::vec4 m_reservedParameters;
    glm::ivec4 m_viewPort;
};
class UNIENGINE_API DirectionalLight : public IPrivateComponent
{
  public:
    bool m_castShadow = true;
    glm::vec3 m_diffuse = glm::vec3(1.0f);
    float m_diffuseBrightness = 0.8f;
    float m_bias = 0.1f;
    float m_normalOffset = 0.001f;
    float m_lightSize = 0.01f;
    void OnCreate() override;
    void OnInspect() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
};
struct UNIENGINE_API PointLightInfo
{
    glm::vec4 m_position;
    glm::vec4 m_constantLinearQuadFarPlane;
    glm::vec4 m_diffuse;
    glm::vec4 m_specular;
    glm::mat4 m_lightSpaceMatrix[6];
    glm::vec4 m_reservedParameters;
    glm::ivec4 m_viewPort;
};

class UNIENGINE_API PointLight : public IPrivateComponent
{
  public:
    bool m_castShadow = true;
    float m_constant = 1.0f;
    float m_linear = 0.07f;
    float m_quadratic = 0.0015f;
    float m_bias = 0.05f;
    glm::vec3 m_diffuse = glm::vec3(1.0f);
    float m_diffuseBrightness = 0.8f;
    float m_lightSize = 0.1f;
    void OnInspect() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    [[nodiscard]] float GetFarPlane() const;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
};
struct UNIENGINE_API SpotLightInfo
{
    glm::vec4 m_position;
    glm::vec4 m_direction;
    glm::mat4 m_lightSpaceMatrix;
    glm::vec4 m_cutOffOuterCutOffLightSizeBias;
    glm::vec4 m_constantLinearQuadFarPlane;
    glm::vec4 m_diffuse;
    glm::vec4 m_specular;
    glm::ivec4 m_viewPort;
};
class UNIENGINE_API SpotLight : public IPrivateComponent
{
  public:
    bool m_castShadow = true;
    float m_innerDegrees = 20;
    float m_outerDegrees = 30;
    float m_constant = 1.0f;
    float m_linear = 0.07f;
    float m_quadratic = 0.0015f;
    float m_bias = 0.001f;
    glm::vec3 m_diffuse = glm::vec3(1.0f);
    float m_diffuseBrightness = 0.8f;
    float m_lightSize = 0.1f;
    void OnInspect() override;
    void OnCreate() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    [[nodiscard]] float GetFarPlane() const;

    void PostCloneAction(const std::shared_ptr<IPrivateComponent>& target) override;
};
#pragma endregion
#pragma region Shadow map
class UNIENGINE_API DirectionalLightShadowMap : public RenderTarget
{
    std::unique_ptr<OpenGLUtils::GLTexture2DArray> m_depthMapArray;
    void Allocate();

  public:
    DirectionalLightShadowMap(size_t resolution);
    void SetResolution(size_t resolution);
    std::unique_ptr<OpenGLUtils::GLTexture2DArray> &DepthMapArray();
    void Bind();
};
class UNIENGINE_API PointLightShadowMap : public RenderTarget
{
    std::unique_ptr<OpenGLUtils::GLTexture2DArray> m_depthMapArray;
    void Allocate();

  public:
    PointLightShadowMap(size_t resolution);
    void SetResolution(size_t resolution);
    std::unique_ptr<OpenGLUtils::GLTexture2DArray> &DepthMapArray();
    void Bind();
};
class UNIENGINE_API SpotLightShadowMap : public RenderTarget
{
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_depthMap;
    void Allocate();

  public:
    SpotLightShadowMap(size_t resolution);
    void SetResolution(size_t resolution);
    std::unique_ptr<OpenGLUtils::GLTexture2D> &DepthMap();
    void Bind();
};
#pragma endregion

} // namespace UniEngine