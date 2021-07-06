#pragma once
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <Model.hpp>
#include <OpenGLUtils.hpp>
#include <LightProbe.hpp>
#include <ReflectionProbe.hpp>
#include <EnvironmentalMap.hpp>
namespace UniEngine
{
#ifdef USE_ASSIMP

struct UNIENGINE_API AssimpNode
{
	aiNode *m_correspondingNode = nullptr;
	std::string m_name;
	Transform m_localTransform;
	AssimpNode(aiNode *node);
	std::shared_ptr<AssimpNode> m_parent;
	std::vector<std::shared_ptr<AssimpNode>> m_children;
	std::shared_ptr<Bone> m_bone;
	bool m_hasMesh;

	bool NecessaryWalker(std::map<std::string, std::shared_ptr<Bone>> &boneMap);
	void AttachToAnimator(std::shared_ptr<Animation> &animation, size_t& index);
    void AttachChild(std::shared_ptr<Bone> &parent, size_t& index);
};

#endif
class UNIENGINE_API ResourceManager : public ISingleton<ResourceManager>
{
	bool m_enableAssetMenu = true;
	std::map<size_t, std::pair<std::string, std::map<size_t, std::shared_ptr<ResourceBehaviour>>>> m_resources;
	std::shared_ptr<OpenGLUtils::GLProgram> m_2DToCubemapProgram;
	friend class DefaultResources;

	static std::shared_ptr<Texture2D> CollectTexture(
		const std::string &directory,
		const std::string &path,
		std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures,
		const float &gamma);
#ifdef USE_ASSIMP
	static void ReadAnimations(
		const aiScene *importerScene,
		std::shared_ptr<Animation> &animator, std::map<std::string, std::shared_ptr<Bone>> &bonesMap);
	static void ReadKeyFrame(BoneKeyFrames &boneAnimation, const aiNodeAnim *channel);
	static std::shared_ptr<Material> ReadMaterial(
		const std::string &directory,
		const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
		std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
		aiMaterial *importerMaterial,
		const float &gamma);
	static bool ProcessNode(
		const std::string &directory,
		std::shared_ptr<ModelNode> &modelNode,
		std::map<unsigned, std::shared_ptr<Material>> &loadedMaterials,
		std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
		std::map<std::string, std::shared_ptr<Bone>> &bonesMap, 
		aiNode *importerNode,
		std::shared_ptr<AssimpNode> assimpNode,
		const aiScene *importerScene,
        const std::shared_ptr<Animation>& animator,
		const float &gamma);
	static std::shared_ptr<Mesh> ReadMesh(aiMesh *importerMesh);
	static std::shared_ptr<SkinnedMesh> ReadSkinnedMesh(std::map<std::string, std::shared_ptr<Bone>> &bonesMap, aiMesh *importerMesh);
#else
	
	static void ProcessNode(
		const std::string &directory,
		std::map<int, std::vector<Vertex>> &meshMaterials,
		const tinyobj::shape_t &shape,
		const tinyobj::attrib_t &attribute);
#endif

	static void AttachChildren(
		EntityArchetype archetype, std::shared_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName);
	friend class EditorManager;
	static std::string GetTypeName(size_t id);
  public:
	template <typename T> 
	static std::string GetTypeName();
	static std::string GetTypeName(const std::shared_ptr<ResourceBehaviour> &resource);
	template <typename T> 
	static void RegisterResourceType(const std::string &name);
	template <typename T> static std::shared_ptr<T> CreateResource(const bool &addResource = false, const std::string& name = "");
	template <typename T> static void Push(std::shared_ptr<T> resource);
	template <typename T> static std::shared_ptr<T> Get(size_t hashCode);
	template <typename T> static std::shared_ptr<T> Find(std::string objectName);
	template <typename T> static void Remove(size_t hashCode);
	static void Remove(size_t id, size_t hashCode);
	static std::shared_ptr<Model> LoadModel(const bool &addResource,
											std::string const &path,
		const unsigned &flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph,
		const bool &optimize = false,
		const float &gamma = 0.0f
		);
	static std::shared_ptr<Texture2D> LoadTexture(
		const bool &addResource,
		const std::string &path,
		const float &gamma = 0.0f);
	static std::shared_ptr<Cubemap> LoadCubemap(
		const bool &addResource,
		const std::string &path,
		const float &gamma = 0.0f);
	static std::shared_ptr<EnvironmentalMap> LoadEnvironmentalMap(
		const bool &addResource, const std::string &path, const float &gamma = 0.0f);

	static std::shared_ptr<LightProbe> LoadLightProbe(
		const bool &addResource, const std::string &path, const float &gamma = 0.0f);
	static std::shared_ptr<ReflectionProbe> LoadReflectionProbe(
		const bool &addResource, const std::string &path, const float &gamma = 0.0f);
	static std::shared_ptr<Cubemap> LoadCubemap(
		const bool &addResource,
		const std::vector<std::string> &paths,
		const float &gamma = 0.0f);
	static std::shared_ptr<Material> LoadMaterial(
		const bool &addResource, const std::shared_ptr<OpenGLUtils::GLProgram> &program);
	static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
		const bool &addResource,
		const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
		const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
	static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
		const bool &addResource,
		const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
		const std::shared_ptr<OpenGLUtils::GLShader> &geometry,
		const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
	static void OnGui();
	static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model);
	static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture);
};

template <typename T> std::string ResourceManager::GetTypeName()
{
	auto &resourceManager = GetInstance();
	const auto id = typeid(T).hash_code();
	if (resourceManager.m_resources.find(id) != resourceManager.m_resources.end())
	{
		return resourceManager.m_resources[id].first;
	}
	UNIENGINE_ERROR("Resource type not registered!");
	throw 0;
}

template <typename T> void ResourceManager::RegisterResourceType(const std::string &name)
{
	auto &resourceManager = GetInstance();
	const auto id = typeid(T).hash_code();
	if (resourceManager.m_resources.find(id) == resourceManager.m_resources.end())
	{
		resourceManager.m_resources[id].first = name;
		return;
	}
	UNIENGINE_ERROR("Resource type already registered!");
	throw 0;
}

template <typename T>
std::shared_ptr<T> ResourceManager::CreateResource(const bool &addResource, const std::string &name)
{
	auto &resourceManager = GetInstance();
	const auto id = typeid(T).hash_code();
	if (resourceManager.m_resources.find(id) != resourceManager.m_resources.end())
	{
		auto retVal = std::make_shared<T>();
		dynamic_cast<ResourceBehaviour*>(retVal.get())->m_typeId = id;
		dynamic_cast<ResourceBehaviour *>(retVal.get())->OnCreate();
		if (addResource)
			Push(retVal);
        if (!name.empty())
            retVal->m_name = name;
		return retVal;
	}
	UNIENGINE_ERROR("Resource type not registered!");
	throw 0;
}

template <typename T> void ResourceManager::Push(std::shared_ptr<T> resource)
{
	auto &resourceManager = GetInstance();
	const auto id = dynamic_cast<ResourceBehaviour *>(resource.get())->m_typeId;
	if (id == 0)
	{
		UNIENGINE_ERROR("Resource not created with ResourceManager!");
		return;
	}
	if (resourceManager.m_resources.find(id) != resourceManager.m_resources.end())
	{
		resourceManager.m_resources[id]
			.second[std::dynamic_pointer_cast<ResourceBehaviour>(resource)->GetHashCode()] = resource;
		return;
	}
	UNIENGINE_ERROR("Resource type not registered!");
	throw 0;
}

template <typename T> std::shared_ptr<T> ResourceManager::Get(size_t hashCode)
{
	return std::dynamic_pointer_cast<ResourceBehaviour>(
		GetInstance().m_resources[typeid(T).hash_code()].second[hashCode]);
}

template <typename T> std::shared_ptr<T> ResourceManager::Find(std::string objectName)
{
	for (const auto &i : GetInstance().m_resources[typeid(T).hash_code()].second)
	{
		if (i.second->m_name.compare(objectName) == 0)
			return std::dynamic_pointer_cast<ResourceBehaviour>(i.second);
	}
	return nullptr;
}

template <typename T> void ResourceManager::Remove(size_t hashCode)
{
	GetInstance().m_resources[typeid(T).hash_code()].second.erase(hashCode);
}
} // namespace UniEngine
