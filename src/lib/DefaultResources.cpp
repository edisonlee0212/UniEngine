#include <Application.hpp>
#include "ProjectManager.hpp"
#include <Collider.hpp>
#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include <LightProbe.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsMaterial.hpp>
#include <PostProcessing.hpp>
#include <Prefab.hpp>
#include <ReflectionProbe.hpp>
#include "Engine/Rendering/Graphics.hpp"
#include <Scene.hpp>
#include <SkinnedMesh.hpp>
#include <Utilities.hpp>
#include "Editor.hpp"
using namespace UniEngine;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_2DToCubemapProgram;

std::unique_ptr<Texture2D> DefaultResources::m_brdfLut;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferInstancedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferInstancedColoredPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferInstancedSkinnedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferSkinnedPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferStrandsPrepass;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_gBufferLightingPass;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightInstancedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_directionalLightInstancedStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_pointLightInstancedStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_spotLightInstancedStrandsProgram;


std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::ConvolutionProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::PrefilterProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::BrdfProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoVertexColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoNormalColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoInstancedColoredProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsVertexColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsNormalColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsInstancedColoredProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::SkyboxProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::ScreenProgram;
std::shared_ptr<OpenGLUtils::GLVAO> DefaultResources::ScreenVAO;
std::shared_ptr<OpenGLUtils::GLVAO> DefaultResources::SkyboxVAO;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightStrandsPrePassProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightSkinnedPrePassProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightPrePassInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightStrandsProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneHighlightInstancedSkinnedProgram;

std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityStrandsRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::m_sceneCameraEntityInstancedSkinnedRecorderProgram;



std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedColoredProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardInstancedSkinnedProgram;
std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GLPrograms::StandardStrandsProgram;

std::shared_ptr<OpenGLUtils::GLShader> DefaultResources::GLShaders::TexturePassThrough;

std::unique_ptr<std::string> DefaultResources::ShaderIncludes::Uniform;

std::shared_ptr<Texture2D> DefaultResources::Textures::MissingTexture;

std::shared_ptr<Mesh> DefaultResources::Primitives::Sphere;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cube;
std::shared_ptr<Mesh> DefaultResources::Primitives::Quad;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cone;
std::shared_ptr<Mesh> DefaultResources::Primitives::Torus;
std::shared_ptr<Mesh> DefaultResources::Primitives::Cylinder;
std::shared_ptr<Mesh> DefaultResources::Primitives::Monkey;
std::shared_ptr<Mesh> DefaultResources::Primitives::Capsule;

std::shared_ptr<Cubemap> DefaultResources::Environmental::DefaultSkybox;
std::shared_ptr<Cubemap> DefaultResources::Environmental::DefaultBlurredSkybox;
std::shared_ptr<EnvironmentalMap> DefaultResources::Environmental::DefaultEnvironmentalMap;

Handle DefaultResources::GenerateNewHandle()
{
	return GetInstance().m_currentMaxHandle.m_value++;
}

Handle DefaultResources::GetMaxHandle()
{
	return GetInstance().m_currentMaxHandle;
}

void DefaultResources::LoadShaders()
{
	int numberOfExtensions;
	OpenGLUtils::Get(GL_NUM_EXTENSIONS, numberOfExtensions);
	for (int i = 0; i < numberOfExtensions; i++)
	{
		const GLubyte* ccc = glGetStringi(GL_EXTENSIONS, i);
	}
#pragma region Shaders
#pragma region Shader Includes
	std::string add;

	add += "\n#define MAX_BONES_AMOUNT " + std::to_string(ShaderIncludes::MaxBonesAmount) +
		"\n#define MAX_TEXTURES_AMOUNT " + std::to_string(ShaderIncludes::MaxMaterialsAmount) +
		"\n#define DIRECTIONAL_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxDirectionalLightAmount) +
		"\n#define POINT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxPointLightAmount) +
		"\n#define SHADOW_CASCADE_AMOUNT " + std::to_string(ShaderIncludes::ShadowCascadeAmount) +
		"\n#define MAX_KERNEL_AMOUNT " + std::to_string(ShaderIncludes::MaxKernelAmount) +
		"\n#define SPOT_LIGHTS_AMOUNT " + std::to_string(ShaderIncludes::MaxSpotLightAmount) + "\n";

	ShaderIncludes::Uniform = std::make_unique<std::string>(
		add +
		FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Include/Uniform.glsl"));

#pragma endregion

#pragma region Standard Shader
	{
		auto vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Standard.vert");
		auto fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardForward.frag");
		auto fragColoredShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardForwardColored.frag");

		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Standard.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto standardFrag =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardForward.frag");
		auto standardColoredFrag =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardForwardColored.frag");

		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		standardColoredFrag->Set(OpenGLUtils::ShaderType::Fragment, fragColoredShaderCode);

		GLPrograms::StandardProgram =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard");
		GLPrograms::StandardProgram->Link(standardVert, standardFrag);
		vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardSkinned.vert");
		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardSkinned.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		GLPrograms::StandardSkinnedProgram =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Skinned");
		GLPrograms::StandardSkinnedProgram->Link(standardVert, standardFrag);
		vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstanced.vert");
		standardVert =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardInstanced.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		GLPrograms::StandardInstancedProgram =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Instanced");
		GLPrograms::StandardInstancedProgram->Link(standardVert, standardFrag);

		vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedColored.vert");
		standardVert =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardInstancedColored.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		GLPrograms::StandardInstancedColoredProgram =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Instanced Colored");
		GLPrograms::StandardInstancedColoredProgram->Link(standardVert, standardColoredFrag);


		vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedSkinned.vert");
		standardVert =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardInstancedSkinned.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		GLPrograms::StandardInstancedSkinnedProgram =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Instanced Skinned");
		GLPrograms::StandardInstancedSkinnedProgram->Link(standardVert, standardFrag);


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/StandardStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/StandardStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/StandardStrands.geom");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		GLPrograms::StandardStrandsProgram = ProjectManager::CreateDefaultResource<OpenGLUtils::GLProgram>(GenerateNewHandle(), "Standard Strands");
		GLPrograms::StandardStrandsProgram->Attach(standardVert);
		GLPrograms::StandardStrandsProgram->Attach(standardTessCont);
		GLPrograms::StandardStrandsProgram->Attach(standardTessEval);
		GLPrograms::StandardStrandsProgram->Attach(standardGeometry);
		GLPrograms::StandardStrandsProgram->Attach(standardFrag);
		GLPrograms::StandardStrandsProgram->Link();

	}


#pragma endregion
	{
		auto vertShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/TexturePassThrough.vert");
		GLShaders::TexturePassThrough =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "TexturePassThrough");
		GLShaders::TexturePassThrough->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
	}
#pragma region Post - Processing

	{
		Bloom::m_separateProgram = std::make_shared<OpenGLUtils::GLProgram>();
		auto fragShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BloomSeparator.frag");
		auto standardFrag =
			ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "BloomSeparator.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		Bloom::m_separateProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		Bloom::m_filterProgram = std::make_shared<OpenGLUtils::GLProgram>();
		fragShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BlurFilter.frag");
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		Bloom::m_filterProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		Bloom::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
		fragShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BloomCombine.frag");
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		Bloom::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSAOGeometry.frag");

		SSAO::m_geometryProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSAO::m_geometryProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BlurFilter.frag");

		SSAO::m_blurProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSAO::m_blurProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSAOCombine.frag");

		SSAO::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSAO::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);


		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSRReflect.frag");

		SSR::m_reflectProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSR::m_reflectProgram->Link(GLShaders::TexturePassThrough, standardFrag);
		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/BlurFilter.frag");

		SSR::m_blurProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSR::m_blurProgram->Link(GLShaders::TexturePassThrough, standardFrag);

		fragShaderCode = std::string("#version 460 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/SSRCombine.frag");
		SSR::m_combineProgram = std::make_shared<OpenGLUtils::GLProgram>();
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SSR::m_combineProgram->Link(GLShaders::TexturePassThrough, standardFrag);
	}
#pragma endregion
#pragma endregion
}

void DefaultResources::LoadPrimitives()
{
#pragma region Models &Primitives

	{
		Primitives::Quad = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Quad");
		Primitives::Quad->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/quad.uemesh");
	}
	{
		Primitives::Sphere = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Sphere");
		Primitives::Sphere->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/sphere.uemesh");
	}
	{
		Primitives::Cube = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Cube");
		Primitives::Cube->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/cube.uemesh");
	}
	{
		Primitives::Cone = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Cone");
		Primitives::Cone->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/cone.uemesh");
	}
	{
		Primitives::Cylinder = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Cylinder");
		Primitives::Cylinder->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/cylinder.uemesh");
	}
	{
		Primitives::Torus = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Torus");
		Primitives::Torus->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/torus.uemesh");
	}
	{
		Primitives::Monkey = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Monkey");
		Primitives::Monkey->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/monkey.uemesh");
	}
	{
		Primitives::Capsule = ProjectManager::CreateDefaultResource<Mesh>(GenerateNewHandle(), "Capsule");
		Primitives::Capsule->LoadInternal(std::filesystem::path("./DefaultResources") / "Primitives/capsule.uemesh");
	}

#pragma endregion
}
void DefaultResources::Load()
{
	LoadShaders();
	LoadPrimitives();

	LoadRenderManagerResources();
	LoadEditorManagerResources();

#pragma region Environmental
	Environmental::DefaultSkybox = ProjectManager::CreateDefaultResource<Cubemap>(GenerateNewHandle(), "DefaultSkybox");
	auto defaultSkyboxEquiTex = ProjectManager::CreateTemporaryAsset<Texture2D>();
	defaultSkyboxEquiTex->LoadInternal(std::filesystem::path("./DefaultResources") / "Textures/Cubemaps/GrandCanyon/GCanyon_C_YumaPoint_3k.hdr");
	Environmental::DefaultSkybox->ConvertFromEquirectangularTexture(defaultSkyboxEquiTex);

	Environmental::DefaultBlurredSkybox = ProjectManager::CreateDefaultResource<Cubemap>(GenerateNewHandle(), "DefaultBlurredSkybox");
	auto defaultBlurredSkyboxEquiTex = ProjectManager::CreateTemporaryAsset<Texture2D>();
	defaultBlurredSkyboxEquiTex->LoadInternal(std::filesystem::path("./DefaultResources") / "Textures/Cubemaps/GrandCanyon/GCanyon_C_YumaPoint_Env.hdr");
	Environmental::DefaultBlurredSkybox->ConvertFromEquirectangularTexture(defaultBlurredSkyboxEquiTex);

	Textures::MissingTexture = ProjectManager::CreateDefaultResource<Texture2D>(GenerateNewHandle(), "Missing");
	Textures::MissingTexture->LoadInternal(std::filesystem::path("./DefaultResources") / "Textures/texture-missing.png");

	Environmental::DefaultEnvironmentalMap =
		ProjectManager::CreateDefaultResource<EnvironmentalMap>(GenerateNewHandle(), "DefaultEnvMap");
	Environmental::DefaultEnvironmentalMap->ConstructFromCubemap(Environmental::DefaultSkybox);
#pragma endregion
}

void DefaultResources::OnDestroy()
{
	GetInstance() = {};
}

void DefaultResources::LoadRenderManagerResources()
{
#pragma region Skybox
	{
		float skyboxVertices[] = {// positions
								  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
								  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

								  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
								  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

								  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
								  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

								  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
								  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

								  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
								  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

								  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
								  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f };
		SkyboxVAO = std::make_shared<OpenGLUtils::GLVAO>();
		SkyboxVAO->SetData(sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		SkyboxVAO->EnableAttributeArray(0);
		SkyboxVAO->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		auto skyboxvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		std::string vertShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Skybox.vert"));
		skyboxvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto skyboxfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		std::string fragShaderCode = std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Skybox.frag"));
		skyboxfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		SkyboxProgram = std::make_shared<OpenGLUtils::GLProgram>();
		SkyboxProgram->Link(skyboxvert, skyboxfrag);
		SkyboxProgram->SetInt("skybox", 0);
	}
	{
		auto convertCubemapvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto vertShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
		convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto convertCubemapfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto fragShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EquirectangularMapToCubemap.frag"));
		convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		m_2DToCubemapProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_2DToCubemapProgram->Link(convertCubemapvert, convertCubemapfrag);
		m_2DToCubemapProgram->SetInt("equirectangularMap", 0);
	}

#pragma endregion
#pragma region Screen Shader
	{
		float quadVertices[] = {
			// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f };

		ScreenVAO = std::make_shared<OpenGLUtils::GLVAO>();
		ScreenVAO->SetData(sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		ScreenVAO->EnableAttributeArray(0);
		ScreenVAO->SetAttributePointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		ScreenVAO->EnableAttributeArray(1);
		ScreenVAO->SetAttributePointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		auto screenvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto vertShaderCode = std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Screen.vert"));
		screenvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto screenfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto fragShaderCode = std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Screen.frag"));
		screenfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		ScreenProgram = std::make_shared<OpenGLUtils::GLProgram>();
		ScreenProgram->Link(screenvert, screenfrag);
	}
#pragma endregion
#pragma region Lighting Shader
	{
		auto vertShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMap.vert");
		auto fragShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Empty.frag");
		std::string geomShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Geometry/DirectionalLightShadowMap.geom");

		auto vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		auto geomShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);
		m_directionalLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_directionalLightProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
			std::filesystem::path("./DefaultResources") /
			"Shaders/Vertex/LightShadowMapInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_directionalLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_directionalLightInstancedProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_directionalLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_directionalLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") /
				"Shaders/Vertex/LightShadowMapInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_directionalLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_directionalLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/DirectionalLightShadowMapStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/DirectionalLightShadowMapStrands.geom");


		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "LightShadowMapStrands.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "DirectionalLightShadowMapStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		geomShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "DirectionalLightShadowMapStrands.geom");
		geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_directionalLightStrandsProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_directionalLightStrandsProgram->Attach(vertShader);
		m_directionalLightStrandsProgram->Attach(standardTessCont);
		m_directionalLightStrandsProgram->Attach(standardTessEval);
		m_directionalLightStrandsProgram->Attach(geomShader);
		m_directionalLightStrandsProgram->Attach(fragShader);
		m_directionalLightStrandsProgram->Link();

		vertShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMap.vert");
		fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Empty.frag");
		geomShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Geometry/PointLightShadowMap.geom");

		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		geomShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_pointLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_pointLightProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
			std::filesystem::path("./DefaultResources") /
			"Shaders/Vertex/LightShadowMapInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_pointLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_pointLightInstancedProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_pointLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_pointLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_pointLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_pointLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapStrands.vert");

		tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/PointLightShadowMapStrands.tesc");

		tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/PointLightShadowMapStrands.geom");


		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "LightShadowMapStrands.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "PointLightShadowMapStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		geomShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "PointLightShadowMapStrands.geom");
		geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_pointLightStrandsProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_pointLightStrandsProgram->Attach(vertShader);
		m_pointLightStrandsProgram->Attach(standardTessCont);
		m_pointLightStrandsProgram->Attach(standardTessEval);
		m_pointLightStrandsProgram->Attach(geomShader);
		m_pointLightStrandsProgram->Attach(fragShader);
		m_pointLightStrandsProgram->Link();


		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMap.vert");
		fragShaderCode = std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Empty.frag");

		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		m_spotLightProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_spotLightProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_spotLightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_spotLightInstancedProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_spotLightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_spotLightSkinnedProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/SpotLightShadowMapInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_spotLightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_spotLightInstancedSkinnedProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/LightShadowMapStrands.vert");

		tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/SpotLightShadowMapStrands.tesc");

		tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/SpotLightShadowMapStrands.geom");


		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "SpotLightShadowMapStrands.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "SpotLightShadowMapStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		geomShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "SpotLightShadowMapStrands.geom");
		geomShader->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_spotLightStrandsProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_spotLightStrandsProgram->Attach(vertShader);
		m_spotLightStrandsProgram->Attach(standardTessCont);
		m_spotLightStrandsProgram->Attach(standardTessEval);
		m_spotLightStrandsProgram->Attach(geomShader);
		m_spotLightStrandsProgram->Attach(fragShader);
		m_spotLightStrandsProgram->Link();
	}
#pragma endregion
#pragma region GBuffer
	{
		auto fragShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardDeferredLighting.frag");
		auto fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		m_gBufferLightingPass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferLightingPass->Link(GLShaders::TexturePassThrough, fragShader);

		auto vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Standard.vert");
		fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardDeferred.frag");
		auto vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		m_gBufferPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferPrepass->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_gBufferSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferSkinnedPrepass->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_gBufferInstancedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferInstancedPrepass->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_gBufferInstancedSkinnedPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferInstancedSkinnedPrepass->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardInstancedColored.vert");
		auto fragColoredShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/StandardDeferredColored.frag");
		auto vertColoredShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		vertColoredShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto fragColoredShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		fragColoredShader->Set(OpenGLUtils::ShaderType::Fragment, fragColoredShaderCode);
		m_gBufferInstancedColoredPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferInstancedColoredPrepass->Link(vertColoredShader, fragColoredShader);


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/StandardStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/StandardStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/StandardStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/StandardStrands.geom");


		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "StandardStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_gBufferStrandsPrepass = std::make_shared<OpenGLUtils::GLProgram>();
		m_gBufferStrandsPrepass->Attach(standardVert);
		m_gBufferStrandsPrepass->Attach(standardTessCont);
		m_gBufferStrandsPrepass->Attach(standardTessEval);
		m_gBufferStrandsPrepass->Attach(standardGeometry);
		m_gBufferStrandsPrepass->Attach(fragShader);
		m_gBufferStrandsPrepass->Link();

	}
#pragma endregion

	{
		auto convertCubemapvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto vertShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
		convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto convertCubemapfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto fragShaderCode =
			std::string("#version 450 core\n") + std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") /
				"Shaders/Fragment/EnvironmentalMapIrradianceConvolution.frag"));
		convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		ConvolutionProgram = std::make_shared<OpenGLUtils::GLProgram>();
		ConvolutionProgram->Link(convertCubemapvert, convertCubemapfrag);
		ConvolutionProgram->SetInt("environmentMap", 0);
	}
	{
		auto convertCubemapvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto vertShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EquirectangularMapToCubemap.vert"));
		convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto convertCubemapfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "");
		auto fragShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EnvironmentalMapPrefilter.frag"));
		convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		PrefilterProgram = std::make_shared<OpenGLUtils::GLProgram>();
		PrefilterProgram->Link(convertCubemapvert, convertCubemapfrag);
		PrefilterProgram->SetInt("environmentMap", 0);
	}
	{
		auto convertCubemapvert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EnvironmentalMapBrdf.vert");
		auto vertShaderCode = std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EnvironmentalMapBrdf.vert"));
		convertCubemapvert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto convertCubemapfrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EnvironmentalMapBrdf.frag");
		auto fragShaderCode =
			std::string("#version 450 core\n") +
			std::string(FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Fragment/EnvironmentalMapBrdf.frag"));
		convertCubemapfrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		BrdfProgram = std::make_shared<OpenGLUtils::GLProgram>();
		BrdfProgram->Link(convertCubemapvert, convertCubemapfrag);
	}

#pragma region Gizmo Shader
	{
		auto vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Gizmo.vert");

		auto fragShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Gizmo.frag");

		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Gizmo.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Gizmo.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoProgram->Attach(standardVert);
		GizmoProgram->Attach(standardFrag);
		GizmoProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstanced.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoInstanced.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Gizmo.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoInstancedProgram->Attach(standardVert);
		GizmoInstancedProgram->Attach(standardFrag);
		GizmoInstancedProgram->Link();

		fragShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/GizmoColored.frag");

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoVertexColored.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoVertexColored.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColored.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoVertexColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoVertexColoredProgram->Attach(standardVert);
		GizmoVertexColoredProgram->Attach(standardFrag);
		GizmoVertexColoredProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoNormalColored.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoNormalColored.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColored.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoNormalColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoNormalColoredProgram->Attach(standardVert);
		GizmoNormalColoredProgram->Attach(standardFrag);
		GizmoNormalColoredProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstancedColored.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoInstancedColored.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColored.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoInstancedColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoInstancedColoredProgram->Attach(standardVert);
		GizmoInstancedColoredProgram->Attach(standardFrag);
		GizmoInstancedColoredProgram->Link();


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstancedColored.vert");

		fragShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/GizmoColored.frag");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoInstancedColored.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColored.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);
		GizmoInstancedColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoInstancedColoredProgram->Attach(standardVert);
		GizmoInstancedColoredProgram->Attach(standardFrag);
		GizmoInstancedColoredProgram->Link();
	}


	{
		auto vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/GizmoStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/GizmoStrands.geom");

		auto fragShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Gizmo.frag");

		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		auto standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Gizmo.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		GizmoStrandsProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoStrandsProgram->Attach(standardVert);
		GizmoStrandsProgram->Attach(standardTessCont);
		GizmoStrandsProgram->Attach(standardTessEval);
		GizmoStrandsProgram->Attach(standardGeometry);
		GizmoStrandsProgram->Attach(standardFrag);
		GizmoStrandsProgram->Link();


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstancedStrands.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoInstancedStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		GizmoStrandsInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoStrandsInstancedProgram->Attach(standardVert);
		GizmoStrandsInstancedProgram->Attach(standardTessCont);
		GizmoStrandsInstancedProgram->Attach(standardTessEval);
		GizmoStrandsInstancedProgram->Attach(standardGeometry);
		GizmoStrandsInstancedProgram->Attach(standardFrag);
		GizmoStrandsInstancedProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoVertexColoredStrands.vert");

		tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/GizmoColoredStrands.tesc");

		tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoColoredStrands.tese");

		geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/GizmoColoredStrands.geom");

		fragShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/GizmoColored.frag");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoVertexColoredStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColoredStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColoredStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColoredStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		standardFrag = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoColored.frag");
		standardFrag->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		GizmoStrandsVertexColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoStrandsVertexColoredProgram->Attach(standardVert);
		GizmoStrandsVertexColoredProgram->Attach(standardTessCont);
		GizmoStrandsVertexColoredProgram->Attach(standardTessEval);
		GizmoStrandsVertexColoredProgram->Attach(standardGeometry);
		GizmoStrandsVertexColoredProgram->Attach(standardFrag);
		GizmoStrandsVertexColoredProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoNormalColoredStrands.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoNormalColoredStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		GizmoStrandsNormalColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoStrandsNormalColoredProgram->Attach(standardVert);
		GizmoStrandsNormalColoredProgram->Attach(standardTessCont);
		GizmoStrandsNormalColoredProgram->Attach(standardTessEval);
		GizmoStrandsNormalColoredProgram->Attach(standardGeometry);
		GizmoStrandsNormalColoredProgram->Attach(standardFrag);
		GizmoStrandsNormalColoredProgram->Link();

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/GizmoInstancedColoredStrands.vert");

		standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoInstancedColoredStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		GizmoStrandsInstancedColoredProgram = std::make_shared<OpenGLUtils::GLProgram>();
		GizmoStrandsInstancedColoredProgram->Attach(standardVert);
		GizmoStrandsInstancedColoredProgram->Attach(standardTessCont);
		GizmoStrandsInstancedColoredProgram->Attach(standardTessEval);
		GizmoStrandsInstancedColoredProgram->Attach(standardGeometry);
		GizmoStrandsInstancedColoredProgram->Attach(standardFrag);
		GizmoStrandsInstancedColoredProgram->Link();
	}
#pragma endregion

	PrepareBrdfLut();
}

void DefaultResources::PrepareBrdfLut()
{
	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	auto brdfLut = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RG16F, 512, 512, true);
	m_brdfLut = std::make_unique<Texture2D>();
	m_brdfLut->m_texture = std::move(brdfLut);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_brdfLut->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_brdfLut->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	size_t resolution = 512;
	auto renderTarget = std::make_shared<RenderTarget>(resolution, resolution);
	auto renderBuffer = std::make_shared<OpenGLUtils::GLRenderBuffer>();
	renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
	renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
	renderTarget->AttachTexture(m_brdfLut->m_texture.get(), GL_COLOR_ATTACHMENT0);
	OpenGLUtils::SetViewPort(resolution, resolution);
	DefaultResources::BrdfProgram->Bind();
	renderTarget->Clear();
	Graphics::RenderQuad();
	OpenGLUtils::GLFrameBuffer::BindDefault();
}
void DefaultResources::LoadEditorManagerResources()
{
#pragma region Recorder
	{
		std::string vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Empty.vert");
		std::string fragShaderCode = std::string("#version 450 core\n") + FileUtils::LoadFileAsString(
			std::filesystem::path("./DefaultResources") /
			"Shaders/Fragment/EntityRecorder.frag");

		auto vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Empty.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		auto fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EntityRecorder.frag");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		m_sceneCameraEntityRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneCameraEntityRecorderProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyInstanced.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneCameraEntityInstancedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneCameraEntityInstancedRecorderProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptySkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptySkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneCameraEntitySkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneCameraEntitySkinnedRecorderProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyInstancedSkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneCameraEntityInstancedSkinnedRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneCameraEntityInstancedSkinnedRecorderProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/GizmoStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/GizmoStrands.geom");


		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_sceneCameraEntityStrandsRecorderProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneCameraEntityStrandsRecorderProgram->Attach(standardVert);
		m_sceneCameraEntityStrandsRecorderProgram->Attach(standardTessCont);
		m_sceneCameraEntityStrandsRecorderProgram->Attach(standardTessEval);
		m_sceneCameraEntityStrandsRecorderProgram->Attach(standardGeometry);
		m_sceneCameraEntityStrandsRecorderProgram->Attach(fragShader);
		m_sceneCameraEntityStrandsRecorderProgram->Link();
	}
#pragma endregion
#pragma region Highlight Prepass
	{
		auto vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Empty.vert");
		auto vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Empty.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto fragShaderCode =
			std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Highlight.frag");

		auto fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Highlight.frag");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		m_sceneHighlightPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightPrePassProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyInstanced.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightPrePassInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightPrePassInstancedProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptySkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptySkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightSkinnedPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightSkinnedPrePassProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyInstancedSkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightPrePassInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightPrePassInstancedSkinnedProgram->Link(vertShader, fragShader);


		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/GizmoStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/GizmoStrands.geom");


		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_sceneHighlightStrandsPrePassProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightStrandsPrePassProgram->Attach(standardVert);
		m_sceneHighlightStrandsPrePassProgram->Attach(standardTessCont);
		m_sceneHighlightStrandsPrePassProgram->Attach(standardTessEval);
		m_sceneHighlightStrandsPrePassProgram->Attach(standardGeometry);
		m_sceneHighlightStrandsPrePassProgram->Attach(fragShader);
		m_sceneHighlightStrandsPrePassProgram->Link();
	}
#pragma endregion
#pragma region Highlight
	{
		auto vertShaderCode =
			std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/Highlight.vert");
		auto vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Highlight.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto fragShaderCode =
			std::string("#version 450 core\n") +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Fragment/Highlight.frag");

		auto fragShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "Highlight.frag");
		fragShader->Set(OpenGLUtils::ShaderType::Fragment, fragShaderCode);

		m_sceneHighlightProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightInstanced.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "HighlightInstanced.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightInstancedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightInstancedProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "HighlightSkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightSkinnedProgram->Link(vertShader, fragShader);

		vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(
				std::filesystem::path("./DefaultResources") / "Shaders/Vertex/HighlightInstancedSkinned.vert");
		vertShader = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "HighlightInstancedSkinned.vert");
		vertShader->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);
		m_sceneHighlightInstancedSkinnedProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightInstancedSkinnedProgram->Link(vertShader, fragShader);

		vertShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Vertex/EmptyStrands.vert");

		auto tessContShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationControl/GizmoStrands.tesc");

		auto tessEvalShaderCode =
			std::string("#version 450 core\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/TessellationEvaluation/GizmoStrands.tese");

		auto geomShaderCode = std::string("#version 450 core\n") + std::string("#extension GL_EXT_geometry_shader4 : enable\n") + *ShaderIncludes::Uniform + "\n" +
			FileUtils::LoadFileAsString(std::filesystem::path("./DefaultResources") / "Shaders/Geometry/HighlightStrands.geom");


		auto standardVert = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "EmptyStrands.vert");
		standardVert->Set(OpenGLUtils::ShaderType::Vertex, vertShaderCode);

		auto standardTessCont = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tesc");
		standardTessCont->Set(OpenGLUtils::ShaderType::TessellationControl, tessContShaderCode);

		auto standardTessEval = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "GizmoStrands.tese");
		standardTessEval->Set(OpenGLUtils::ShaderType::TessellationEvaluation, tessEvalShaderCode);

		auto standardGeometry = ProjectManager::CreateDefaultResource<OpenGLUtils::GLShader>(GenerateNewHandle(), "HighlightStrands.geom");
		standardGeometry->Set(OpenGLUtils::ShaderType::Geometry, geomShaderCode);

		m_sceneHighlightStrandsProgram = std::make_shared<OpenGLUtils::GLProgram>();
		m_sceneHighlightStrandsProgram->Attach(standardVert);
		m_sceneHighlightStrandsProgram->Attach(standardTessCont);
		m_sceneHighlightStrandsProgram->Attach(standardTessEval);
		m_sceneHighlightStrandsProgram->Attach(standardGeometry);
		m_sceneHighlightStrandsProgram->Attach(fragShader);
		m_sceneHighlightStrandsProgram->Link();
		
	}
#pragma endregion

	LoadIcons();
}
void DefaultResources::LoadIcons()
{
	auto& editorManager = Editor::GetInstance();
	auto texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/project.png");
	editorManager.m_assetsIcons["Project"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/scene.png");
	editorManager.m_assetsIcons["Scene"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/binary.png");
	editorManager.m_assetsIcons["Binary"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/folder.png");
	editorManager.m_assetsIcons["Folder"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/material.png");
	editorManager.m_assetsIcons["Material"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/mesh.png");
	editorManager.m_assetsIcons["Mesh"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/prefab.png");
	editorManager.m_assetsIcons["Prefab"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Assets/texture2d.png");
	editorManager.m_assetsIcons["Texture2D"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/PlayButton.png");
	editorManager.m_assetsIcons["PlayButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/PauseButton.png");
	editorManager.m_assetsIcons["PauseButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/StopButton.png");
	editorManager.m_assetsIcons["StopButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/StepButton.png");
	editorManager.m_assetsIcons["StepButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/back.png");
	editorManager.m_assetsIcons["BackButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/left.png");
	editorManager.m_assetsIcons["LeftButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/right.png");
	editorManager.m_assetsIcons["RightButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Navigation/refresh.png");
	editorManager.m_assetsIcons["RefreshButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Console/InfoButton.png");
	editorManager.m_assetsIcons["InfoButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Console/ErrorButton.png");
	editorManager.m_assetsIcons["ErrorButton"] = texture2D;

	texture2D = std::make_shared<Texture2D>();
	texture2D->LoadInternal(std::filesystem::path("./DefaultResources") / "Editor/Console/WarningButton.png");
	editorManager.m_assetsIcons["WarningButton"] = texture2D;

}
