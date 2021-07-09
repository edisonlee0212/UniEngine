#include <Cubemap.hpp>
using namespace UniEngine;

void Cubemap::OnCreate()
{
    m_name = "New Cubemap";
}

std::unique_ptr<OpenGLUtils::GLTextureCubeMap> &Cubemap::Texture()
{
    return m_texture;
}
