#include <ProjectManager.hpp>
#include <Cubemap.hpp>
#include <RenderTarget.hpp>
#include "Engine/Rendering/Graphics.hpp"
#include "Editor.hpp"
using namespace UniEngine;

std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Cubemap::Texture()
{
    return m_texture;
}

void Cubemap::OnInspect()
{
    ImGui::DragFloat("Gamma", &m_gamma);
    static AssetRef targetTexture;
    if(Editor::DragAndDropButton<Texture2D>(targetTexture, "Convert from equirectangular texture")){
        auto tex = targetTexture.Get<Texture2D>();
        if(tex) ConvertFromEquirectangularTexture(tex);
        targetTexture.Clear();
    }
}
void Cubemap::ConvertFromEquirectangularTexture(const std::shared_ptr<Texture2D> &targetTexture)
{
#pragma region Conversion
    // pbr: setup framebuffer
    // ----------------------
    size_t resolution = 1024;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(resolution, resolution))) + 1;
    m_texture = std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGB32F, resolution, resolution, true);
    m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    DefaultResources::m_2DToCubemapProgram->Bind();
    DefaultResources::m_2DToCubemapProgram->SetInt("equirectangularMap", 0);
    DefaultResources::m_2DToCubemapProgram->SetFloat4x4("projection", captureProjection);
    targetTexture->UnsafeGetGLTexture()->Bind(0);
    OpenGLUtils::SetViewPort(resolution, resolution);
    for (unsigned int i = 0; i < 6; ++i)
    {
        DefaultResources::m_2DToCubemapProgram->SetFloat4x4("view", captureViews[i]);
        renderTarget->AttachTexture2D(m_texture.get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        renderTarget->Clear();
        Graphics::RenderCube();
    }
    OpenGLUtils::GLFrameBuffer::BindDefault();
    m_texture->GenerateMipMap();
#pragma endregion
}
