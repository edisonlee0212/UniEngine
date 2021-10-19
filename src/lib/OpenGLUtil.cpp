#include <ConsoleManager.hpp>
#include <EditorManager.hpp>
#include <OpenGLUtils.hpp>
using namespace UniEngine;

void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam);

void OpenGLUtils::Init()
{
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        UNIENGINE_ERROR("Failed to initialize GLAD");
        exit(-1);
    }
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &GLTexture::m_maxAllowedTexture);
    GLTexture::m_currentBoundTextures.resize(GLTexture::m_maxAllowedTexture);

    // enable OpenGL debug context if context allows for debug context

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

GLint OpenGLUtils::GLTexture::m_maxAllowedTexture = 0;
std::vector<std::pair<GLenum, GLuint>> OpenGLUtils::GLTexture::m_currentBoundTextures;
std::map<GLenum, GLuint> OpenGLUtils::GLBuffer::m_boundBuffers;
GLuint OpenGLUtils::GLVAO::m_boundVAO = 0;
GLuint OpenGLUtils::GLProgram::m_boundProgram = 0;
GLuint OpenGLUtils::GLFrameBuffer::m_boundFrameBuffer = 0;
GLuint OpenGLUtils::GLRenderBuffer::m_boundRenderBuffer = 0;
void OpenGLUtils::PreUpdate()
{
    for (auto &i : GLTexture::m_currentBoundTextures)
    {
        i.first = -1;
        i.second = -1;
    }

    GLBuffer::m_boundBuffers.clear();
    GLVAO::m_boundVAO = 0;
    GLProgram::m_boundProgram = 0;
    GLFrameBuffer::m_boundFrameBuffer = 0;
    GLRenderBuffer::m_boundRenderBuffer = 0;

    auto& utils = GetInstance();
    utils.m_depthTest = false;
    utils.m_scissorTest = false;
    utils.m_stencilTest = false;
    utils.m_blend = false;
    utils.m_cullFace = false;
    SetEnable(OpenGLCapability::DepthTest, true);
    SetEnable(OpenGLCapability::ScissorTest, false);
    SetEnable(OpenGLCapability::StencilTest, true);
    SetEnable(OpenGLCapability::Blend, true);
    SetEnable(OpenGLCapability::CullFace, true);
}
void OpenGLUtils::SetEnable(OpenGLCapability capability, bool enable)
{
    auto& utils = GetInstance();
    switch(capability){
    case OpenGLCapability::DepthTest:
        if(utils.m_depthTest != enable){
            if(enable){
                glEnable(GL_DEPTH_TEST);
            }else{
                glDisable(GL_DEPTH_TEST);
            }
        }
        utils.m_depthTest = enable;
        break;
    case OpenGLCapability::ScissorTest:
        if(utils.m_scissorTest != enable){
            if(enable){
                glEnable(GL_SCISSOR_TEST);
            }else{
                glDisable(GL_SCISSOR_TEST);
            }
        }
        utils.m_scissorTest = enable;
        break;
    case OpenGLCapability::StencilTest:
        if(utils.m_stencilTest != enable){
            if(enable){
                glEnable(GL_STENCIL_TEST);
            }else{
                glDisable(GL_STENCIL_TEST);
            }
        }
        utils.m_stencilTest = enable;
        break;
    case OpenGLCapability::Blend:
        if(utils.m_blend != enable){
            if(enable){
                glEnable(GL_BLEND);
            }else{
                glDisable(GL_BLEND);
            }
        }
        utils.m_blend = enable;
        break;
    case OpenGLCapability::CullFace:
        if(utils.m_cullFace != enable){
            if(enable){
                glEnable(GL_CULL_FACE);
            }else{
                glDisable(GL_CULL_FACE);
            }
        }
        utils.m_cullFace = enable;
        break;
    }
}

GLuint OpenGLUtils::GLObject::Id() const
{
    return m_id;
}

OpenGLUtils::GLBuffer::GLBuffer(GLenum target)
{
    glGenBuffers(1, &m_id);
    m_target = target;
}

void OpenGLUtils::GLBuffer::Bind() const
{
    const auto search = m_boundBuffers.find(m_target);
    if (search != m_boundBuffers.end() && search->second == m_id)
    {
        return;
    }
    m_boundBuffers[m_target] = m_id;
    glBindBuffer(m_target, m_id);
}

void OpenGLUtils::GLBuffer::SetData(const GLsizei &length, const GLvoid *data, const GLenum &usage) const
{
    Bind();
    glNamedBufferData(m_id, length, data, usage);
}

void OpenGLUtils::GLBuffer::SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) const
{
    Bind();
    glNamedBufferSubData(m_id, offset, size, data);
}

OpenGLUtils::GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &m_id);
}

OpenGLUtils::GLSSBO::GLSSBO() : GLBuffer(GL_SHADER_STORAGE_BUFFER)
{
}

void OpenGLUtils::GLSSBO::SetBase(const GLuint &index) const
{
    glBindBufferBase(m_target, index, m_id);
}

void OpenGLUtils::GLSSBO::BindDefault()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

OpenGLUtils::GLPPBO::GLPPBO() : GLBuffer(GL_PIXEL_PACK_BUFFER)
{
}

void OpenGLUtils::GLPPBO::BindDefault()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

OpenGLUtils::GLPUBO::GLPUBO() : GLBuffer(GL_PIXEL_UNPACK_BUFFER)
{
}

void OpenGLUtils::GLPUBO::BindDefault()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

OpenGLUtils::GLEBO::GLEBO() : GLBuffer(GL_ELEMENT_ARRAY_BUFFER)
{
}

void OpenGLUtils::GLEBO::BindDefault()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

OpenGLUtils::GLVBO::GLVBO() : GLBuffer(GL_ARRAY_BUFFER)
{
}

void OpenGLUtils::GLVBO::BindDefault()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OpenGLUtils::GLUBO::GLUBO() : GLBuffer(GL_UNIFORM_BUFFER)
{
}

void OpenGLUtils::GLUBO::BindDefault()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLUtils::GLUBO::SetBase(const GLuint &index) const
{
    glBindBufferBase(m_target, index, m_id);
}

void OpenGLUtils::GLUBO::SetRange(const GLuint &index, const GLintptr &offset, const GLsizeiptr &size) const
{
    glBindBufferRange(m_target, index, m_id, offset, size);
}

OpenGLUtils::GLVAO::~GLVAO()
{
    BindDefault();
    glDeleteVertexArrays(1, &m_id);
}

void OpenGLUtils::GLVAO::Bind() const
{
    if (m_boundVAO == m_id)
        return;
    m_boundVAO = m_id;
    glBindVertexArray(m_id);
}

void OpenGLUtils::GLVAO::BindDefault()
{
    if (m_boundVAO == 0)
        return;
    m_boundVAO = 0;
    glBindVertexArray(0);
}

OpenGLUtils::GLVAO::GLVAO()
{
    glGenVertexArrays(1, &m_id);
}

OpenGLUtils::GLVBO *OpenGLUtils::GLVAO::Vbo()
{
    return &m_vbo;
}

OpenGLUtils::GLEBO *OpenGLUtils::GLVAO::Ebo()
{
    return &m_ebo;
}

void OpenGLUtils::GLVAO::SetData(const GLsizei &length, const GLvoid *data, const GLenum &usage) const
{
    Bind();
    m_vbo.SetData(length, data, usage);
}

void OpenGLUtils::GLVAO::SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) const
{
    Bind();
    m_vbo.SubData(offset, size, data);
}

void OpenGLUtils::GLVAO::EnableAttributeArray(const GLuint &index)
{
    glEnableVertexAttribArray(index);
}

void OpenGLUtils::GLVAO::DisableAttributeArray(const GLuint &index)
{
    glDisableVertexAttribArray(index);
}

void OpenGLUtils::GLVAO::SetAttributePointer(
    const GLuint &index,
    const GLint &size,
    const GLenum &type,
    const GLboolean &normalized,
    const GLsizei &stride,
    const void *pointer) const
{
    Bind();
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void OpenGLUtils::GLVAO::SetAttributeIntPointer(
    const GLuint &index, const GLint &size, const GLenum &type, const GLsizei &stride, const void *pointer) const
{
    Bind();
    glVertexAttribIPointer(index, size, type, stride, pointer);
}

void OpenGLUtils::GLVAO::SetAttributeDivisor(const GLuint &index, const GLuint &divisor) const
{
    Bind();
    glVertexAttribDivisor(index, divisor);
}

OpenGLUtils::TextureBinding::TextureBinding()
{
    m_1d = 0;
    m_2d = 0;
    m_3d = 0;
    m_rectangle = 0;
    m_buffer = 0;
    m_cubeMap = 0;
    m_1dArray = 0;
    m_2dArray = 0;
    m_cubeMapArray = 0;
    m_2dMS = 0;
    m_2dMSArray = 0;
}

GLint OpenGLUtils::GLTexture::GetMaxAllowedTexture()
{
    return m_maxAllowedTexture;
}

void OpenGLUtils::GLTexture::Clear(const GLint &level) const
{
    Bind(0);
    glClearTexImage(m_id, level, m_format, m_type, nullptr);
}

void OpenGLUtils::GLTexture::SetInt(const GLenum &paramName, const GLint &param)
{
    Bind(0);
    glTextureParameteri(m_id, paramName, param);
}

void OpenGLUtils::GLTexture::SetFloat(const GLenum &paramName, const GLfloat &param)
{
    Bind(0);
    glTextureParameterf(m_id, paramName, param);
}

void OpenGLUtils::GLTexture::SetFloat4(const GLenum &paramName, const GLfloat *params)
{
    Bind(0);
    glTextureParameterfv(m_id, paramName, params);
}

void OpenGLUtils::GLTexture::GenerateMipMap() const
{
    Bind(0);
    glGenerateTextureMipmap(m_id);
}

void OpenGLUtils::GLTexture::Bind(const GLenum &activate) const
{
    if ((GLint)activate >= m_maxAllowedTexture)
    {
        UNIENGINE_ERROR("Max allowed texture exceeded!");
    }
    if (m_currentBoundTextures[activate].first == m_type && m_currentBoundTextures[activate].second == m_id)
        return;
    glActiveTexture(GL_TEXTURE0 + activate);
    glBindTexture(m_type, m_id);

    m_currentBoundTextures[activate].first = m_type;
    m_currentBoundTextures[activate].second = m_id;
}

OpenGLUtils::GLTexture::GLTexture(const GLenum &target, const GLenum &format)
{
    glGenTextures(1, &m_id);
    m_type = target;
    m_format = format;
}

OpenGLUtils::GLTexture::~GLTexture()
{
    glDeleteTextures(1, &m_id);
}

OpenGLUtils::GLTexture1D::GLTexture1D(
    const GLsizei &levels, const GLenum &internalFormat, const GLsizei &width, const bool &immutable)
    : GLTexture(GL_TEXTURE_1D, internalFormat)
{
    m_width = width;
    Bind(0);
    if (immutable)
        glTexStorage1D(m_type, levels, internalFormat, width);
}

void OpenGLUtils::GLTexture1D::SetData(const GLint &level, const GLenum &type, const void *pixels) const
{
    Bind(0);
    glTexSubImage1D(m_type, level, 0, m_width, m_format, type, pixels);
}

void OpenGLUtils::GLTexture1D::SubData(
    const GLint &level, const GLint &xOffset, const GLenum &type, const GLsizei &width, const void *pixels) const
{
    if (xOffset + width > m_width)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage1D(m_type, level, xOffset, width, m_format, type, pixels);
}

OpenGLUtils::GLTexture2D::GLTexture2D(
    const GLsizei &levels,
    const GLenum &internalFormat,
    const GLsizei &width,
    const GLsizei &height,
    const bool &immutable)
    : GLTexture(GL_TEXTURE_2D, internalFormat), m_height(height)
{
    m_width = width;
    m_height = height;
    m_immutable = immutable;
    Bind(0);
    if (immutable)
        glTextureStorage2D(m_id, levels, internalFormat, width, height);
}

void OpenGLUtils::GLTexture2D::SetData(
    const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    if (!m_immutable)
    {
        UNIENGINE_ERROR("GLTexture2D: Not Immutable!");
        return;
    }
    Bind(0);
    glTextureSubImage2D(m_id, level, 0, 0, m_width, m_height, format, type, pixels);
}

void OpenGLUtils::GLTexture2D::ReSize(
    const GLint &level,
    const GLenum &internalFormat,
    const GLenum &format,
    const GLenum &type,
    const void *pixels,
    const GLsizei &width,
    const GLsizei &height)
{
    if (m_immutable)
    {
        UNIENGINE_ERROR("GLTexture2D: Immutable!");
        return;
    }
    m_width = width;
    m_height = height;
    SetData(level, internalFormat, format, type, pixels);
}

void OpenGLUtils::GLTexture2D::SetData(
    const GLint &level,
    const GLenum &internalFormat,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if (m_immutable)
    {
        UNIENGINE_ERROR("GLTexture2D: Immutable!");
        return;
    }
    Bind(0);
    glTexImage2D(m_type, level, internalFormat, m_width, m_height, 0, format, type, pixels);
}

void OpenGLUtils::GLTexture2D::SubData(
    const GLint &level,
    const GLenum &format,
    const GLint &xOffset,
    const GLint &yOffset,
    const GLsizei &width,
    const GLsizei &height,
    const GLenum &type,
    const void *pixels) const
{
    if ((xOffset + width) > m_width || (yOffset + height) > m_height)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage2D(m_type, level, xOffset, yOffset, width, height, format, type, pixels);
}
glm::ivec2 OpenGLUtils::GLTexture2D::GetSize() const
{
    return glm::ivec2(m_width, m_height);
}

OpenGLUtils::GLTexture3D::GLTexture3D(
    const GLsizei &levels,
    const GLenum &internalFormat,
    const GLsizei &width,
    const GLsizei &height,
    const GLsizei &depth,
    const bool &immutable)
    : GLTexture(GL_TEXTURE_3D, internalFormat)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
    Bind(0);
    if (immutable)
        glTexStorage3D(m_type, levels, internalFormat, width, height, depth);
}

void OpenGLUtils::GLTexture3D::SetData(
    const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    Bind(0);
    glTexSubImage3D(m_type, level, 0, 0, 0, m_width, m_height, m_depth, format, type, pixels);
}

void OpenGLUtils::GLTexture3D::SubData(
    const GLint &level,
    const GLint &xOffset,
    const GLint &yOffset,
    const GLint &zOffset,
    const GLsizei &width,
    const GLsizei &height,
    const GLsizei &depth,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if ((xOffset + width) > m_width || (yOffset + height) > m_height || (zOffset + depth) > m_depth)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage3D(m_type, level, xOffset, yOffset, zOffset, width, height, depth, format, type, pixels);
}

OpenGLUtils::GLTextureCubeMap::GLTextureCubeMap(
    const GLsizei &levels,
    const GLenum &internalFormat,
    const GLsizei &width,
    const GLsizei &height,
    const bool &immutable)
    : GLTexture(GL_TEXTURE_CUBE_MAP, internalFormat)
{
    m_width = width;
    m_height = height;
    Bind(0);
    if (immutable)
        glTexStorage2D(m_type, levels, internalFormat, width, height);
}

void OpenGLUtils::GLTextureCubeMap::SetData(
    const CubeMapIndex &index, const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    if ((size_t)index > 5)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTextureSubImage3D(m_id, level, 0, 0, (size_t)index, m_width, m_height, 1, format, type, pixels);
}

void OpenGLUtils::GLTextureCubeMap::SetData(
    const CubeMapIndex &index,
    const GLint &level,
    const GLenum &internalFormat,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if ((size_t)index > 5)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)index, level, internalFormat, m_width, m_height, 0, format, type, pixels);
}

void OpenGLUtils::GLTextureCubeMap::SubData(
    const CubeMapIndex &index,
    const GLint &level,
    const GLint &xOffset,
    const GLint &yOffset,
    const GLsizei &width,
    const GLsizei &height,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if ((xOffset + width) > m_width || (yOffset + height) > m_height || (size_t)index > 5)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage3D(m_type, level, xOffset, yOffset, (size_t)index, width, height, 1, format, type, pixels);
}

OpenGLUtils::GLTexture1DArray::GLTexture1DArray(
    const GLsizei &levels, const GLenum &internalFormat, const GLsizei &width, const GLsizei &layers)
    : GLTexture(GL_TEXTURE_1D_ARRAY, internalFormat)
{
    m_width = width;
    m_layers = layers;
    Bind(0);
    glTexStorage2D(m_type, levels, internalFormat, width, layers);
}

void OpenGLUtils::GLTexture1DArray::SetData(
    const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    Bind(0);
    glTexSubImage2D(m_type, level, 0, 0, m_width, m_layers, format, type, pixels);
}

void OpenGLUtils::GLTexture1DArray::SubData(
    const GLint &level,
    const GLint &xOffset,
    const GLint &layer,
    const GLsizei &width,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    Bind(0);
    glTexSubImage2D(m_type, level, xOffset, layer, m_width, 1, format, type, pixels);
}

OpenGLUtils::GLTexture2DArray::GLTexture2DArray(
    const GLsizei &levels,
    const GLenum &internalFormat,
    const GLsizei &width,
    const GLsizei &height,
    const GLsizei &layers)
    : GLTexture(GL_TEXTURE_2D_ARRAY, internalFormat)
{
    m_width = width;
    m_height = height;
    m_layers = layers;
    Bind(0);
    glTexStorage3D(m_type, levels, internalFormat, width, height, layers);
}

void OpenGLUtils::GLTexture2DArray::SetData(
    const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    Bind(0);
    glTexSubImage3D(m_type, level, 0, 0, 0, m_width, m_height, m_layers, format, type, pixels);
}

void OpenGLUtils::GLTexture2DArray::SubData(
    const GLint &level,
    const GLint &xOffset,
    const GLint &yOffset,
    const GLsizei &layer,
    const GLsizei &width,
    const GLsizei &height,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if ((xOffset + width) > m_width || (yOffset + height) > m_height || layer >= m_layers)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage3D(m_type, level, xOffset, yOffset, layer, width, height, 1, format, type, pixels);
}

OpenGLUtils::GLTextureCubeMapArray::GLTextureCubeMapArray(
    const GLsizei &levels,
    const GLenum &internalFormat,
    const GLsizei &width,
    const GLsizei &height,
    const GLsizei &layers)
    : GLTexture(GL_TEXTURE_CUBE_MAP_ARRAY, internalFormat)
{
    m_width = width;
    m_height = height;
    m_layers = layers;
    Bind(0);
    glTexStorage3D(m_type, levels, internalFormat, width, height, layers * 6);
}

void OpenGLUtils::GLTextureCubeMapArray::SetData(
    const CubeMapIndex &index, const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const
{
    if ((size_t)index > 5)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage3D(m_type, level, 0, 0, 0, m_width, m_height, m_layers * 6, format, type, pixels);
}

void OpenGLUtils::GLTextureCubeMapArray::SubData(
    const CubeMapIndex &index,
    const GLint &level,
    const GLint &xOffset,
    const GLint &yOffset,
    const GLsizei &layer,
    const GLsizei &width,
    const GLsizei &height,
    const GLenum &format,
    const GLenum &type,
    const void *pixels) const
{
    if ((xOffset + width) > m_width || (yOffset + height) > m_height || (size_t)index > 5)
    {
        UNIENGINE_ERROR("Error!");
    }
    Bind(0);
    glTexSubImage3D(m_type, level, xOffset, yOffset, layer * 6 + (size_t)index, width, height, 1, format, type, pixels);
}

void OpenGLUtils::GLRenderBuffer::Bind()
{
    if (m_boundRenderBuffer == m_id)
        return;
    m_boundRenderBuffer = m_id;
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
}

void OpenGLUtils::GLRenderBuffer::BindDefault()
{
    if (m_boundRenderBuffer == 0)
        return;
    m_boundRenderBuffer = 0;
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

OpenGLUtils::GLRenderBuffer::GLRenderBuffer()
{
    glGenRenderbuffers(1, &m_id);
}

OpenGLUtils::GLRenderBuffer::~GLRenderBuffer()
{
    BindDefault();
    glDeleteRenderbuffers(1, &m_id);
}

void OpenGLUtils::GLRenderBuffer::AllocateStorage(GLenum internalFormat, GLsizei width, GLsizei height)
{
    Bind();
    glNamedRenderbufferStorage(m_id, internalFormat, width, height);
}

void OpenGLUtils::GLFrameBuffer::Enable(const GLenum &cap)
{
    glEnable(cap);
}

void OpenGLUtils::GLFrameBuffer::Disable(const GLenum &cap)
{
    glDisable(cap);
}

void OpenGLUtils::GLFrameBuffer::Bind() const
{
    if (m_boundFrameBuffer == m_id)
        return;
    m_boundFrameBuffer = m_id;
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void OpenGLUtils::GLFrameBuffer::ClearColor(const glm::vec4 &value) const
{
    Bind();
    glClearColor(value.r, value.g, value.b, value.a);
}

void OpenGLUtils::GLFrameBuffer::ViewPort(const glm::ivec4 &value) const
{
    Bind();
    glViewport(value[0], value[1], value[2], value[3]);
}

void OpenGLUtils::GLFrameBuffer::ViewPort(size_t x, size_t y) const
{
    Bind();
    glViewport(0, 0, x, y);
}

void OpenGLUtils::GLFrameBuffer::Check() const
{
    Bind();
    const auto status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        UNIENGINE_ERROR("GLFrameBuffer: Not Complete!");
}

void OpenGLUtils::GLFrameBuffer::DrawBuffer(const GLenum &buffer) const
{
    Bind();
    glNamedFramebufferDrawBuffer(m_id, buffer);
}

void OpenGLUtils::GLFrameBuffer::DrawBuffers(const GLsizei &n, const GLenum *buffers) const
{
    Bind();
    glNamedFramebufferDrawBuffers(m_id, n, buffers);
}

void OpenGLUtils::GLFrameBuffer::BindDefault()
{
    if (m_boundFrameBuffer == 0)
        return;
    m_boundFrameBuffer = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLUtils::GLFrameBuffer::GLFrameBuffer()
{
    m_color = false;
    m_depth = false;
    m_stencil = false;
    glCreateFramebuffers(1, &m_id);
}

OpenGLUtils::GLFrameBuffer::~GLFrameBuffer()
{
    BindDefault();
    glDeleteFramebuffers(1, &m_id);
}

bool OpenGLUtils::GLFrameBuffer::Color() const
{
    return m_color;
}

bool OpenGLUtils::GLFrameBuffer::Depth() const
{
    return m_depth;
}

bool OpenGLUtils::GLFrameBuffer::Stencil() const
{
    return m_stencil;
}

void OpenGLUtils::GLFrameBuffer::AttachRenderBuffer(const GLRenderBuffer *buffer, const GLenum &attachPoint)
{
    switch (attachPoint)
    {
    case GL_DEPTH_ATTACHMENT:
        m_depth = true;
        break;
    case GL_STENCIL_ATTACHMENT:
        m_stencil = true;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        m_depth = true;
        m_stencil = true;
        break;
    default:
        m_color = true;
        break;
    }
    Bind();
    glNamedFramebufferRenderbuffer(m_id, attachPoint, GL_RENDERBUFFER, buffer->Id());
}

void OpenGLUtils::GLFrameBuffer::AttachTexture(const GLTexture *texture, const GLenum &attachPoint)
{
    switch (attachPoint)
    {
    case GL_DEPTH_ATTACHMENT:
        m_depth = true;
        break;
    case GL_STENCIL_ATTACHMENT:
        m_stencil = true;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        m_depth = true;
        m_stencil = true;
        break;
    default:
        m_color = true;
        break;
    }
    Bind();
    glNamedFramebufferTexture(m_id, attachPoint, texture->Id(), 0);
}

void OpenGLUtils::GLFrameBuffer::AttachTextureLayer(
    const GLTexture *texture, const GLenum &attachPoint, const GLint &layer)
{
    switch (attachPoint)
    {
    case GL_DEPTH_ATTACHMENT:
        m_depth = true;
        break;
    case GL_STENCIL_ATTACHMENT:
        m_stencil = true;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        m_depth = true;
        m_stencil = true;
        break;
    default:
        m_color = true;
        break;
    }
    Bind();
    glNamedFramebufferTextureLayer(m_id, attachPoint, texture->Id(), 0, layer);
}

void OpenGLUtils::GLFrameBuffer::AttachTexture2D(
    const GLTexture *texture, const GLenum &attachPoint, const GLenum &texTarget, const GLint &level)
{
    switch (attachPoint)
    {
    case GL_DEPTH_ATTACHMENT:
        m_depth = true;
        break;
    case GL_STENCIL_ATTACHMENT:
        m_stencil = true;
        break;
    case GL_DEPTH_STENCIL_ATTACHMENT:
        m_depth = true;
        m_stencil = true;
        break;
    default:
        m_color = true;
        break;
    }
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachPoint, texTarget, texture->Id(), level);
}

void OpenGLUtils::GLFrameBuffer::Clear()
{
    Bind();
    glClear(
        (m_color ? GL_COLOR_BUFFER_BIT : 0) | (m_depth ? GL_DEPTH_BUFFER_BIT : 0) |
        (m_stencil ? GL_STENCIL_BUFFER_BIT : 0));
}

std::string OpenGLUtils::GLShader::GetCode() const
{
    return m_code;
}

OpenGLUtils::GLShader::~GLShader()
{
    if (m_id != 0)
        glDeleteShader(m_id);
}

OpenGLUtils::ShaderType OpenGLUtils::GLShader::Type() const
{
    return m_type;
}

bool OpenGLUtils::GLShader::Compiled() const
{
    return m_compiled;
}

void OpenGLUtils::GLShader::Attach(GLuint programID)
{
    glAttachShader(programID, m_id);
}

void OpenGLUtils::GLShader::Detach(GLuint programID) const
{
    glDetachShader(programID, m_id);
}
void OpenGLUtils::GLShader::OnCreate()
{
    m_id = 0;
    m_compiled = false;
}
void OpenGLUtils::GLShader::Compile()
{
    if (m_compiled)
        return;
    if (m_code.empty())
    {
        UNIENGINE_ERROR("Shader is empty!");
        return;
    }
    const char *ptr = m_code.c_str();
    glShaderSource(m_id, 1, &ptr, nullptr);
    glCompileShader(m_id);
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_id, 1024, NULL, infoLog);
        std::string type;
        switch (m_type)
        {
        case ShaderType::Vertex:
            type = "Vertex";
            break;
        case ShaderType::Geometry:
            type = "Geometry";
            break;
        case ShaderType::Fragment:
            type = "Fragment";
            break;
        case ShaderType::Tessellation:
            type = "Tessellation";
            break;
        case ShaderType::Compute:
            type = "Compute";
            break;
        default:
            break;
        }
        UNIENGINE_ERROR(
            "ERROR::SHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog +
            "\n -- --------------------------------------------------- -- ");
    }
    else
        m_compiled = true;
}
void OpenGLUtils::GLShader::Set(ShaderType type, const std::string &code)
{
    m_type = type;
    m_code = code;
    if (m_id != 0)
        glDeleteShader(m_id);
    switch (m_type)
    {
    case ShaderType::Vertex:
        m_id = glCreateShader(GL_VERTEX_SHADER);
        break;
    case ShaderType::Compute:
        m_id = glCreateShader(GL_COMPUTE_SHADER);
        break;
    case ShaderType::Tessellation:
        m_id = glCreateShader(GL_TESS_CONTROL_SHADER);
        break;
    case ShaderType::Geometry:
        m_id = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    case ShaderType::Fragment:
        m_id = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        break;
    }
    m_compiled = false;
}
void OpenGLUtils::GLShader::Serialize(YAML::Emitter &out)
{
    if (m_code.empty())
        out << YAML::Key << "m_code" << m_code;
    out << YAML::Key << "m_type" << (unsigned)m_type;
}
void OpenGLUtils::GLShader::Deserialize(const YAML::Node &in)
{
    if (in["m_code"])
        m_code = in["m_code"].as<std::string>();
    m_type = (ShaderType)in["m_type"].as<unsigned>();
    m_compiled = false;
}
void OpenGLUtils::GLShader::OnInspect()
{
    switch (m_type)
    {
    case ShaderType::Vertex:
        ImGui::Text("Type: Vertex");
        break;
    case ShaderType::Compute:
        ImGui::Text("Type: Compute");
        break;
    case ShaderType::Tessellation:
        ImGui::Text("Type: Tessellation");
        break;
    case ShaderType::Geometry:
        ImGui::Text("Type: Geometry");
        break;
    case ShaderType::Fragment:
        ImGui::Text("Type: Fragment");
        break;
    default:
        break;
    }
    ImGui::InputTextMultiline("Code", &m_code, ImVec2(0, 0), ImGuiInputTextFlags_AllowTabInput);
}

void OpenGLUtils::GLProgram::Bind()
{
    if (m_boundProgram == m_id)
        return;
    m_boundProgram = m_id;
    if (!m_linked)
        Link();
    glUseProgram(m_id);
}

void OpenGLUtils::GLProgram::BindDefault()
{
    if (m_boundProgram == 0)
        return;
    m_boundProgram = 0;
    glUseProgram(0);
}

void OpenGLUtils::GLProgram::OnCreate()
{
    m_name = "New Program";
}

OpenGLUtils::GLProgram::~GLProgram()
{
    BindDefault();
    glDeleteProgram(m_id);
}

std::shared_ptr<OpenGLUtils::GLShader> OpenGLUtils::GLProgram::GetShader(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        return m_vertexShader.Get<OpenGLUtils::GLShader>();
    case ShaderType::Tessellation:
        return m_tessellationShader.Get<OpenGLUtils::GLShader>();
    case ShaderType::Geometry:
        return m_geometryShader.Get<OpenGLUtils::GLShader>();
    case ShaderType::Fragment:
        return m_fragmentShader.Get<OpenGLUtils::GLShader>();
    case ShaderType::Compute:
        return m_computeShader.Get<OpenGLUtils::GLShader>();
    }
    return nullptr;
}

bool OpenGLUtils::GLProgram::HasShader(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:
        return m_vertexShader.Get<OpenGLUtils::GLShader>().get() != nullptr;
    case ShaderType::Tessellation:
        return m_tessellationShader.Get<OpenGLUtils::GLShader>().get() != nullptr;
    case ShaderType::Geometry:
        return m_geometryShader.Get<OpenGLUtils::GLShader>().get() != nullptr;
    case ShaderType::Fragment:
        return m_fragmentShader.Get<OpenGLUtils::GLShader>().get() != nullptr;
    case ShaderType::Compute:
        return m_computeShader.Get<OpenGLUtils::GLShader>().get() != nullptr;
    }
    return false;
}

void OpenGLUtils::GLProgram::Link()
{
    if (m_linked)
        return;
    if (!m_vertexShader.Get<OpenGLUtils::GLShader>())
    {
        UNIENGINE_ERROR("Missing vertex shader!");
        return;
    }

    if (!m_fragmentShader.Get<OpenGLUtils::GLShader>())
    {
        UNIENGINE_ERROR("Missing fragment shader!");
        return;
    }
    auto vertexShader = GetShader(ShaderType::Vertex);
    auto tessellationShader = GetShader(ShaderType::Tessellation);
    auto geometryShader = GetShader(ShaderType::Geometry);
    auto fragmentShader = GetShader(ShaderType::Fragment);
    auto computeShader = GetShader(ShaderType::Compute);

    if (vertexShader)
    {
        vertexShader->Compile();
    }
    if (tessellationShader)
    {
        tessellationShader->Compile();
    }
    if (geometryShader)
    {
        geometryShader->Compile();
    }
    if (fragmentShader)
    {
        fragmentShader->Compile();
    }
    if (computeShader)
    {
        computeShader->Compile();
    }

    glLinkProgram(m_id);
    GLint status = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLchar infoLog[1024];
        const std::string type = "PROGRAM";
        glGetProgramInfoLog(m_id, 1024, nullptr, infoLog);
        UNIENGINE_ERROR(
            "ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog +
            "\n -- --------------------------------------------------- -- ");
    }
    else
        m_linked = true;
}

void OpenGLUtils::GLProgram::Link(const std::shared_ptr<GLShader> &shader1, const std::shared_ptr<GLShader> &shader2)
{
    Attach(shader1);
    Attach(shader2);
    Link();
}

void OpenGLUtils::GLProgram::Link(
    const std::shared_ptr<GLShader> &shader1,
    const std::shared_ptr<GLShader> &shader2,
    const std::shared_ptr<GLShader> &shader3)
{
    Attach(shader1);
    Attach(shader2);
    Attach(shader3);
    Link();
}

void OpenGLUtils::GLProgram::Attach(const std::shared_ptr<GLShader> &shader)
{
    if (!shader)
        return;
    const auto type = shader->Type();
    if (HasShader(type))
        Detach(type);
    switch (type)
    {
    case ShaderType::Vertex:
        m_vertexShader = shader;
        break;
    case ShaderType::Tessellation:
        m_tessellationShader = shader;
        break;
    case ShaderType::Geometry:
        m_geometryShader = shader;
        break;
    case ShaderType::Fragment:
        m_fragmentShader = shader;
        break;
    case ShaderType::Compute:
        m_computeShader = shader;
        break;
    }
    shader->Attach(m_id);
}

void OpenGLUtils::GLProgram::Detach(ShaderType type)
{
    if (!HasShader(type))
        return;
    switch (type)
    {
    case ShaderType::Vertex:
        m_vertexShader.Get<OpenGLUtils::GLShader>()->Detach(m_id);
        m_vertexShader.Clear();
        break;
    case ShaderType::Tessellation:
        m_tessellationShader.Get<OpenGLUtils::GLShader>()->Detach(m_id);
        m_tessellationShader.Clear();
        break;
    case ShaderType::Geometry:
        m_geometryShader.Get<OpenGLUtils::GLShader>()->Detach(m_id);
        m_geometryShader.Clear();
        break;
    case ShaderType::Fragment:
        m_fragmentShader.Get<OpenGLUtils::GLShader>()->Detach(m_id);
        m_fragmentShader.Clear();
        break;
    case ShaderType::Compute:
        m_computeShader.Get<OpenGLUtils::GLShader>()->Detach(m_id);
        m_computeShader.Clear();
        break;
    }
}

void OpenGLUtils::GLProgram::SetBool(const std::string &name, bool value)
{
    Bind();
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
}
void OpenGLUtils::GLProgram::SetInt(const std::string &name, int value)
{
    Bind();
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}
void OpenGLUtils::GLProgram::SetFloat(const std::string &name, float value)
{
    Bind();
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}
void OpenGLUtils::GLProgram::SetFloat2(const std::string &name, const glm::vec2 &value)
{
    Bind();
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}
void OpenGLUtils::GLProgram::SetFloat2(const std::string &name, float x, float y)
{
    Bind();
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}
void OpenGLUtils::GLProgram::SetFloat3(const std::string &name, const glm::vec3 &value)
{
    Bind();
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}
void OpenGLUtils::GLProgram::SetFloat3(const std::string &name, float x, float y, float z)
{
    Bind();
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}
void OpenGLUtils::GLProgram::SetFloat4(const std::string &name, const glm::vec4 &value)
{
    Bind();
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}
void OpenGLUtils::GLProgram::SetFloat4(const std::string &name, float x, float y, float z, float w)
{
    Bind();
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
}
void OpenGLUtils::GLProgram::SetFloat2x2(const std::string &name, const glm::mat2 &mat)
{
    Bind();
    glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void OpenGLUtils::GLProgram::SetFloat3x3(const std::string &name, const glm::mat3 &mat)
{
    Bind();
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void OpenGLUtils::GLProgram::SetFloat4x4(const std::string &name, const glm::mat4 &mat)
{
    Bind();
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

OpenGLUtils::GLProgram::GLProgram()
{
    m_id = glCreateProgram();
}
void OpenGLUtils::GLProgram::OnInspect()
{
    auto vertexShader = GetShader(ShaderType::Vertex);
    auto fragmentShader = GetShader(ShaderType::Fragment);
    auto geometryShader = GetShader(ShaderType::Geometry);
    auto tessellationShader = GetShader(ShaderType::Tessellation);
    auto computeShader = GetShader(ShaderType::Compute);

    if (EditorManager::DragAndDropButton<OpenGLUtils::GLShader>(m_vertexShader, "Vertex"))
    {
        if (vertexShader)
            vertexShader->Detach(m_id);
        vertexShader = m_vertexShader.Get<OpenGLUtils::GLShader>();
        if (vertexShader)
            vertexShader->Attach(m_id);
        m_linked = false;
    }
    if (EditorManager::DragAndDropButton<OpenGLUtils::GLShader>(m_tessellationShader, "Tessellation"))
    {
        if (tessellationShader)
            tessellationShader->Detach(m_id);
        tessellationShader = m_tessellationShader.Get<OpenGLUtils::GLShader>();
        if (tessellationShader)
            tessellationShader->Attach(m_id);
        m_linked = false;
    }
    if (EditorManager::DragAndDropButton<OpenGLUtils::GLShader>(m_geometryShader, "Geometry"))
    {
        if (geometryShader)
            geometryShader->Detach(m_id);
        geometryShader = m_geometryShader.Get<OpenGLUtils::GLShader>();
        if (geometryShader)
            geometryShader->Attach(m_id);
        m_linked = false;
    }
    if (EditorManager::DragAndDropButton<OpenGLUtils::GLShader>(m_fragmentShader, "Fragment"))
    {
        if (fragmentShader)
            fragmentShader->Detach(m_id);
        fragmentShader = m_fragmentShader.Get<OpenGLUtils::GLShader>();
        if (fragmentShader)
            fragmentShader->Attach(m_id);
        m_linked = false;
    }
    if (EditorManager::DragAndDropButton<OpenGLUtils::GLShader>(m_computeShader, "Compute"))
    {
        if (computeShader)
            computeShader->Detach(m_id);
        computeShader = m_computeShader.Get<OpenGLUtils::GLShader>();
        if (computeShader)
            computeShader->Attach(m_id);
        m_linked = false;
    }
    ImGui::Text((std::string("Linked: ") + (m_linked ? "True" : "False")).c_str());
    if (ImGui::Button("Link"))
        Link();
}

void OpenGLUtils::GLProgram::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_vertexShader);
    list.push_back(m_tessellationShader);
    list.push_back(m_geometryShader);
    list.push_back(m_fragmentShader);
    list.push_back(m_computeShader);
}
void OpenGLUtils::GLProgram::Serialize(YAML::Emitter &out)
{
    m_vertexShader.Save("m_vertexShader", out);
    m_tessellationShader.Save("m_tessellationShader", out);
    m_geometryShader.Save("m_geometryShader", out);
    m_fragmentShader.Save("m_fragmentShader", out);
    m_computeShader.Save("m_computeShader", out);
}
void OpenGLUtils::GLProgram::Deserialize(const YAML::Node &in)
{
    AssetRef vertexShader;
    AssetRef tessellationShader;
    AssetRef geometryShader;
    AssetRef fragmentShader;
    AssetRef computeShader;
    vertexShader.Load("m_vertexShader", in);
    tessellationShader.Load("m_tessellationShader", in);
    geometryShader.Load("m_geometryShader", in);
    fragmentShader.Load("m_fragmentShader", in);
    computeShader.Load("m_computeShader", in);
    Attach(vertexShader.Get<OpenGLUtils::GLShader>());
    Attach(tessellationShader.Get<OpenGLUtils::GLShader>());
    Attach(geometryShader.Get<OpenGLUtils::GLShader>());
    Attach(fragmentShader.Get<OpenGLUtils::GLShader>());
    Attach(computeShader.Get<OpenGLUtils::GLShader>());
    m_linked = false;
}

#pragma region OpenGL Debugging

void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam)
{
    if (id == 131154 || id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131184)
        return; // ignore these non-significant error codes
    if (severity != GL_DEBUG_SEVERITY_HIGH)
        return; // ignore non-error messages.
    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Share Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

#pragma endregion