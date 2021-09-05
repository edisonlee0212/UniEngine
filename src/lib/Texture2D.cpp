#include <ConsoleManager.hpp>
#include <Texture2D.hpp>
using namespace UniEngine;

void Texture2D::OnCreate()
{
    m_name = "New Texture2D";
}

glm::vec2 Texture2D::GetResolution() const
{
    return {m_texture->m_width, m_texture->m_height};
}

void Texture2D::StoreToPng(
    const std::string &path, int resizeX, int resizeY, bool alphaChannel, unsigned compressionLevel) const
{
    stbi_write_png_compression_level = static_cast<int>(compressionLevel);
    const auto resolutionX = m_texture->m_width;
    const auto resolutionY = m_texture->m_height;
    float channels = 3;
    if (alphaChannel)
        channels = 4;
    std::vector<float> dst;
    dst.resize(resolutionX * resolutionY * channels);
    m_texture->Bind(0);
    glGetTexImage(GL_TEXTURE_2D, 0, (alphaChannel ? GL_RGBA : GL_RGB), GL_FLOAT, (void *)dst.data());
    std::vector<uint8_t> pixels;
    if (resizeX > 0 && resizeY > 0 && (resizeX != resolutionX || resizeY != resolutionY))
    {
        std::vector<float> res;
        res.resize(resizeX * resizeY * channels);
        stbir_resize_float(dst.data(), resolutionX, resolutionY, 0, res.data(), resizeX, resizeY, 0, channels);
        pixels.resize(resizeX * resizeY * channels);
        for (int i = 0; i < resizeX * resizeY; i++)
        {
            pixels[i * channels] = glm::clamp<int>(int(255.99f * res[i * channels]), 0, 255);
            pixels[i * channels + 1] = glm::clamp<int>(int(255.99f * res[i * channels + 1]), 0, 255);
            pixels[i * channels + 2] = glm::clamp<int>(int(255.99f * res[i * channels + 2]), 0, 255);
            if (channels == 4)
                pixels[i * channels + 3] = glm::clamp<int>(int(255.99f * res[i * channels + 3]), 0, 255);
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_png(path.c_str(), resizeX, resizeY, channels, pixels.data(), resizeX * channels);
    }
    else
    {
        pixels.resize(resolutionX * resolutionY * channels);
        for (int i = 0; i < resolutionX * resolutionY; i++)
        {
            pixels[i * channels] = glm::clamp<int>(int(255.99f * dst[i * channels]), 0, 255);
            pixels[i * channels + 1] = glm::clamp<int>(int(255.99f * dst[i * channels + 1]), 0, 255);
            pixels[i * channels + 2] = glm::clamp<int>(int(255.99f * dst[i * channels + 2]), 0, 255);
            if (channels == 4)
                pixels[i * channels + 3] = glm::clamp<int>(int(255.99f * dst[i * channels + 3]), 0, 255);
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_png(path.c_str(), resolutionX, resolutionY, channels, pixels.data(), resolutionX * channels);
    }
}

void Texture2D::StoreToJpg(const std::string &path, int resizeX, int resizeY, unsigned quality) const
{
    const auto resolutionX = m_texture->m_width;
    const auto resolutionY = m_texture->m_height;
    std::vector<float> dst;
    dst.resize(resolutionX * resolutionY * 3);
    m_texture->Bind(0);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, (void *)dst.data());
    std::vector<uint8_t> pixels;
    if (resizeX > 0 && resizeY > 0 && (resizeX != resolutionX || resizeY != resolutionY))
    {
        std::vector<float> res;
        res.resize(resizeX * resizeY * 3);
        stbir_resize_float(dst.data(), resolutionX, resolutionY, 0, res.data(), resizeX, resizeY, 0, 3);
        pixels.resize(resizeX * resizeY * 3);
        for (int i = 0; i < resizeX * resizeY; i++)
        {
            pixels[i * 3] = glm::clamp<int>(int(255.99f * res[i * 3]), 0, 255);
            pixels[i * 3 + 1] = glm::clamp<int>(int(255.99f * res[i * 3 + 1]), 0, 255);
            pixels[i * 3 + 2] = glm::clamp<int>(int(255.99f * res[i * 3 + 2]), 0, 255);
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_jpg(path.c_str(), resizeX, resizeY, 3, pixels.data(), quality);
    }
    else
    {
        pixels.resize(resolutionX * resolutionY * 3);
        for (int i = 0; i < resolutionX * resolutionY; i++)
        {
            pixels[i * 3] = glm::clamp<int>(int(255.99f * dst[i * 3]), 0, 255);
            pixels[i * 3 + 1] = glm::clamp<int>(int(255.99f * dst[i * 3 + 1]), 0, 255);
            pixels[i * 3 + 2] = glm::clamp<int>(int(255.99f * dst[i * 3 + 2]), 0, 255);
        }
        stbi_flip_vertically_on_write(true);
        stbi_write_jpg(path.c_str(), resolutionX, resolutionY, 3, pixels.data(), quality);
    }
}

std::shared_ptr<OpenGLUtils::GLTexture2D>& Texture2D::UnsafeGetGLTexture()
{
    return m_texture;
}

void Texture2D::Load(const std::filesystem::path &path)
{
    stbi_set_flip_vertically_on_load(true);
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
        GLenum format = GL_RED;
        if (nrComponents == 2)
        {
            format = GL_RG;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
        }
        GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(width, height))) + 1;
        m_texture = std::make_shared<OpenGLUtils::GLTexture2D>(mipmap, GL_RGBA32F, width, height, true);
        m_texture->SetData(0, format, GL_FLOAT, data);
        m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_REPEAT);
        m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_REPEAT);
        m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_texture->GenerateMipMap();
        stbi_image_free(data);
    }
    else
    {
        UNIENGINE_LOG("Texture failed to load at path: " + path.filename().string());
        stbi_image_free(data);
        return;
    }
    m_name = path.filename().string();
    m_gamma = actualGamma;
}
void Texture2D::Save(const std::filesystem::path &path)
{
    if(path.extension() == ".png"){
        StoreToPng(path.string());
    }else if(path.extension() == ".jpg"){
        StoreToJpg(path.string());
    }
}
