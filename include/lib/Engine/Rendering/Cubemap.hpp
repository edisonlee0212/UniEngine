#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Texture2D.hpp>
namespace UniEngine
{
class UNIENGINE_API Cubemap : public IAsset
{
    friend class LightProbe;
    friend class ReflectionProbe;
    friend class DefaultResources;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> m_texture;

  public:
    void ConvertFromEquirectangularTexture(const std::shared_ptr<Texture2D> &targetTexture);

    float m_gamma = 1.0f;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Texture();
    void OnInspect() override;

};
} // namespace UniEngine
