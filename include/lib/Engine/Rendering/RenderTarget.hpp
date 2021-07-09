#pragma once
#include <Core/OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API RenderTarget
{
    bool m_bound;

  protected:
    std::unique_ptr<OpenGLUtils::GLFrameBuffer> m_frameBuffer;
    size_t m_resolutionX;
    size_t m_resolutionY;

  public:
    void AttachTextureLayer(OpenGLUtils::GLTexture *texture, const GLenum &attachPoint, const GLint &layer) const;
    void AttachTexture(OpenGLUtils::GLTexture *texture, const GLenum &attachPoint) const;
    void AttachTexture2D(
        OpenGLUtils::GLTexture *texture,
        const GLenum &attachPoint,
        const GLenum &texTarget,
        const GLint &level = 0) const;
    void AttachRenderBuffer(OpenGLUtils::GLRenderBuffer *renderBuffer, const GLenum &attachPoint) const;
    RenderTarget();
    RenderTarget(size_t width, size_t height);
    glm::vec2 GetResolution() const;
    void SetResolution(size_t width, size_t height);
    float GetResolutionRatio() const;
    void Bind() const;
    void Clear() const;
    std::unique_ptr<OpenGLUtils::GLFrameBuffer> &GetFrameBuffer();
    static void BindDefault();
};
} // namespace UniEngine