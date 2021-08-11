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
class UNIENGINE_API Texture2D : public IAsset
{
    friend class Material;
    friend class RenderManager;
    friend class Bloom;
    std::shared_ptr<OpenGLUtils::GLTexture2D> m_texture;
    friend class AssetManager;
    friend class Camera;
    friend class DefaultResources;

    friend class LightProbe;
    friend class ReflectionProbe;
    friend class EnvironmentalMap;
    friend class Cubemap;
  public:
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
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLTexture2D> Texture() const;

    void Load(const std::filesystem::path & path) override;
};
} // namespace UniEngine
