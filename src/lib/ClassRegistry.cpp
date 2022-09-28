#include "Engine/ECS/Entities.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsLayer.hpp>
#include <PlayerController.hpp>
#include <PointCloud.hpp>
#include <PostProcessing.hpp>
#include <UnknownPrivateComponent.hpp>
#include "Prefab.hpp"
#include "StrandsRenderer.hpp"
using namespace UniEngine;

DataComponentRegistration<Ray> RayRegistry("Ray");

PrivateComponentRegistration<Joint> JointRegistry("Joint");
PrivateComponentRegistration<RigidBody> RigidBodyRegistry("RigidBody");
PrivateComponentRegistration<Camera> CameraRegistry("Camera");
PrivateComponentRegistration<PlayerController> PlayerControllerRegistry("PlayerController");
PrivateComponentRegistration<Particles> ParticlesRegistry("Particles");
PrivateComponentRegistration<MeshRenderer> MeshRendererRegistry("MeshRenderer");
PrivateComponentRegistration<PostProcessing> PostProcessingRegistry("PostProcessing");
PrivateComponentRegistration<SkinnedMeshRenderer> SkinnedMeshRendererRegistry("SkinnedMeshRenderer");

PrivateComponentRegistration<UnknownPrivateComponent> UnknownPrivateComponentRegistry("UnknownPrivateComponent");

SystemRegistration<PhysicsSystem> PhysicsSystemRegistry("PhysicsSystem");

AssetRegistration<IAsset> IAssetRegistry("IAsset", {".ueasset"});
AssetRegistration<Material> MaterialRegistry("Material", {".uemat"});
AssetRegistration<Mesh> MeshRegistry("Mesh", {".uemesh"});
AssetRegistration<Texture2D> Texture2DReg("Texture2D", {".png", ".jpg", ".jpeg", ".tga", ".hdr"});
AssetRegistration<Cubemap> CubemapReg("Cubemap", {".uecubemap"});
AssetRegistration<LightProbe> LightProbeReg("LightProbe", {".uelightprobe"});
AssetRegistration<ReflectionProbe> ReflectionProbeReg("ReflectionProbe", {".uereflecprobe"});
AssetRegistration<OpenGLUtils::GLProgram> GLProgramReg("GLProgram", {".ueglprogram"});
AssetRegistration<OpenGLUtils::GLShader> GLShaderReg("GLShader", {".ueglshader"});



AssetRegistration<Animation> AnimationReg("Animation", {".ueanimation"});
AssetRegistration<SkinnedMesh> SkinnedMeshReg("SkinnedMesh", {"ueskinnedmesh"});
AssetRegistration<PhysicsMaterial> PhysicsMaterialReg("PhysicsMaterial", {"uephysmat"});


