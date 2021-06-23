#pragma once
#include <OpenGLUtils.hpp>
namespace UniEngine
{
enum class UNIENGINE_API TextureType
{
    Albedo,
    Normal,
    Metallic,
    Roughness,
    AO,
};
class UNIENGINE_API Texture2D : public ResourceBehaviour
{
    friend class Material;
    friend class RenderManager;
    friend class Bloom;
    TextureType m_type;
    std::shared_ptr<OpenGLUtils::GLTexture2D> m_texture;
    std::string m_path;
    friend class ResourceManager;
    friend class EnvironmentalMap;
    friend class CameraComponent;

  public:
    Texture2D(TextureType type = TextureType::Albedo);
    void SetType(TextureType type);
    [[nodiscard]] TextureType GetType() const;
    [[nodiscard]] glm::vec2 GetResolution() const;
    void StoreToPng(
        const std::string &path,
        int resizeX = -1,
        int resizeY = -1,
        bool alphaChannel = false,
        unsigned compressionLevel = 8) const;
    void StoreToJpg(const std::string &path, int resizeX = -1, int resizeY = -1, unsigned quality = 100) const;
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLTexture2D> Texture() const;
    [[nodiscard]] std::string Path() const;
};
} // namespace UniEngine
