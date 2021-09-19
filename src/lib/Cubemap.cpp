#include <AssetManager.hpp>
#include <Cubemap.hpp>
#include <RenderTarget.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void Cubemap::OnCreate()
{
    m_name = "New Cubemap";
}

std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Cubemap::Texture()
{
    return m_texture;
}
void Cubemap::LoadInternal(const std::filesystem::path &path)
{
    auto &manager = AssetManager::GetInstance();
    stbi_set_flip_vertically_on_load(true);
    auto texture2D = AssetManager::CreateAsset<Texture2D>();
    int width, height, nrComponents;
    float actualGamma = 0.0f;

    if (path.extension() == ".hdr")
    {
        actualGamma = 2.2f;
    }
    else
    {
        actualGamma = 1.0f;
    }

    stbi_hdr_to_ldr_gamma(actualGamma);
    stbi_ldr_to_hdr_gamma(actualGamma);
    float *data = stbi_loadf(path.string().c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        texture2D->m_texture = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RGB32F, width, height, true);
        texture2D->m_texture->SetData(0, GL_RGB, GL_FLOAT, data);
        texture2D->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        texture2D->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture2D->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture2D->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        UNIENGINE_LOG("Texture failed to load at path: " + path.string());
        stbi_image_free(data);
        return;
    }

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
    texture2D->m_texture->Bind(0);
    renderTarget->GetFrameBuffer()->ViewPort(resolution, resolution);
    for (unsigned int i = 0; i < 6; ++i)
    {
        DefaultResources::m_2DToCubemapProgram->SetFloat4x4("view", captureViews[i]);
        renderTarget->AttachTexture2D(m_texture.get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        renderTarget->Clear();
        RenderManager::RenderCube();
    }
    OpenGLUtils::GLFrameBuffer::BindDefault();
    m_texture->GenerateMipMap();
#pragma endregion
    m_name = path.filename().string();
    m_gamma = actualGamma;
}