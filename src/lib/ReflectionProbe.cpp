#include <DefaultResources.hpp>
#include <ReflectionProbe.hpp>
#include "Engine/Rendering/Graphics.hpp"
#include <RenderTarget.hpp>
using namespace UniEngine;

void ReflectionProbe::ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap)
{
    m_gamma = targetCubemap->m_gamma;
    size_t resolution = m_preFilteredMapResolution;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);

    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
    // --------------------------------------------------------------------------------
    GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(resolution, resolution))) + 1;
    auto preFilteredMap =
        std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGB32F, resolution, resolution, true);
    m_preFilteredMap = std::make_unique<Cubemap>();
    m_preFilteredMap->m_gamma = m_gamma;
    m_preFilteredMap->m_texture = std::move(preFilteredMap);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_preFilteredMap->m_texture->GenerateMipMap();
    glm::mat4 EnvironmentalMapCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 EnvironmentalMapCaptureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    DefaultResources::PrefilterProgram->Bind();
    DefaultResources::PrefilterProgram->SetInt("environmentMap", 0);
    DefaultResources::PrefilterProgram->SetFloat4x4("projection", EnvironmentalMapCaptureProjection);

    unsigned int maxMipLevels = mipmap;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = resolution * std::pow(0.5, mip);
        auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
        renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, mipWidth, mipWidth);
        renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
        targetCubemap->Texture()->Bind(0);
        OpenGLUtils::SetViewPort(mipWidth, mipWidth);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        DefaultResources::PrefilterProgram->SetFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            DefaultResources::PrefilterProgram->SetFloat4x4("view", EnvironmentalMapCaptureViews[i]);
            renderTarget->AttachTexture2D(
                m_preFilteredMap->m_texture.get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip);
            renderTarget->Clear();
            Graphics::RenderCube();
        }
    }
    OpenGLUtils::GLFrameBuffer::BindDefault();
    m_ready = true;
}
