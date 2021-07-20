#include <Gui.hpp>
#include <Lights.hpp>
using namespace UniEngine;

void SpotLight::OnGui()
{
    ImGui::Checkbox("Cast Shadow", &m_castShadow);
    ImGui::ColorEdit3("Color", &m_diffuse[0]);
    ImGui::DragFloat("Intensity", &m_diffuseBrightness, 0.1f, 0.0f, 999.0f);
    ImGui::DragFloat("Bias", &m_bias, 0.001f, 0.0f, 999.0f);

    ImGui::DragFloat("Constant", &m_constant, 0.01f, 0.0f, 999.0f);
    ImGui::DragFloat("Linear", &m_linear, 0.001f, 0, 1, "%.3f");
    ImGui::DragFloat("Quadratic", &m_quadratic, 0.001f, 0, 10, "%.4f");

    ImGui::DragFloat("Inner Degrees", &m_innerDegrees, 0.1f, 0.0f, m_outerDegrees);
    ImGui::DragFloat("Outer Degrees", &m_outerDegrees, 0.1f, m_innerDegrees, 180.0f);
    ImGui::DragFloat("Light Size", &m_lightSize, 0.01f, 0.0f, 999.0f);
}

void SpotLight::OnCreate()
{
    SetEnabled(true);
}

void SpotLight::Serialize(YAML::Emitter &out)
{
}

void SpotLight::Deserialize(const YAML::Node &in)
{
}

void PointLight::OnGui()
{
    ImGui::Checkbox("Cast Shadow", &m_castShadow);
    ImGui::ColorEdit3("Color", &m_diffuse[0]);
    ImGui::DragFloat("Intensity", &m_diffuseBrightness, 0.1f, 0.0f, 999.0f);
    ImGui::DragFloat("Bias", &m_bias, 0.001f, 0.0f, 999.0f);

    ImGui::DragFloat("Constant", &m_constant, 0.01f, 0.0f, 999.0f);
    ImGui::DragFloat("Linear", &m_linear, 0.0001f, 0, 1, "%.4f");
    ImGui::DragFloat("Quadratic", &m_quadratic, 0.00001f, 0, 10, "%.5f");

    // ImGui::InputFloat("Normal Offset", &dl->normalOffset, 0.01f);
    ImGui::DragFloat("Light Size", &m_lightSize, 0.01f, 0.0f, 999.0f);
}

void PointLight::OnCreate()
{
    SetEnabled(true);
}

void PointLight::Serialize(YAML::Emitter &out)
{
}

void PointLight::Deserialize(const YAML::Node &in)
{
}

void DirectionalLight::OnCreate()
{
    SetEnabled(true);
}

void DirectionalLight::OnGui()
{
    ImGui::Checkbox("Cast Shadow", &m_castShadow);
    ImGui::ColorEdit3("Color", &m_diffuse[0]);
    ImGui::DragFloat("Intensity", &m_diffuseBrightness, 0.1f, 0.0f, 999.0f);
    ImGui::DragFloat("Bias", &m_bias, 0.001f, 0.0f, 999.0f);
    ImGui::DragFloat("Normal Offset", &m_normalOffset, 0.001f, 0.0f, 999.0f);
    ImGui::DragFloat("Light Size", &m_lightSize, 0.01f, 0.0f, 999.0f);
}

void DirectionalLight::Serialize(YAML::Emitter &out)
{
}

void DirectionalLight::Deserialize(const YAML::Node &in)
{
}

void UniEngine::DirectionalLightShadowMap::Allocate()
{
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_depthMapArray = std::make_unique<OpenGLUtils::GLTexture2DArray>(
        1, GL_DEPTH_COMPONENT16, (GLsizei)m_resolutionX, (GLsizei)m_resolutionY, (GLsizei)4);
    m_depthMapArray->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_depthMapArray->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_depthMapArray->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    m_depthMapArray->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    m_depthMapArray->SetFloat4(GL_TEXTURE_BORDER_COLOR, borderColor);
    AttachTexture(m_depthMapArray.get(), GL_DEPTH_ATTACHMENT);
}
UniEngine::DirectionalLightShadowMap::DirectionalLightShadowMap(size_t resolution)
{
    SetResolution(resolution);
}

void DirectionalLightShadowMap::SetResolution(size_t resolution)
{
    m_resolutionX = m_resolutionY = resolution;
    Allocate();
}

std::unique_ptr<OpenGLUtils::GLTexture2DArray> &DirectionalLightShadowMap::DepthMapArray()
{
    return m_depthMapArray;
}

void UniEngine::DirectionalLightShadowMap::Bind()
{
    RenderTarget::Bind();
}
void UniEngine::SpotLightShadowMap::Allocate()
{
    m_depthMap = std::make_unique<OpenGLUtils::GLTexture2D>(
        1, GL_DEPTH_COMPONENT16, (GLsizei)m_resolutionX, (GLsizei)m_resolutionY);
    m_depthMap->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_depthMap->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_depthMap->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthMap->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthMap->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    AttachTexture(m_depthMap.get(), GL_DEPTH_ATTACHMENT);
}

UniEngine::SpotLightShadowMap::SpotLightShadowMap(size_t resolution)
{
    SetResolution(resolution);
}

void UniEngine::SpotLightShadowMap::SetResolution(size_t resolution)
{
    m_resolutionX = m_resolutionY = resolution;
    Allocate();
}

std::unique_ptr<UniEngine::OpenGLUtils::GLTexture2D> &UniEngine::SpotLightShadowMap::DepthMap()
{
    return m_depthMap;
}

void UniEngine::SpotLightShadowMap::Bind()
{
    RenderTarget::Bind();
}
void PointLightShadowMap::Allocate()
{
    m_depthMapArray = std::make_unique<OpenGLUtils::GLTexture2DArray>(
        1, GL_DEPTH_COMPONENT16, (GLsizei)m_resolutionX, (GLsizei)m_resolutionY, (GLsizei)6);
    m_depthMapArray->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_depthMapArray->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_depthMapArray->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthMapArray->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthMapArray->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    AttachTexture(m_depthMapArray.get(), GL_DEPTH_ATTACHMENT);
}

UniEngine::PointLightShadowMap::PointLightShadowMap(size_t resolution)
{
    SetResolution(resolution);
}

void PointLightShadowMap::SetResolution(size_t resolution)
{
    m_resolutionX = m_resolutionY = resolution;
    Allocate();
}

std::unique_ptr<OpenGLUtils::GLTexture2DArray> &UniEngine::PointLightShadowMap::DepthMapArray()
{
    return m_depthMapArray;
}

void UniEngine::PointLightShadowMap::Bind()
{
    RenderTarget::Bind();
}
