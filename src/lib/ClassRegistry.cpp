#include <AssetManager.hpp>
#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <EntityManager.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsLayer.hpp>
#include <PlayerController.hpp>
#include <PointCloud.hpp>
#include <PostProcessing.hpp>
#include <RenderManager.hpp>
#include <UnknownPrivateComponent.hpp>
using namespace UniEngine;

DataComponentRegistration<Transform> TransformRegistry("Transform");
DataComponentRegistration<GlobalTransform> GlobalTransformRegistry("GlobalTransform");
DataComponentRegistration<GlobalTransformUpdateFlag> GlobalTransformUpdateFlagRegistry("GlobalTransformUpdateFlag");
DataComponentRegistration<Ray> RayRegistry("Ray");

PrivateComponentRegistration<Animator> AnimatorRegistry("Animator");
PrivateComponentRegistration<Joint> JointRegistry("Joint");
PrivateComponentRegistration<RigidBody> RigidBodyRegistry("RigidBody");
PrivateComponentRegistration<SpotLight> SpotLightRegistry("SpotLight");
PrivateComponentRegistration<PointLight> PointLightRegistry("PointLight");
PrivateComponentRegistration<DirectionalLight> DirectionalLightRegistry("DirectionalLight");
PrivateComponentRegistration<Camera> CameraRegistry("Camera");
PrivateComponentRegistration<PlayerController> PlayerControllerRegistry("PlayerController");
PrivateComponentRegistration<Particles> ParticlesRegistry("Particles");
PrivateComponentRegistration<MeshRenderer> MeshRendererRegistry("MeshRenderer");
PrivateComponentRegistration<PostProcessing> PostProcessingRegistry("PostProcessing");
PrivateComponentRegistration<SkinnedMeshRenderer> SkinnedMeshRendererRegistry("SkinnedMeshRenderer");
PrivateComponentRegistration<PointCloud> PointCloudRegistry("PointCloud");
PrivateComponentRegistration<UnknownPrivateComponent> UnknownPrivateComponentRegistry("UnknownPrivateComponent");

SystemRegistration<PhysicsSystem> PhysicsSystemRegistry("PhysicsSystem");

AssetRegistration<Material> MaterialRegistry("Material", ".uemat");
AssetRegistration<Mesh> MeshRegistry("Mesh", ".uemesh");
AssetRegistration<Texture2D> Texture2DReg("Texture2D", ".uetexture2d");
AssetRegistration<Cubemap> CubemapReg("Cubemap", ".uecubemap");
AssetRegistration<LightProbe> LightProbeReg("LightProbe", ".uelightprobe");
AssetRegistration<ReflectionProbe> ReflectionProbeReg("ReflectionProbe", ".uereflecprobe");
AssetRegistration<OpenGLUtils::GLProgram> GLProgramReg("GLProgram", ".ueglprogram");
AssetRegistration<OpenGLUtils::GLShader> GLShaderReg("GLShader", ".ueglshader");

AssetRegistration<EnvironmentalMap> EnvironmentalMapReg("EnvironmentalMap", ".ueenvirmap");
AssetRegistration<Animation> AnimationReg("Animation", ".ueanimation");
AssetRegistration<SkinnedMesh> SkinnedMeshReg("SkinnedMesh", "ueskinnedmesh");
AssetRegistration<PhysicsMaterial> PhysicsMaterialReg("PhysicsMaterial", "uephysmat");
AssetRegistration<Collider> ColliderReg("Collider", "uecollider");
AssetRegistration<Prefab> PrefabReg("Prefab", ".ueprefab");
AssetRegistration<Scene> SceneReg("Scene", ".uescene");

SerializableRegistration<Project> ProjectRegistryReg("Project");
