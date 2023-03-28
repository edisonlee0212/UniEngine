#pragma once
#include <OpenGLUtils.hpp>
#include <ISingleton.hpp>
#include <Scene.hpp>
#include <PostProcessing.hpp>
namespace UniEngine
{
class EnvironmentalMap;
class Cubemap;
class Mesh;
class Material;
class PhysicsMaterial;

class UNIENGINE_API DefaultResources : ISingleton<DefaultResources>
{
    static void LoadShaders();
    static void LoadPrimitives();
    static void LoadIcons();
    static void LoadRenderManagerResources();
    static void LoadEditorManagerResources();
    static void PrepareBrdfLut();
    friend class IAsset;
    friend class Graphics;
    friend class Editor;
    friend class LightProbe;
    friend class ReflectionProbe;
    friend class EnvironmentalMap;
    friend class Cubemap;
    friend class PostProcessing;
    friend class SSAO;
    friend class Bloom;
    friend class SSR;
    friend class Windows;
    friend class EditorLayer;
    friend class RenderLayer;
    friend class Gizmos;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_2DToCubemapProgram;

    static std::unique_ptr<Texture2D> m_brdfLut;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferInstancedPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferInstancedColoredPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferInstancedSkinnedPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferSkinnedPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferStrandsPrepass;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_gBufferLightingPass;



    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightInstancedProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightInstancedSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightInstancedSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightInstancedSkinnedProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_directionalLightInstancedStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_pointLightInstancedStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_spotLightInstancedStrandsProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> GizmoProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> GizmoVertexColoredProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> GizmoNormalColoredProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedColoredProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsVertexColoredProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsNormalColoredProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> DefaultResources::GizmoStrandsInstancedColoredProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> ConvolutionProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> PrefilterProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> BrdfProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> ScreenProgram;
    static std::shared_ptr<OpenGLUtils::GLVAO> ScreenVAO;
    static std::shared_ptr<OpenGLUtils::GLVAO> SkyboxVAO;
    static std::shared_ptr<OpenGLUtils::GLProgram> SkyboxProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightStrandsPrePassProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightSkinnedPrePassProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassInstancedSkinnedProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightStrandsProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightSkinnedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightInstancedProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightInstancedSkinnedProgram;

    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityRecorderProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityStrandsRecorderProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntitySkinnedRecorderProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityInstancedRecorderProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityInstancedSkinnedRecorderProgram;
    Handle m_currentMaxHandle = Handle(1);
  public:
    static Handle GetMaxHandle();
    static Handle GenerateNewHandle();
    class UNIENGINE_API GLPrograms
    {
      public:
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardInstancedProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardInstancedColoredProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardSkinnedProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardInstancedSkinnedProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardStrandsProgram;
    };

    class UNIENGINE_API GLShaders
    {
      public:
        static std::shared_ptr<OpenGLUtils::GLShader> TexturePassThrough;
    };

    class UNIENGINE_API ShaderIncludes
    {
      public:
        static std::unique_ptr<std::string> Uniform;
        const static size_t MaxBonesAmount = 65536;
        const static size_t MaxMaterialsAmount = 1;
        const static size_t MaxKernelAmount = 64;
        const static size_t MaxDirectionalLightAmount = 128;
        const static size_t MaxPointLightAmount = 128;
        const static size_t MaxSpotLightAmount = 128;
        const static size_t ShadowCascadeAmount = 4;
    };

    class UNIENGINE_API Textures
    {
      public:
        static std::shared_ptr<Texture2D> MissingTexture;
    };

    class UNIENGINE_API Primitives
    {
      public:
        static std::shared_ptr<Mesh> Sphere;
        static std::shared_ptr<Mesh> Cube;
        static std::shared_ptr<Mesh> Quad;
        static std::shared_ptr<Mesh> Cylinder;
        static std::shared_ptr<Mesh> Cone;
        static std::shared_ptr<Mesh> Torus;
        static std::shared_ptr<Mesh> Monkey;
        static std::shared_ptr<Mesh> Capsule;
    };
    class UNIENGINE_API Environmental
    {
      public:
        static std::shared_ptr<Cubemap> DefaultSkybox;
        static std::shared_ptr<Cubemap> DefaultBlurredSkybox;
        static std::shared_ptr<EnvironmentalMap> DefaultEnvironmentalMap;
    };

    static void Load();
    static void OnDestroy();
};

} // namespace UniEngine