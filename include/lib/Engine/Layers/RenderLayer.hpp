#pragma once

#include <Camera.hpp>
#include <DefaultResources.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <Particles.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <EnvironmentalMap.hpp>

#include "ILayer.hpp"

namespace UniEngine {


	struct UNIENGINE_API RenderingSettingsBlock {
		float m_splitDistance[4];
		int m_pcfSampleAmount = 64;
		int m_blockerSearchAmount = 1;
		float m_seamFixRatio = 0.05f;
		float m_gamma = 2.2f;

		float m_strandsSubdivisionXFactor = 50.0f;
		float m_strandsSubdivisionYFactor = 50.0f;
		int m_strandsSubdivisionMaxX = 15;
		int m_strandsSubdivisionMaxY = 8;
	};

	struct EnvironmentalMapSettingsBlock {
		glm::vec4 m_backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		float m_environmentalMapGamma = 1.0f;
		float m_environmentalLightingIntensity = 1.0f;
		float m_backgroundIntensity = 1.0f;
		float m_environmentalPadding2 = 0.0f;
	};

	struct MaterialSettingsBlock {
		int m_albedoEnabled = 0;
		int m_normalEnabled = 0;
		int m_metallicEnabled = 0;
		int m_roughnessEnabled = 0;

		int m_aoEnabled = 0;
		int m_castShadow = true;
		int m_receiveShadow = true;
		int m_enableShadow = true;

		glm::vec4 m_albedoColorVal = glm::vec4(1.0f);
		glm::vec4 m_subsurfaceColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
		glm::vec4 m_subsurfaceRadius = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
		float m_metallicVal = 0.5f;
		float m_roughnessVal = 0.5f;
		float m_aoVal = 1.0f;
		float m_emissionVal = 0.0f;
	};


	enum class RenderCommandType {
		None,
		FromRenderer,
		FromAPI,
		FromAPIInstanced
	};

	enum class RenderGeometryType {
		None,
		Mesh,
		SkinnedMesh,
		Strands
	};
	struct RenderCommand {
		RenderCommandType m_commandType = RenderCommandType::None;
		RenderGeometryType m_geometryType = RenderGeometryType::None;
		Entity m_owner = Entity();
		std::shared_ptr<RenderGeometry> m_renderGeometry;
		bool m_castShadow = true;
		bool m_receiveShadow = true;
		std::shared_ptr<ParticleMatrices> m_matrices;
		std::shared_ptr<BoneMatrices> m_boneMatrices; // We require the skinned mesh renderer to provide bones.
		GlobalTransform m_globalTransform;
	};

	struct RenderCommandGroup {
		std::shared_ptr<Material> m_material;
		std::unordered_map<Handle, std::vector<RenderCommand>> m_renderCommands;
	};

	struct RenderInstances {
		std::shared_ptr<Camera> m_camera;
		std::unordered_map<Handle, RenderCommandGroup> m_renderCommandsGroups;
	};

	class UNIENGINE_API RenderLayer : public ILayer {
	public:

		void RenderToCamera(const std::shared_ptr<Camera>& cameraComponent, const GlobalTransform& cameraModel);

		void ApplyEnvironmentalSettings(const std::shared_ptr<Camera>& cameraComponent);

		int m_mainCameraResolutionX = 1;
		int m_mainCameraResolutionY = 1;
		bool m_allowAutoResize = true;
		float m_mainCameraResolutionMultiplier = 1.0f;
	private:
		unsigned m_frameIndex = 0;
#pragma region GUI
		bool m_enableRenderMenu = false;
		
#pragma endregion

		friend class Mesh;
		friend class Strands;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_instancedColorBuffer;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_instancedMatricesBuffer;


		std::unordered_map<Handle, RenderInstances> m_deferredRenderInstances;
		std::unordered_map<Handle, RenderInstances> m_deferredInstancedRenderInstances;
		std::unordered_map<Handle, RenderInstances> m_forwardRenderInstances;
		std::unordered_map<Handle, RenderInstances> m_forwardInstancedRenderInstances;
		std::unordered_map<Handle, RenderInstances> m_transparentRenderInstances;
		std::unordered_map<Handle, RenderInstances> m_instancedTransparentRenderInstances;
#pragma region Settings
		RenderingSettingsBlock m_renderSettings;
		bool m_stableFit = true;
		float m_maxShadowDistance = 300;
		float m_shadowCascadeSplit[DefaultResources::ShaderIncludes::ShadowCascadeAmount] = { 0.075f, 0.15f, 0.3f, 1.0f };

		void SetSplitRatio(const float& r1, const float& r2, const float& r3, const float& r4);

		void SetDirectionalLightShadowMapResolution(const size_t& value);

		void SetPointLightShadowMapResolution(const size_t& value);

		void SetSpotLightShadowMapResolution(const size_t& value);

		glm::vec3 ClosestPointOnLine(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b);

#pragma endregion

		void OnInspect() override;

		void PreUpdate() override;

		void LateUpdate() override;

		size_t Triangles();

		size_t DrawCall();

		void DispatchRenderCommands(
			const RenderInstances& renderCommands,
			const std::function<void(const std::shared_ptr<Material>&, const RenderCommand& renderCommand)>& func,
			const bool& setMaterial);

		void OnCreate() override;

		std::unique_ptr<OpenGLUtils::GLBuffer> m_kernelBlock;

		

		friend class RenderTarget;

		friend class DefaultResources;

		friend class Editor;

		friend class EditorLayer;

		friend class LightProbe;

		friend class ReflectionProbe;

		friend class EnvironmentalMap;

		friend class Cubemap;

		friend class Graphics;

		size_t m_triangles = 0;
		size_t m_strandsSegments = 0;
		size_t m_drawCall = 0;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_materialSettingsBuffer;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_environmentalMapSettingsBuffer;
#pragma endregion
#pragma region Lightings


		std::unique_ptr<OpenGLUtils::GLBuffer> m_directionalLightBlock;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_pointLightBlock;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_spotLightBlock;

		size_t m_directionalLightShadowMapResolution = 4096;
		size_t m_pointLightShadowMapResolution = 2048;
		size_t m_spotLightShadowMapResolution = 2048;
		std::unique_ptr<OpenGLUtils::GLBuffer> m_shadowCascadeInfoBlock;

		DirectionalLightInfo m_directionalLights[DefaultResources::ShaderIncludes::MaxDirectionalLightAmount];
		PointLightInfo m_pointLights[DefaultResources::ShaderIncludes::MaxPointLightAmount];
		SpotLightInfo m_spotLights[DefaultResources::ShaderIncludes::MaxSpotLightAmount];

		std::unique_ptr<DirectionalLightShadowMap> m_directionalLightShadowMap;
		std::unique_ptr<PointLightShadowMap> m_pointLightShadowMap;
		std::unique_ptr<SpotLightShadowMap> m_spotLightShadowMap;

#pragma endregion
#pragma endregion
#pragma region internal helpers

		MaterialSettingsBlock m_materialSettings;
		EnvironmentalMapSettingsBlock m_environmentalMapSettings;

		void ApplyShadowMapSettings();

		void MaterialPropertySetter(const std::shared_ptr<Material>& material, const bool& disableBlending = false);

		void ApplyMaterialSettings(const std::shared_ptr<Material>& material);

		void ApplyProgramSettings(const std::shared_ptr<OpenGLUtils::GLProgram>& program,
			const std::shared_ptr<Material>& material);

		void ReleaseMaterialSettings(const std::shared_ptr<Material>& material);

		void ShadowMapPrePass(
			const int& enabledSize,
			std::shared_ptr<OpenGLUtils::GLProgram>& meshProgram,
			std::shared_ptr<OpenGLUtils::GLProgram>& meshInstancedProgram,
			std::shared_ptr<OpenGLUtils::GLProgram>& skinnedMeshProgram,
			std::shared_ptr<OpenGLUtils::GLProgram>& instancedSkinnedMeshProgram,
			std::shared_ptr<OpenGLUtils::GLProgram>& strandsProgram);

		void RenderShadows(
			Bound& worldBound, const std::shared_ptr<Camera>& cameraComponent, const GlobalTransform& cameraModel);

		void CollectRenderInstances(Bound& worldBound);

		void PrepareBrdfLut();

		void DeferredPrepassInternal(const std::shared_ptr<Mesh>& mesh);

		void DeferredPrepassInstancedInternal(
			const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<ParticleMatrices>& matrices);

		void DeferredPrepassInternal(const std::shared_ptr<SkinnedMesh>& skinnedMesh);
		void DeferredPrepassInternal(const std::shared_ptr<Strands>& strands);
		void DeferredPrepassInstancedInternal(
			const std::shared_ptr<SkinnedMesh>& skinnedMesh, const std::shared_ptr<ParticleMatrices>& matrices);

		void DrawMeshInternal(
			const std::shared_ptr<Mesh>& mesh,
			const std::shared_ptr<Material>& material,
			const glm::mat4& model,
			const bool& receiveShadow);

		void DrawMeshInternal(const std::shared_ptr<Mesh>& mesh);
		void DrawStrandsInternal(const std::shared_ptr<Strands>& strands);
		void DrawSkinnedMeshInternal(const std::shared_ptr<SkinnedMesh>& mesh);

		void DrawMeshInstancedInternal(
			const std::shared_ptr<Mesh>& mesh,
			const std::shared_ptr<Material>& material,
			const glm::mat4& model,
			const std::vector<glm::mat4>& matrices,
			const bool& receiveShadow);

		void DrawMeshInstancedInternal(
			const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<ParticleMatrices>& matrices);

		void DrawMeshInstancedInternal(
			const std::shared_ptr<SkinnedMesh>& mesh, const std::vector<glm::mat4>& matrices);



		float Lerp(const float& a, const float& b, const float& f);
	};


}