#pragma once
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API Cubemap : public ResourceBehaviour
{
    friend class ResourceManager;
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> m_texture;
    std::vector<std::string> m_paths;

  public:
    Cubemap();
    std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Texture();
    [[nodiscard]] std::vector<std::string> Paths() const;
};
} // namespace UniEngine
