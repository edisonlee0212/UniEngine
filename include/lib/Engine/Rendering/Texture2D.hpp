#pragma once
#include <Core/OpenGLUtils.hpp>
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

enum class UNIENGINE_API TextureColorType{
    Red = 1,
    RG = 2,
    RGB = 3,
    RGBA = 4
};
class UNIENGINE_API Texture2D : public IAsset
{
    friend class Material;
    friend class Graphics;
    friend class Bloom;
    std::shared_ptr<OpenGLUtils::GLTexture2D> m_texture;
    friend class AssetManager;
    friend class Camera;
    friend class DefaultResources;
    friend class RenderLayer;
    friend class LightProbe;
    friend class ReflectionProbe;
    friend class EnvironmentalMap;
    friend class Cubemap;
  protected:
    bool SaveInternal(const std::filesystem::path &path) override;
    bool LoadInternal(const std::filesystem::path & path) override;
  public:
    TextureColorType m_textureColorType;
    void OnInspect() override;
    float m_gamma = 1.0f;
    void OnCreate() override;
    [[nodiscard]] glm::vec2 GetResolution() const;
    void StoreToPng(
        const std::string &path,
        int resizeX = -1,
        int resizeY = -1,
        bool alphaChannel = false,
        unsigned compressionLevel = 8) const;
    void StoreToJpg(const std::string &path, int resizeX = -1, int resizeY = -1, unsigned quality = 100) const;
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLTexture2D>& UnsafeGetGLTexture();

};
} // namespace UniEngine
