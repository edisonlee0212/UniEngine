#pragma once
#include "OpenGLUtils.hpp"
#include "IAsset.hpp"
#include "Scene.hpp"
#include "Transform.hpp"
#include "Vertex.hpp"
#include "RenderGeometry.hpp"
namespace UniEngine
{
	enum class MeshStoreType {
		//Meshes with persistent type will be combined into a single mesh to improve performance, removing meshes will have a bigger penalty.
		Persistent,
		//Meshes here will have it's own VAO. It's relatively more lightweight to make changes here.
		Temporal
	};

	class UNIENGINE_API MeshStorage {
		std::unique_ptr<OpenGLUtils::GLVAO> m_persistentMeshesVAO;
	};
	class ParticleMatrices;
	class UNIENGINE_API Mesh : public IAsset, public RenderGeometry
	{
		std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
		size_t m_offset = 0;

		unsigned m_mask = 0;
		Bound m_bound;
		friend class Graphics;
		friend class RenderLayer;
		friend class MeshRenderer;
		friend class Particles;
		friend class Editor;
		size_t m_version = 0;

		std::vector<Vertex> m_vertices;
		std::vector<glm::uvec3> m_triangles;
		unsigned m_verticesSize = 0;
		unsigned m_triangleSize = 0;
	protected:
		bool SaveInternal(const std::filesystem::path& path) override;

	public:
		void Draw() const override;
		void DrawInstancedColored(const std::vector<glm::vec4>& colors, const std::vector<glm::mat4>& matrices) const override;
		void DrawInstancedColored(const std::vector<glm::vec4>& colors, const std::vector<GlobalTransform>& matrices) const override;

		void DrawInstanced(const std::vector<glm::mat4>& matrices) const override;
		void DrawInstanced(const std::shared_ptr<ParticleMatrices>& particleMatrices) const override;
		void DrawInstanced(const std::vector<GlobalTransform>& matrices) const override;

		void OnInspect() override;
		[[nodiscard]] glm::vec3 GetCenter() const;
		[[nodiscard]] Bound GetBound() const;
		void OnCreate() override;
		void Upload();
		void SetVertices(const unsigned& mask, std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
		void SetVertices(const unsigned& mask, const std::vector<Vertex>& vertices, const std::vector<glm::uvec3>& triangles);
		[[nodiscard]] size_t GetVerticesAmount() const;
		[[nodiscard]] size_t GetTriangleAmount() const;

		void UnsafeSetVerticesAmount(unsigned size);
		void UnsafeSetTrianglesAmount(unsigned size);
		void RecalculateNormal();
		void RecalculateTangent();
		[[nodiscard]] std::shared_ptr<OpenGLUtils::GLVAO> Vao() const;
		void Enable() const;
		[[nodiscard]] size_t& GetVersion();
		[[nodiscard]] std::vector<Vertex>& UnsafeGetVertices();
		[[nodiscard]] std::vector<glm::uvec3>& UnsafeGetTriangles();

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& in) override;
	};
} // namespace UniEngine