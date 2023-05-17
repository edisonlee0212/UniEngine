#pragma once

#include "Camera.hpp"
#include "DefaultResources.hpp"
#include "Lights.hpp"
#include "MeshRenderer.hpp"
#include "Particles.hpp"
#include "SkinnedMeshRenderer.hpp"
#include "RenderLayer.hpp"
#include "StrandsRenderer.hpp"

namespace UniEngine {
	struct GizmoSettings {
		DrawSettings m_drawSettings;
		enum class ColorMode {
			Default,
			VertexColor,
			NormalColor
		} m_colorMode = ColorMode::Default;
		bool m_depthTest = true;
	};

	class UNIENGINE_API Graphics {
		public:
		/**
		 * DrawMeshInternal draws a mesh for one frame. The mesh will be affected by the lights, can cast and receive shadows and be
		 * affected by Projectors - just like it was part of some game object.
		 * @param mesh The Mesh to draw.
		 * @param material The material to use.
		 * @param model The transform of the mesh
		 * @param cameraComponent The target camera to draw the mesh.
		 * @param receiveShadow Determines whether the mesh can receive shadows.
		 * @param castShadow Determines whether the mesh can cast shadows.
		 */
		static void DrawMesh(
						const std::shared_ptr<Mesh> &mesh,
						const std::shared_ptr<Material> &material,
						const glm::mat4 &model,
						const std::shared_ptr<Camera> &cameraComponent,
						const bool &receiveShadow = true,
						const bool &castShadow = true);

		/**
		 * Draws the same mesh multiple times using GPU instancing. Use this function in situations where you want to draw
		 * the same mesh for a particular amount of times using an instanced shader.
		 * @param mesh The Mesh to draw.
		 * @param material The material to use. The program should contain instanced shader.
		 * @param model The transform of the group
		 * @param matrices The transform the each individual instances
		 * @param cameraComponent The target camera to draw the mesh.
		 * @param receiveShadow Determines whether the mesh can receive shadows.
		 * @param castShadow Determines whether the mesh can cast shadows.
		 */
		static void DrawMeshInstanced(
						const std::shared_ptr<Mesh> &mesh,
						const std::shared_ptr<Material> &material,
						const glm::mat4 &model,
						const std::shared_ptr<ParticleMatrices> &matrices,
						const std::shared_ptr<Camera> &cameraComponent,
						const bool &receiveShadow = true,
						const bool &castShadow = true);

		/*
		static void DrawTexture2D(
				const OpenGLUtils::GLTexture2D *texture, const float &depth, const glm::vec2 &center, const glm::vec2 &size);
		static void DrawTexture2D(
				const OpenGLUtils::GLTexture2D *texture,
				const float &depth,
				const glm::vec2 &center,
				const glm::vec2 &size,
				const RenderTarget *target);
		static void DrawTexture2D(
				const Texture2D *texture,
				const float &depth,
				const glm::vec2 &center,
				const glm::vec2 &size,
				const RenderTarget *target);
		static void DrawTexture2D(
				const Texture2D *texture,
				const float &depth,
				const glm::vec2 &center,
				const glm::vec2 &size,
				const Camera &cameraComponent);
		*/

#pragma region Render

		static void RenderCube();

		static void RenderQuad();

#pragma endregion
	};

	class UNIENGINE_API Gizmos {
		static void DrawGizmoStrandsInternal(const GizmoSettings &gizmoSettings,
																				 const std::shared_ptr<Strands> &strands,
																				 const glm::vec4 &color,
																				 const glm::mat4 &model,
																				 const glm::mat4 &scaleMatrix);

		static void DrawGizmoStrandsInstancedInternal(const GizmoSettings &gizmoSettings,
																									const std::shared_ptr<Strands> &strands,
																									const glm::vec4 &color,
																									const glm::mat4 &model,
																									const std::vector<glm::mat4> &matrices,
																									const glm::mat4 &scaleMatrix);

		static void DrawGizmoStrandsInstancedColoredInternal(const GizmoSettings &gizmoSettings,
																												 const std::shared_ptr<Strands> &strands,
																												 const std::vector<glm::vec4> &colors,
																												 const std::vector<glm::mat4> &matrices,
																												 const glm::mat4 &model, const glm::mat4 &scaleMatrix);

		static void DrawGizmoMeshInternal(const GizmoSettings &gizmoSettings,
																			const std::shared_ptr<Mesh> &mesh,
																			const glm::vec4 &color,
																			const glm::mat4 &model,
																			const glm::mat4 &scaleMatrix);

		static void DrawGizmoMeshInstancedInternal(const GizmoSettings &gizmoSettings,
																							 const std::shared_ptr<Mesh> &mesh,
																							 const glm::vec4 &color,
																							 const glm::mat4 &model,
																							 const std::vector<glm::mat4> &matrices,
																							 const glm::mat4 &scaleMatrix);

		static void DrawGizmoMeshInstancedColoredInternal(const GizmoSettings &gizmoSettings,
																											const std::shared_ptr<Mesh> &mesh,
																											const std::vector<glm::vec4> &colors,
																											const std::vector<glm::mat4> &matrices,
																											const glm::mat4 &model,
																											const glm::mat4 &scaleMatrix);

		public:
#pragma region Gizmos

		static void DrawGizmoMesh(
						const std::shared_ptr<Mesh> &mesh,
						const glm::vec4 &color = glm::vec4(1.0f),
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoStrands(
						const std::shared_ptr<Strands> &strands,
						const glm::vec4 &color = glm::vec4(1.0f),
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoMeshInstanced(
						const std::shared_ptr<Mesh> &mesh,
						const glm::vec4 &color,
						const std::vector<glm::mat4> &matrices,
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoMeshInstancedColored(
						const std::shared_ptr<Mesh> &mesh,
						const std::vector<glm::vec4> &colors,
						const std::vector<glm::mat4> &matrices,
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoStrands(
						const std::shared_ptr<Strands> &strands,
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color = glm::vec4(1.0f),
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoMesh(
						const std::shared_ptr<Mesh> &mesh,
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color = glm::vec4(1.0f),
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoMeshInstanced(
						const std::shared_ptr<Mesh> &mesh,
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color,
						const std::vector<glm::mat4> &matrices,
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoMeshInstancedColored(
						const std::shared_ptr<Mesh> &mesh,
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const std::vector<glm::vec4> &colors,
						const std::vector<glm::mat4> &matrices,
						const glm::mat4 &model = glm::mat4(1.0f),
						const float &size = 1.0f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRay(
						const glm::vec4 &color, const glm::vec3 &start, const glm::vec3 &end, const float &width = 0.01f,
						const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(
						const glm::vec4 &color,
						const std::vector<std::pair<glm::vec3, glm::vec3>> &startEnds,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(
						const glm::vec4 &color,
						const std::vector<glm::vec3> &starts,
						const std::vector<glm::vec3> &ends,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(
						const std::vector<glm::vec4> &colors,
						const std::vector<glm::vec3> &starts,
						const std::vector<glm::vec3> &ends,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(const glm::vec4 &color, const std::vector<Ray> &rays, const float &width = 0.01f,
															const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRay(const glm::vec4 &color, const Ray &ray, const float &width = 0.01f,
														 const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRay(
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color,
						const glm::vec3 &start,
						const glm::vec3 &end,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color,
						const std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRays(
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color,
						const std::vector<Ray> &rays,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

		static void DrawGizmoRay(
						const std::shared_ptr<Camera> &cameraComponent,
						const glm::vec3 &cameraPosition,
						const glm::quat &cameraRotation,
						const glm::vec4 &color,
						const Ray &ray,
						const float &width = 0.01f, const GizmoSettings &gizmoSettings = {});

#pragma endregion
	};
} // namespace UniEngine
