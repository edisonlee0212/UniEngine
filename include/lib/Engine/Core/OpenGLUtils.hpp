#pragma once
#include <AssetRef.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
	enum class OpenGLPolygonMode
	{
		Point = GL_POINT,
		Line = GL_LINE,
		Fill = GL_FILL
	};
	enum class OpenGLCullFace
	{
		Front = GL_FRONT,
		Back = GL_BACK,
		FrontAndBack = GL_FRONT_AND_BACK
	};
	enum class OpenGLCapability
	{
		DepthTest = GL_DEPTH_TEST,
		ScissorTest = GL_SCISSOR_TEST,
		StencilTest = GL_STENCIL_TEST,
		Blend = GL_BLEND,
		CullFace = GL_CULL_FACE
	};
	enum class OpenGLBlendFactor
	{
		Zero = GL_ZERO,
		One = GL_ONE,
		SrcColor = GL_SRC_COLOR,
		OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
		DstColor = GL_DST_COLOR,
		OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
		SrcAlpha = GL_SRC_ALPHA,
		OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
		DstAlpha = GL_DST_ALPHA,
		OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
		ConstantColor = GL_CONSTANT_COLOR,
		OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
		ConstantAlpha = GL_CONSTANT_ALPHA,
		OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
		SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE,
		Src1Color = GL_SRC1_COLOR,
		OneMinusSrc1Color = GL_ONE_MINUS_SRC1_COLOR,
		Src1Alpha = GL_SRC1_ALPHA,
		OneMinusSrc1Alpha = GL_ONE_MINUS_SRC1_ALPHA
	};
	class UNIENGINE_API OpenGLUtils : ISingleton<OpenGLUtils>
	{
		friend class DefaultResources;
		// friend class Graphics;
		float m_lineWidth = 1.0f;
		float m_pointSize = 1.0f;
		bool m_depthTest = true;
		bool m_scissorTest = false;
		bool m_stencilTest = false;
		bool m_blend = false;
		bool m_cullFace = false;
		OpenGLPolygonMode m_polygonMode = OpenGLPolygonMode::Fill;
		OpenGLCullFace m_cullFaceMode = OpenGLCullFace::Back;
		OpenGLBlendFactor m_blendingSrcFactor = OpenGLBlendFactor::One;
		OpenGLBlendFactor m_blendingDstFactor = OpenGLBlendFactor::Zero;

	public:
		static void InsertMemoryBarrier(GLbitfield barriers);
		static void InsertMemoryBarrierByRegion(GLbitfield barriers);
		static void Init();
		static void PreUpdate();
		static void SetEnable(OpenGLCapability capability, bool enable);
		static void SetPolygonMode(OpenGLPolygonMode mode);
		static void SetCullFace(OpenGLCullFace cullFace);
		static void SetBlendFunc(OpenGLBlendFactor srcFactor, OpenGLBlendFactor dstFactor);
		static void SetViewPort(int x1, int y1, int x2, int y2);
		static void SetViewPort(int x, int y);
		static void SetPointSize(float size);
		static void SetLineWidth(float width);
		static void Get(GLenum param, int& data);
		static void Get(GLenum param, float& data);
		static void Get(GLenum param, boolean& data);
		static void Get(GLenum param, double& data);

		static void PatchParameter(GLenum param, int value);
		static void PatchParameter(GLenum param, const std::vector<float>& values);
#pragma region Sub classes
		class UNIENGINE_API GLObject
		{
		protected:
			GLuint m_id = 0;

		public:
			[[nodiscard]] GLuint Id() const;
			virtual ~GLObject() = default;
		};

		enum class GLBufferTarget
		{
			Array = GL_ARRAY_BUFFER,
			AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
			CopyRead = GL_COPY_READ_BUFFER,
			CopyWrite = GL_COPY_WRITE_BUFFER,
			DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
			DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
			ElementArray = GL_ELEMENT_ARRAY_BUFFER,
			PixelPack = GL_PIXEL_PACK_BUFFER,
			PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
			Query = GL_QUERY_BUFFER,
			ShaderStorage = GL_SHADER_STORAGE_BUFFER,
			Texture = GL_TEXTURE_BUFFER,
			TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
			Uniform = GL_UNIFORM_BUFFER,
		};

		class UNIENGINE_API GLBuffer : public GLObject
		{
			friend class OpenGLUtils;

		protected:
			GLBufferTarget m_target = GLBufferTarget::Array;
			GLuint m_index = 0;
			static std::map<OpenGLUtils::GLBufferTarget, std::map<GLuint, GLuint>> m_boundBuffers;

		public:
			[[nodiscard]] GLBufferTarget GetTarget() const;
			GLBuffer();
			GLBuffer(GLBufferTarget target);
			GLBuffer(GLBufferTarget target, const GLuint& index);
			void SetTarget(GLBufferTarget newTarget);
			void SetTargetBase(GLBufferTarget newTarget, const GLuint& index);
			void Bind() const;
			void Unbind();
			static void BindDefault(GLBufferTarget target);
			void SetData(const GLsizei& length, const GLvoid* data, const GLenum& usage) const;
			void SubData(const GLintptr& offset, const GLsizeiptr& size, const GLvoid* data) const;
			void SetRange(const GLuint& index, const GLintptr& offset, const GLsizeiptr& size) const;
			~GLBuffer() override;
		};

		class UNIENGINE_API GLVAO : public GLObject
		{
			friend class OpenGLUtils;
			static GLuint m_boundVAO;

		protected:
			GLBuffer m_vbo = GLBuffer(GLBufferTarget::Array);
			GLBuffer m_ebo = GLBuffer(GLBufferTarget::ElementArray);

		public:
			~GLVAO() override;
			void Bind() const;
			static void BindDefault();
			GLVAO();
			GLBuffer& Vbo();
			GLBuffer& Ebo();
			void SetData(const GLsizei& length, const GLvoid* data, const GLenum& usage) const;
			void SubData(const GLintptr& offset, const GLsizeiptr& size, const GLvoid* data) const;
			void EnableAttributeArray(const GLuint& index);
			void DisableAttributeArray(const GLuint& index);
			void SetAttributePointer(
				const GLuint& index,
				const GLint& size,
				const GLenum& type,
				const GLboolean& normalized,
				const GLsizei& stride,
				const void* pointer);

			void SetAttributeIntPointer(
				const GLuint& index,
				const GLint& size,
				const GLenum& type,
				const GLsizei& stride,
				const void* pointer);

			void SetAttributeDivisor(const GLuint& index, const GLuint& divisor);
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
			void Clear(const GLint& level) const;
			void SetInt(const GLenum& paramName, const GLint& param);
			void SetFloat(const GLenum& paramName, const GLfloat& param);
			void SetFloat4(const GLenum& paramName, const GLfloat* params);
			void GenerateMipMap() const;
			void Bind(const GLenum& activate) const;
			GLTexture(const GLenum& target, const GLenum& format);
			~GLTexture() override;
		};

		class UNIENGINE_API GLTexture1D : public GLTexture
		{
			GLsizei m_width = 0;

		public:
			GLTexture1D(
				const GLsizei& levels, const GLenum& internalFormat, const GLsizei& width, const bool& immutable = true);
			void SetData(const GLint& level, const GLenum& type, const void* pixels) const;
			void SubData(
				const GLint& level,
				const GLint& xOffset,
				const GLenum& type,
				const GLsizei& width,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTexture2D : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_height = 0;
			bool m_immutable;
			friend class Texture2D;

		public:
			[[nodiscard]] glm::ivec2 GetSize() const;

			GLTexture2D(
				const GLsizei& levels,
				const GLenum& internalFormat,
				const GLsizei& width,
				const GLsizei& height,
				const bool& immutable = true);

			void SetData(const GLint& level, const GLenum& format, const GLenum& type, const void* pixels) const;

			void ReSize(
				const GLint& level,
				const GLenum& internalFormat,
				const GLenum& format,
				const GLenum& type,
				const void* pixels,
				const GLsizei& width,
				const GLsizei& height);

			void SetData(
				const GLint& level,
				const GLenum& internalFormat,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;

			void SubData(
				const GLint& level,
				const GLenum& format,
				const GLint& xOffset,
				const GLint& yOffset,
				const GLsizei& width,
				const GLsizei& height,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTexture3D : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_height = 0;
			GLsizei m_depth = 0;

		public:
			GLTexture3D(
				const GLsizei& levels,
				const GLenum& internalFormat,
				const GLsizei& width,
				const GLsizei& height,
				const GLsizei& depth,
				const bool& immutable = true);
			;

			void SetData(const GLint& level, const GLenum& format, const GLenum& type, const void* pixels) const;

			void SubData(
				const GLint& level,
				const GLint& xOffset,
				const GLint& yOffset,
				const GLint& zOffset,
				const GLsizei& width,
				const GLsizei& height,
				const GLsizei& depth,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTextureCubeMap : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_height = 0;

		public:
			GLTextureCubeMap(
				const GLsizei& levels,
				const GLenum& internalFormat,
				const GLsizei& width,
				const GLsizei& height,
				const bool& immutable = true);

			void SetData(
				const CubeMapIndex& index,
				const GLint& level,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;

			void SetData(
				const CubeMapIndex& index,
				const GLint& level,
				const GLenum& internalFormat,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;

			void SubData(
				const CubeMapIndex& index,
				const GLint& level,
				const GLint& xOffset,
				const GLint& yOffset,
				const GLsizei& width,
				const GLsizei& height,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTexture1DArray : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_layers = 0;

		public:
			GLTexture1DArray(
				const GLsizei& levels, const GLenum& internalFormat, const GLsizei& width, const GLsizei& layers);

			void SetData(const GLint& level, const GLenum& format, const GLenum& type, const void* pixels) const;

			void SubData(
				const GLint& level,
				const GLint& xOffset,
				const GLint& layer,
				const GLsizei& width,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTexture2DArray : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_height = 0;
			GLsizei m_layers = 0;

		public:
			GLTexture2DArray(
				const GLsizei& levels,
				const GLenum& internalFormat,
				const GLsizei& width,
				const GLsizei& height,
				const GLsizei& layers);
			;

			void SetData(const GLint& level, const GLenum& format, const GLenum& type, const void* pixels) const;

			void SubData(
				const GLint& level,
				const GLint& xOffset,
				const GLint& yOffset,
				const GLsizei& layer,
				const GLsizei& width,
				const GLsizei& height,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLTextureCubeMapArray : public GLTexture
		{
			GLsizei m_width = 0;
			GLsizei m_height = 0;
			GLsizei m_layers = 0;

		public:
			GLTextureCubeMapArray(
				const GLsizei& levels,
				const GLenum& internalFormat,
				const GLsizei& width,
				const GLsizei& height,
				const GLsizei& layers);

			void SetData(
				const CubeMapIndex& index,
				const GLint& level,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;

			void SubData(
				const CubeMapIndex& index,
				const GLint& level,
				const GLint& xOffset,
				const GLint& yOffset,
				const GLsizei& layer,
				const GLsizei& width,
				const GLsizei& height,
				const GLenum& format,
				const GLenum& type,
				const void* pixels) const;
		};

		class UNIENGINE_API GLFrameBuffer : public GLObject
		{
			friend class OpenGLUtils;
			bool m_color;
			bool m_depth;
			bool m_stencil;
			static GLuint m_boundFrameBuffer;

		public:
			static void Enable(GLenum cap);
			static void Disable(GLenum cap);
			void Bind() const;
			void ClearColor(const glm::vec4& value) const;
			void Check() const;
			static void DefaultFrameBufferDrawBuffer(GLenum buffer);
			void DrawBuffers(const std::vector<GLenum>& buffers) const;
			static void BindDefault();
			GLFrameBuffer();
			~GLFrameBuffer() override;
			bool Color() const;
			bool Depth() const;
			bool Stencil() const;
			void AttachRenderBuffer(const GLRenderBuffer* buffer, GLenum attachPoint);
			void AttachTexture(const GLTexture* texture, GLenum attachPoint);
			void AttachTextureLayer(const GLTexture* texture, GLenum attachPoint, GLint layer);
			void AttachTexture2D(const GLTexture* texture, GLenum attachPoint, GLenum texTarget, GLint level = 0);
			void Clear();
		};
		enum class UNIENGINE_API ShaderType
		{
			Vertex,
			TessellationControl,
			TessellationEvaluation,
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
			void Set(ShaderType type, const std::string& code);
			void Attach(GLuint programID);
			void Detach(GLuint programID) const;

			void OnInspect() override;
			void Serialize(YAML::Emitter& out) override;
			void Deserialize(const YAML::Node& in) override;
		};

		class UNIENGINE_API GLProgram : public GLObject, public IAsset
		{
			friend class OpenGLUtils;

			AssetRef m_vertexShader;
			AssetRef m_tessellationControlShader;
			AssetRef m_tessellationEvaluationShader;
			AssetRef m_geometryShader;
			AssetRef m_fragmentShader;
			AssetRef m_computeShader;

			static GLuint m_boundProgram;
			bool m_linked = false;

		public:
			GLProgram();
			~GLProgram() override;
			std::shared_ptr<GLShader> GetShader(ShaderType type);
			bool HasShader(ShaderType type);
			void Bind();

			static void BindDefault();
			void Link();
			void Link(const std::shared_ptr<GLShader>& shader1, const std::shared_ptr<GLShader>& shader2);
			void Link(
				const std::shared_ptr<GLShader>& shader1,
				const std::shared_ptr<GLShader>& shader2,
				const std::shared_ptr<GLShader>& shader3);
			void Attach(const std::shared_ptr<GLShader>& shader);
			void AttachAndLink(const std::vector<std::shared_ptr<GLShader>>& shaders);
			void Detach(ShaderType type);
			void SetBool(const std::string& name, bool value);
			void SetInt(const std::string& name, int value);
			void SetFloat(const std::string& name, float value);
			void SetFloat2(const std::string& name, const glm::vec2& value);
			void SetFloat2(const std::string& name, float x, float y);
			void SetFloat3(const std::string& name, const glm::vec3& value);
			void SetFloat3(const std::string& name, float x, float y, float z);
			void SetFloat4(const std::string& name, const glm::vec4& value);
			void SetFloat4(const std::string& name, float x, float y, float z, float w);
			void SetFloat2x2(const std::string& name, const glm::mat2& mat);
			void SetFloat3x3(const std::string& name, const glm::mat3& mat);
			void SetFloat4x4(const std::string& name, const glm::mat4& mat);
			void DispatchCompute(const glm::uvec3& workGroupSize);
			void OnInspect() override;
			void CollectAssetRef(std::vector<AssetRef>& list) override;
			void Serialize(YAML::Emitter& out) override;
			void Deserialize(const YAML::Node& in) override;
		};
#pragma endregion
	};
} // namespace UniEngine
