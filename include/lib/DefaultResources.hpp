#pragma once
#include <OpenGLUtils.hpp>
#include <World.hpp>
namespace UniEngine
{
class LightProbe;
class ReflectionProbe;
class Mesh;
class Material;
class Cubemap;
class UNIENGINE_API DefaultResources
{
    static void LoadShaders(World* world);
    static void LoadTextures(World *world);
    static void LoadPrimitives(World *world);
  public:
    class UNIENGINE_API GLPrograms
    {
      public:
        static std::shared_ptr<OpenGLUtils::GLProgram> ScreenProgram;
        static OpenGLUtils::GLVAO *ScreenVAO;

        static std::shared_ptr<OpenGLUtils::GLVAO> SkyboxVAO;

        static std::shared_ptr<OpenGLUtils::GLProgram> SkyboxProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardInstancedProgram;

        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedColoredProgram;

        static std::shared_ptr<OpenGLUtils::GLProgram> ConvolutionProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> PrefilterProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> BrdfProgram;
    };
    class UNIENGINE_API Materials
    {
      public:
        static std::shared_ptr<Material> StandardMaterial;
        static std::shared_ptr<Material> StandardInstancedMaterial;
    };
    class UNIENGINE_API ShaderIncludes
    {
      public:
        static std::string *Uniform;
        const static size_t MaxBonesAmount = 1024;
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
        static std::shared_ptr<Texture2D> UV;
        static std::shared_ptr<Texture2D> ObjectIcon;
        static std::shared_ptr<Texture2D> StandardTexture;
        static std::shared_ptr<Texture2D> Border;
    };

    class UNIENGINE_API Primitives
    {
      public:
        static std::shared_ptr<Mesh> Sphere;
        static std::shared_ptr<Mesh> Cube;
        static std::shared_ptr<Mesh> Quad;
        static std::shared_ptr<Mesh> Cylinder;
        static std::shared_ptr<Mesh> Cone;
        static std::shared_ptr<Mesh> Ring;
        static std::shared_ptr<Mesh> Monkey;
    };
    class UNIENGINE_API Environmental
    {
      public:
        static std::shared_ptr<Cubemap> DefaultSkybox;
        static std::shared_ptr<Cubemap> MilkyWaySkybox;
        static std::shared_ptr<Cubemap> CircusSkybox;

        static std::shared_ptr<LightProbe> DefaultLightProbe;
        static std::shared_ptr<LightProbe> MilkyWayLightProbe;
        static std::shared_ptr<LightProbe> CircusLightProbe;

        static std::shared_ptr<ReflectionProbe> DefaultReflectionProbe;
        static std::shared_ptr<ReflectionProbe> MilkyWayReflectionProbe;
        static std::shared_ptr<ReflectionProbe> CircusReflectionProbe;
    };
    
    static void Load(World *world);

};
} // namespace UniEngine