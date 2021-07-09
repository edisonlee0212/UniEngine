#include <DefaultResources.hpp>
#include <LightProbe.hpp>
#include <RenderManager.hpp>
#include <RenderTarget.hpp>
using namespace UniEngine;

void LightProbe::OnCreate()
{
    m_name = "New light probe";
}

void LightProbe::ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap)
{
    m_gamma = targetCubemap->m_gamma;
    size_t resolution = m_irradianceMapResolution;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    auto irradianceMap = std::make_unique<OpenGLUtils::GLTextureCubeMap>(1, GL_RGB32F, resolution, resolution, true);
    m_irradianceMap = std::make_unique<Cubemap>();
    m_irradianceMap->m_gamma = m_gamma;
    m_irradianceMap->m_texture = std::move(irradianceMap);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glm::mat4 EnvironmentalMapCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 EnvironmentalMapCaptureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    DefaultResources::GLPrograms::ConvolutionProgram->Bind();
    DefaultResources::GLPrograms::ConvolutionProgram->SetInt("environmentMap", 0);
    DefaultResources::GLPrograms::ConvolutionProgram->SetFloat4x4("projection", EnvironmentalMapCaptureProjection);
    targetCubemap->m_texture->Bind(0);
    renderTarget->GetFrameBuffer()->ViewPort(
        resolution, resolution); // don't forget to configure the viewport to the capture dimensions.
    for (unsigned int i = 0; i < 6; ++i)
    {
        DefaultResources::GLPrograms::ConvolutionProgram->SetFloat4x4("view", EnvironmentalMapCaptureViews[i]);
        renderTarget->AttachTexture2D(
            m_irradianceMap->Texture().get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        renderTarget->Clear();
        RenderManager::RenderCube();
    }
    OpenGLUtils::GLFrameBuffer::BindDefault();
    m_ready = true;
}
