#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsManager.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <RenderManager.hpp>
#include <AssetManager.hpp>
#include <EntityManager.hpp>
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

SystemRegistration<PhysicsSystem> PhysicsSystemRegistry("PhysicsSystem");

AssetRegistration<Material> MaterialRegistry("Material");
AssetRegistration<Mesh> MeshRegistry("Mesh");
AssetRegistration<Texture2D> Texture2DReg("Texture2D");
AssetRegistration<Cubemap> CubemapReg("Cubemap");
AssetRegistration<LightProbe> LightProbeReg("LightProbe");
AssetRegistration<ReflectionProbe> ReflectionProbeReg("ReflectionProbe");
AssetRegistration<OpenGLUtils::GLProgram> GLProgramReg("GLProgram");
AssetRegistration<EnvironmentalMap> EnvironmentalMapReg("EnvironmentalMap");
AssetRegistration<Animation> AnimationReg("Animation");
AssetRegistration<SkinnedMesh> SkinnedMeshReg("SkinnedMesh");
AssetRegistration<PhysicsMaterial> PhysicsMaterialReg("PhysicsMaterial");
AssetRegistration<Collider> ColliderReg("Collider");
AssetRegistration<Prefab> PrefabReg("Prefab");
AssetRegistration<Scene> SceneReg("Scene");

SerializableRegistration<AssetRegistry> AssetRegistryReg("AssetRegistry");
SerializableRegistration<Project> ProjectRegistryReg("Project");
SerializableRegistration<EntityRef> EntityRefReg("EntityRef");
SerializableRegistration<AssetRef> AssetRefReg("AssetRef");
SerializableRegistration<PrivateComponentRef> PrivateComponentRefReg("PrivateComponentRef");
