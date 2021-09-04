#pragma once
#include <IAsset.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API OpenGLUtils : ISingleton<OpenGLUtils>
{
    friend class DefaultResources;
    friend class RenderManager;
  public:
    static void Init();
    static void PreUpdate();
#pragma region Sub classes
    class UNIENGINE_API GLObject
    {
      protected:
        GLuint m_id = 0;
      public:
        [[nodiscard]] GLuint Id() const;
        virtual ~GLObject() = default;
    };

    class UNIENGINE_API GLBuffer : public GLObject
    {
        friend class OpenGLUtils;
      protected:
        GLenum m_target;
        static std::map<GLenum, GLuint> m_boundBuffers;
      public:
        GLBuffer(GLenum target);
        void Bind() const;
        void SetData(const GLsizei &length, const GLvoid *data, const GLenum &usage) const;
        void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) const;
        ~GLBuffer() override;
    };

    class UNIENGINE_API GLSSBO : public GLBuffer
    {
      public:
        GLSSBO();
        void SetBase(const GLuint &index) const;
        static void BindDefault();
    };

    class UNIENGINE_API GLPPBO : public GLBuffer
    {
      public:
        GLPPBO();
        static void BindDefault();
    };

    class UNIENGINE_API GLPUBO : public GLBuffer
    {
      public:
        GLPUBO();
        static void BindDefault();
    };

    class UNIENGINE_API GLEBO : public GLBuffer
    {
      public:
        GLEBO();
        static void BindDefault();
    };

    class UNIENGINE_API GLVBO : public GLBuffer
    {
      public:
        GLVBO();
        static void BindDefault();
    };

    class UNIENGINE_API GLUBO : public GLBuffer
    {
      public:
        GLUBO();
        static void BindDefault();
        void SetBase(const GLuint &index) const;
        void SetRange(const GLuint &index, const GLintptr &offset, const GLsizeiptr &size) const;
    };

    class UNIENGINE_API GLVAO : public GLObject
    {
        friend class OpenGLUtils;
        static GLuint m_boundVAO;
      protected:
        GLVBO m_vbo;
        GLEBO m_ebo;

      public:
        ~GLVAO() override;
        void Bind() const;
        static void BindDefault();
        GLVAO();
        GLVBO *Vbo();
        GLEBO *Ebo();
        void SetData(const GLsizei &length, const GLvoid *data, const GLenum &usage) const;
        void SubData(const GLintptr &offset, const GLsizeiptr &size, const GLvoid *data) const;
        void EnableAttributeArray(const GLuint &index);
        void DisableAttributeArray(const GLuint &index);
        void SetAttributePointer(
            const GLuint &index,
            const GLint &size,
            const GLenum &type,
            const GLboolean &normalized,
            const GLsizei &stride,
            const void *pointer) const;

        void SetAttributeIntPointer(
            const GLuint &index,
            const GLint &size,
            const GLenum &type,
            const GLsizei &stride,
            const void *pointer) const;

        void SetAttributeDivisor(const GLuint &index, const GLuint &divisor) const;
    };

    class UNIENGINE_API GLRenderBuffer : public GLObject
    {
        friend class OpenGLUtils;
        static GLuint m_boundRenderBuffer;
      public:
        void Bind();
        static void BindDefault();
        GLRenderBuffer();
        ~GLRenderBuffer() override;
        void AllocateStorage(GLenum internalFormat, GLsizei width, GLsizei height);
    };

    enum class UNIENGINE_API CubeMapIndex
    {
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,
    };

    struct UNIENGINE_API TextureBinding
    {
        GLuint m_1d;
        GLuint m_2d;
        GLuint m_3d;
        GLuint m_rectangle;
        GLuint m_buffer;
        GLuint m_cubeMap;
        GLuint m_1dArray;
        GLuint m_2dArray;
        GLuint m_cubeMapArray;
        GLuint m_2dMS;
        GLuint m_2dMSArray;
        TextureBinding();
    };

    class UNIENGINE_API GLTexture : public GLObject
    {
        friend class OpenGLUtils;
        static GLint m_maxAllowedTexture;
        static std::vector<std::pair<GLenum, GLuint>> m_currentBoundTextures;

      protected:
        GLenum m_type;
        GLenum m_format;
      public:
        static GLint GetMaxAllowedTexture();
        void Clear(const GLint &level) const;
        void SetInt(const GLenum &paramName, const GLint &param);
        void SetFloat(const GLenum &paramName, const GLfloat &param);
        void SetFloat4(const GLenum &paramName, const GLfloat *params);
        void GenerateMipMap() const;
        void Bind(const GLenum &activate) const;
        GLTexture(const GLenum &target, const GLenum &format);
        ~GLTexture() override;
    };

    class UNIENGINE_API GLTexture1D : public GLTexture
    {
        GLsizei m_width = 0;

      public:
        GLTexture1D(
            const GLsizei &levels, const GLenum &internalFormat, const GLsizei &width, const bool &immutable = true);
        void SetData(const GLint &level, const GLenum &type, const void *pixels) const;
        void SubData(
            const GLint &level,
            const GLint &xOffset,
            const GLenum &type,
            const GLsizei &width,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTexture2D : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_height = 0;
        bool m_immutable;
        friend class Texture2D;

      public:
        GLTexture2D(
            const GLsizei &levels,
            const GLenum &internalFormat,
            const GLsizei &width,
            const GLsizei &height,
            const bool &immutable = true);

        void SetData(const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const;

        void ReSize(
            const GLint &level,
            const GLenum &internalFormat,
            const GLenum &format,
            const GLenum &type,
            const void *pixels,
            const GLsizei &width,
            const GLsizei &height);

        void SetData(
            const GLint &level,
            const GLenum &internalFormat,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;

        void SubData(
            const GLint &level,
            const GLenum &format,
            const GLint &xOffset,
            const GLint &yOffset,
            const GLsizei &width,
            const GLsizei &height,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTexture3D : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_height = 0;
        GLsizei m_depth = 0;

      public:
        GLTexture3D(
            const GLsizei &levels,
            const GLenum &internalFormat,
            const GLsizei &width,
            const GLsizei &height,
            const GLsizei &depth,
            const bool &immutable = true);
        ;

        void SetData(const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const;

        void SubData(
            const GLint &level,
            const GLint &xOffset,
            const GLint &yOffset,
            const GLint &zOffset,
            const GLsizei &width,
            const GLsizei &height,
            const GLsizei &depth,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTextureCubeMap : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_height = 0;

      public:
        GLTextureCubeMap(
            const GLsizei &levels,
            const GLenum &internalFormat,
            const GLsizei &width,
            const GLsizei &height,
            const bool &immutable = true);

        void SetData(
            const CubeMapIndex &index,
            const GLint &level,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;

        void SetData(
            const CubeMapIndex &index,
            const GLint &level,
            const GLenum &internalFormat,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;

        void SubData(
            const CubeMapIndex &index,
            const GLint &level,
            const GLint &xOffset,
            const GLint &yOffset,
            const GLsizei &width,
            const GLsizei &height,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTexture1DArray : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_layers = 0;

      public:
        GLTexture1DArray(
            const GLsizei &levels, const GLenum &internalFormat, const GLsizei &width, const GLsizei &layers);

        void SetData(const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const;

        void SubData(
            const GLint &level,
            const GLint &xOffset,
            const GLint &layer,
            const GLsizei &width,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTexture2DArray : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_height = 0;
        GLsizei m_layers = 0;

      public:
        GLTexture2DArray(
            const GLsizei &levels,
            const GLenum &internalFormat,
            const GLsizei &width,
            const GLsizei &height,
            const GLsizei &layers);
        ;

        void SetData(const GLint &level, const GLenum &format, const GLenum &type, const void *pixels) const;

        void SubData(
            const GLint &level,
            const GLint &xOffset,
            const GLint &yOffset,
            const GLsizei &layer,
            const GLsizei &width,
            const GLsizei &height,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLTextureCubeMapArray : public GLTexture
    {
        GLsizei m_width = 0;
        GLsizei m_height = 0;
        GLsizei m_layers = 0;

      public:
        GLTextureCubeMapArray(
            const GLsizei &levels,
            const GLenum &internalFormat,
            const GLsizei &width,
            const GLsizei &height,
            const GLsizei &layers);

        void SetData(
            const CubeMapIndex &index,
            const GLint &level,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;

        void SubData(
            const CubeMapIndex &index,
            const GLint &level,
            const GLint &xOffset,
            const GLint &yOffset,
            const GLsizei &layer,
            const GLsizei &width,
            const GLsizei &height,
            const GLenum &format,
            const GLenum &type,
            const void *pixels) const;
    };

    class UNIENGINE_API GLFrameBuffer : public GLObject
    {
        friend class OpenGLUtils;
        bool m_color;
        bool m_depth;
        bool m_stencil;
        static GLuint m_boundFrameBuffer;
      public:
        static void Enable(const GLenum &cap);
        static void Disable(const GLenum &cap);
        void Bind() const;
        void ClearColor(const glm::vec4 &value) const;
        void ViewPort(const glm::ivec4 &value) const;
        void ViewPort(size_t x, size_t y) const;
        void Check() const;
        void DrawBuffer(const GLenum &buffer) const;
        void DrawBuffers(const GLsizei &n, const GLenum *buffers) const;
        static void BindDefault();
        GLFrameBuffer();
        ~GLFrameBuffer() override;
        bool Color() const;
        bool Depth() const;
        bool Stencil() const;
        void AttachRenderBuffer(const GLRenderBuffer *buffer, const GLenum &attachPoint);
        void AttachTexture(const GLTexture *texture, const GLenum &attachPoint);
        void AttachTextureLayer(const GLTexture *texture, const GLenum &attachPoint, const GLint &layer);
        void AttachTexture2D(
            const GLTexture *texture, const GLenum &attachPoint, const GLenum &texTarget, const GLint &level = 0);
        void Clear();
    };
    enum class UNIENGINE_API ShaderType
    {
        Vertex,
        Tessellation,
        Geometry,
        Fragment,
        Compute
    };

    class UNIENGINE_API GLShader : public GLObject, public IAsset
    {
        std::string m_code;
        ShaderType m_type;
        bool m_compiled;
      public:
        [[nodiscard]] std::string GetCode() const;
        void OnCreate() override;
        ~GLShader() override;
        [[nodiscard]] ShaderType Type() const;
        [[nodiscard]] bool Compiled() const;
        void Compile();
        void Set(ShaderType type, const std::string &code);
        void Attach(GLuint programID);
        void Detach(GLuint programID) const;

        void OnInspect() override;
        void Serialize(YAML::Emitter &out) override;
        void Deserialize(const YAML::Node &in) override;
    };

    class UNIENGINE_API GLProgram : public GLObject, public IAsset
    {
        friend class OpenGLUtils;
        friend class AssetManager;

        AssetRef m_vertexShader;
        AssetRef m_tessellationShader;
        AssetRef m_geometryShader;
        AssetRef m_fragmentShader;
        AssetRef m_computeShader;

        static GLuint m_boundProgram;
        bool m_linked = false;
      public:
        GLProgram();
        void OnCreate() override;
        ~GLProgram() override;
        std::shared_ptr<GLShader> GetShader(ShaderType type);
        bool HasShader(ShaderType type);
        void Bind();
        static void BindDefault();
        void Link();
        void Link(const std::shared_ptr<GLShader> &shader1, const std::shared_ptr<GLShader> &shader2);
        void Link(
            const std::shared_ptr<GLShader> &shader1,
            const std::shared_ptr<GLShader> &shader2,
            const std::shared_ptr<GLShader> &shader3);
        void Attach(const std::shared_ptr<GLShader> &shader);
        void Detach(ShaderType type);
        void SetBool(const std::string &name, bool value);
        void SetInt(const std::string &name, int value);
        void SetFloat(const std::string &name, float value);
        void SetFloat2(const std::string &name, const glm::vec2 &value);
        void SetFloat2(const std::string &name, float x, float y);
        void SetFloat3(const std::string &name, const glm::vec3 &value);
        void SetFloat3(const std::string &name, float x, float y, float z);
        void SetFloat4(const std::string &name, const glm::vec4 &value);
        void SetFloat4(const std::string &name, float x, float y, float z, float w);
        void SetFloat2x2(const std::string &name, const glm::mat2 &mat);
        void SetFloat3x3(const std::string &name, const glm::mat3 &mat);
        void SetFloat4x4(const std::string &name, const glm::mat4 &mat);

        void OnInspect() override;
        void CollectAssetRef(std::vector<AssetRef> &list) override;
        void Serialize(YAML::Emitter &out) override;
        void Deserialize(const YAML::Node &in) override;
    };
#pragma endregion
};

} // namespace UniEngine
