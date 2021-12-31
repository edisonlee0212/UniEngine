#include "Engine/Utilities/Console.hpp"
#include <RenderTarget.hpp>
using namespace UniEngine;
RenderTarget::RenderTarget()
{
    m_bound = false;
    m_frameBuffer = std::make_unique<OpenGLUtils::GLFrameBuffer>();
    m_resolutionX = 0;
    m_resolutionY = 0;
}

RenderTarget::RenderTarget(size_t width, size_t height)
{
    m_bound = false;
    m_frameBuffer = std::make_unique<OpenGLUtils::GLFrameBuffer>();
    m_resolutionX = width;
    m_resolutionY = height;
}

glm::vec2 RenderTarget::GetResolution() const
{
    return glm::vec2(m_resolutionX, m_resolutionY);
}

void RenderTarget::SetResolution(size_t width, size_t height)
{
    m_resolutionX = width;
    m_resolutionY = height;
}

float RenderTarget::GetResolutionRatio() const
{
    if (m_resolutionX == 0 || m_resolutionY == 0)
        return 0;
    return (float)m_resolutionX / (float)m_resolutionY;
}

void RenderTarget::AttachTextureLayer(
    OpenGLUtils::GLTexture *texture, const GLenum &attachPoint, const GLint &layer) const
{
    if (m_bound)
    {
        UNIENGINE_ERROR("Error");
        return;
    }
    m_frameBuffer->AttachTextureLayer(texture, attachPoint, layer);
}

void RenderTarget::AttachTexture(OpenGLUtils::GLTexture *texture, const GLenum &attachPoint) const
{
    if (m_bound)
    {
        UNIENGINE_ERROR("Error");
        return;
    }
    m_frameBuffer->AttachTexture(texture, attachPoint);
}

void RenderTarget::AttachTexture2D(
    OpenGLUtils::GLTexture *texture, const GLenum &attachPoint, const GLenum &texTarget, const GLint &level) const
{
    if (m_bound)
    {
        UNIENGINE_ERROR("Error");
        return;
    }
    m_frameBuffer->AttachTexture2D(texture, attachPoint, texTarget, level);
}

void RenderTarget::AttachRenderBuffer(OpenGLUtils::GLRenderBuffer *renderBuffer, const GLenum &attachPoint) const
{
    if (m_bound)
    {
        UNIENGINE_ERROR("Error");
        return;
    }
    m_frameBuffer->AttachRenderBuffer(renderBuffer, attachPoint);
}

void RenderTarget::Bind() const
{
    if (m_bound)
    {
        UNIENGINE_ERROR("Error");
        return;
    }
    m_frameBuffer->Bind();
    m_frameBuffer->Check();
    OpenGLUtils::SetViewPort(m_resolutionX, m_resolutionY);
}

void RenderTarget::Clear() const
{
    m_frameBuffer->Clear();
}

std::unique_ptr<OpenGLUtils::GLFrameBuffer> &RenderTarget::GetFrameBuffer()
{
    return m_frameBuffer;
}

void RenderTarget::BindDefault()
{
    OpenGLUtils::GLFrameBuffer::BindDefault();
}
