#pragma once
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API Cubemap : public ResourceBehaviour
{
    friend class ResourceManager;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> m_texture;
  public:
    Cubemap();
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Texture();
    [[nodiscard]] std::vector<std::string> Paths() const;
};
} // namespace UniEngine
