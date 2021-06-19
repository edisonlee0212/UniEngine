#pragma once
#include <OpenGLUtils.hpp>
#include <World.hpp>
namespace UniEngine
{
class Mesh;
class Material;
class Cubemap;
class UNIENGINE_API Default
{
  public:
    class UNIENGINE_API GLPrograms
    {
      public:
        static std::shared_ptr<OpenGLUtils::GLProgram> ScreenProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> BackGroundProgram;
        static OpenGLUtils::GLVAO *ScreenVAO;

        static std::shared_ptr<OpenGLUtils::GLVAO> SkyboxVAO;

        static std::shared_ptr<OpenGLUtils::GLProgram> SkyboxProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> StandardInstancedProgram;

        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedProgram;
        static std::shared_ptr<OpenGLUtils::GLProgram> GizmoInstancedColoredProgram;
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
        static std::shared_ptr<Cubemap> DefaultSkybox;
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

    static void Load(World *world);
};
} // namespace UniEngine