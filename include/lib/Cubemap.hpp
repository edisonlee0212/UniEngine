#pragma once
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API Cubemap : public ResourceBehaviour
{
    friend class ResourceManager;
    friend class LightProbe;
    friend class ReflectionProbe;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> m_texture;

  public:
    float m_gamma = 1.0f;
    void OnCreate() override;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Texture();
};
} // namespace UniEngine
