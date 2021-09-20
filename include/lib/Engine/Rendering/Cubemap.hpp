#pragma once
#include <Core/OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API Cubemap : public IAsset
{
    friend class AssetManager;
    friend class LightProbe;
    friend class ReflectionProbe;
    friend class DefaultResources;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> m_texture;

  protected:
    bool LoadInternal(const std::filesystem::path & path) override;

  public:
    float m_gamma = 1.0f;
    void OnCreate() override;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Texture();


};
} // namespace UniEngine
