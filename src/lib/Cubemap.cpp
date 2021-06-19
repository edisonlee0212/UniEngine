#include <Cubemap.hpp>
using namespace UniEngine;

Cubemap::Cubemap()
{
    m_name = "New Cubemap";
}

std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Cubemap::Texture()
{
    return m_texture;
}

std::vector<std::string> Cubemap::Paths() const
{
    return m_paths;
}
