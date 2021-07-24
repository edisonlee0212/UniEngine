#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <EnvironmentalMap.hpp>
#include <LightProbe.hpp>
#include <Model.hpp>
#include <ReflectionProbe.hpp>
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
    void AttachToAnimator(std::shared_ptr<Animation> &animation, size_t &index);
    void AttachChild(std::shared_ptr<Bone> &parent, size_t &index);
};
#endif
class UNIENGINE_API AssetManager : public ISingleton<AssetManager>
{
    bool m_enableAssetMenu = true;
    std::map<size_t, std::pair<std::string, std::map<size_t, std::shared_ptr<IAsset>>>> m_assets;
    friend class DefaultResources;
#pragma region Model Loading
    static void AttachAnimator(const Entity &parent, const Entity &animator);
    std::shared_ptr<OpenGLUtils::GLProgram> m_2DToCubemapProgram;
    static std::shared_ptr<Texture2D> CollectTexture(
        const std::string &directory,
        const std::string &path,
        std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures,
        const float &gamma);
#ifdef USE_ASSIMP
    static void ApplyBoneIndices(std::shared_ptr<ModelNode> &node);
    static void ReadAnimations(
        const aiScene *importerScene,
        std::shared_ptr<Animation> &animator,
        std::map<std::string, std::shared_ptr<Bone>> &bonesMap);
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
        const std::shared_ptr<Animation> &animator,
        const float &gamma);
    static std::shared_ptr<Mesh> ReadMesh(aiMesh *importerMesh);
    static std::shared_ptr<SkinnedMesh> ReadSkinnedMesh(
        std::map<std::string, std::shared_ptr<Bone>> &bonesMap, aiMesh *importerMesh);
#else

    static void ProcessNode(
        const std::string &directory,
        std::map<int, std::vector<Vertex>> &meshMaterials,
        const tinyobj::shape_t &shape,
        const tinyobj::attrib_t &attribute);
#endif

    static void AttachChildren(
        EntityArchetype archetype, std::shared_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName);

#pragma endregion
    friend class EditorManager;
  public:
    template <typename T> static void RegisterAssetType(const std::string &name);
    template <typename T>
    static std::shared_ptr<T> CreateResource(const bool &addResource = false, const std::string &name = "");
    template <typename T> static void Push(std::shared_ptr<T> resource);
    template <typename T> static std::shared_ptr<T> Get(size_t hashCode);
    template <typename T> static std::shared_ptr<T> Find(std::string objectName);
    template <typename T> static void Remove(size_t hashCode);
    static void Remove(size_t id, size_t hashCode);
#pragma region Loaders
    static std::shared_ptr<Model> LoadModel(
        const bool &addResource,
        std::string const &path,
        const unsigned &flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes |
                                aiProcess_OptimizeGraph,
        const bool &optimize = false,
        const float &gamma = 0.0f);
    static std::shared_ptr<Texture2D> LoadTexture(
        const bool &addResource, const std::string &path, const float &gamma = 0.0f);
    static std::shared_ptr<Cubemap> LoadCubemap(
        const bool &addResource, const std::string &path, const float &gamma = 0.0f);
    static std::shared_ptr<EnvironmentalMap> LoadEnvironmentalMap(
        const bool &addResource, const std::string &path, const float &gamma = 0.0f);

    static std::shared_ptr<LightProbe> LoadLightProbe(
        const bool &addResource, const std::string &path, const float &gamma = 0.0f);
    static std::shared_ptr<ReflectionProbe> LoadReflectionProbe(
        const bool &addResource, const std::string &path, const float &gamma = 0.0f);
    static std::shared_ptr<Cubemap> LoadCubemap(
        const bool &addResource, const std::vector<std::string> &paths, const float &gamma = 0.0f);
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
#pragma endregion
    static void OnGui();
#pragma region ToEntity
    static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model);
    static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture);
#pragma endregion
    static void Init();
};

template <typename T> void AssetManager::RegisterAssetType(const std::string &name)
{
    auto &resourceManager = GetInstance();
    const auto id = typeid(T).hash_code();
    if (resourceManager.m_assets.find(id) == resourceManager.m_assets.end())
    {
        resourceManager.m_assets[id].first = name;
        SerializableFactory::RegisterSerializable<T>(name);
        return;
    }
    UNIENGINE_ERROR("Resource type already registered!");
    throw 0;
}

template <typename T>
std::shared_ptr<T> AssetManager::CreateResource(const bool &addResource, const std::string &name)
{
    auto &resourceManager = GetInstance();
    const auto id = typeid(T).hash_code();
    if (resourceManager.m_assets.find(id) != resourceManager.m_assets.end())
    {
        auto retVal = std::make_shared<T>();
        dynamic_cast<IAsset *>(retVal.get())->m_typeId = id;
        dynamic_cast<IAsset *>(retVal.get())->OnCreate();
        dynamic_cast<IAsset *>(retVal.get())->m_typeName = SerializableFactory::GetSerializableTypeName<T>();
        if (addResource)
            Push(retVal);
        if (!name.empty())
            retVal->m_name = name;
        return retVal;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> void AssetManager::Push(std::shared_ptr<T> resource)
{
    auto &resourceManager = GetInstance();
    const auto id = dynamic_cast<IAsset *>(resource.get())->m_typeId;
    if (id == 0)
    {
        UNIENGINE_ERROR("Resource not created with AssetManager!");
        return;
    }
    if (resourceManager.m_assets.find(id) != resourceManager.m_assets.end())
    {
        resourceManager.m_assets[id].second[std::dynamic_pointer_cast<IAsset>(resource)->GetHashCode()] =
            resource;
        return;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> std::shared_ptr<T> AssetManager::Get(size_t hashCode)
{
    return std::dynamic_pointer_cast<IAsset>(
        GetInstance().m_assets[typeid(T).hash_code()].second[hashCode]);
}

template <typename T> std::shared_ptr<T> AssetManager::Find(std::string objectName)
{
    for (const auto &i : GetInstance().m_assets[typeid(T).hash_code()].second)
    {
        if (i.second->m_name.compare(objectName) == 0)
            return std::dynamic_pointer_cast<IAsset>(i.second);
    }
    return nullptr;
}

template <typename T> void AssetManager::Remove(size_t hashCode)
{
    GetInstance().m_assets[typeid(T).hash_code()].second.erase(hashCode);
}
} // namespace UniEngine
