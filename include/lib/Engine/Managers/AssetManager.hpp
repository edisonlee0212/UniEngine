#pragma once
#include <OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <EnvironmentalMap.hpp>
#include <LightProbe.hpp>
#include <Model.hpp>
#include <ReflectionProbe.hpp>
#include <IAsset.hpp>

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

struct UNIENGINE_API AssetRecord
{
    std::string m_filePath = "";
    bool m_external = false;
    std::string m_typeName = "";
};

class UNIENGINE_API AssetRegistry : public ISerializable
{
  public:
    size_t m_version = 0;
    std::unordered_map<Handle, AssetRecord> m_assetRecords;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

class UNIENGINE_API AssetManager : public ISingleton<AssetManager>
{
    std::filesystem::path m_resourceRootPath;
    bool m_enableAssetMenu = true;
    std::map<std::string, std::unordered_map<Handle, std::shared_ptr<IAsset>>> m_sharedAssets;
    std::shared_ptr<AssetRegistry> m_assetRegistry;
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
    static void SetResourcePath(const std::filesystem::path &path);
    static std::filesystem::path GetAssetFolderPath();
    static std::filesystem::path GetResourceFolderPath();
    static void ScanAssetFolder();


    template <typename T> static void RegisterAssetType(const std::string &name);
    template <typename T> static std::shared_ptr<T> CreateAsset(const std::string &name = "");
    template <typename T> static void Share(std::shared_ptr<T> resource);
    template <typename T> static std::shared_ptr<T> Get(const Handle &handle);
    template <typename T> static void RemoveFromShared(const Handle &handle);
    static void RemoveFromShared(const std::string &typeName, const Handle &handle);
#pragma region Loaders
    template <typename T>
    static std::shared_ptr<T> Load(const std::filesystem::path& path);

    static std::shared_ptr<Model> LoadModel(
        std::filesystem::path const &path,
        const unsigned &flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals,
        const bool &optimize = false,
        const float &gamma = 0.0f);
    static std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path &path, const float &gamma = 0.0f);
    static std::shared_ptr<Cubemap> LoadCubemap(const std::filesystem::path &path, const float &gamma = 0.0f);
    static std::shared_ptr<EnvironmentalMap> LoadEnvironmentalMap(const std::filesystem::path &path, const float &gamma = 0.0f);

    static std::shared_ptr<LightProbe> LoadLightProbe(const std::filesystem::path &path, const float &gamma = 0.0f);
    static std::shared_ptr<ReflectionProbe> LoadReflectionProbe(const std::filesystem::path &path, const float &gamma = 0.0f);
    static std::shared_ptr<Material> LoadMaterial(const std::shared_ptr<OpenGLUtils::GLProgram> &program);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
        const std::shared_ptr<OpenGLUtils::GLShader> &vertex, const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
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


template <typename T> std::shared_ptr<T> AssetManager::Load(const std::filesystem::path& path)
{
    if(typeid(T).hash_code() == typeid(Model).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadModel(path));
    }
    if(typeid(T).hash_code() == typeid(Texture2D).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadTexture(path));
    }
    if(typeid(T).hash_code() == typeid(Cubemap).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadCubemap(path));
    }
    if(typeid(T).hash_code() == typeid(EnvironmentalMap).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadEnvironmentalMap(path));
    }
    if(typeid(T).hash_code() == typeid(LightProbe).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadLightProbe(path));
    }
    if(typeid(T).hash_code() == typeid(ReflectionProbe).hash_code()){
        return std::dynamic_pointer_cast<T>(LoadReflectionProbe(path));
    }
}

template <typename T> void AssetManager::RegisterAssetType(const std::string &name)
{
    auto &resourceManager = GetInstance();
    SerializationManager::RegisterSerializableType<T>(name);
    resourceManager.m_sharedAssets[name] = std::unordered_map<Handle, std::shared_ptr<IAsset>>();
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const std::string &name)
{
    auto &resourceManager = GetInstance();
    if (resourceManager.m_sharedAssets.find(SerializationManager::GetSerializableTypeName<T>()) !=
    resourceManager.m_sharedAssets.end())
    {
        auto retVal = std::make_shared<T>();
        dynamic_cast<IAsset *>(retVal.get())->OnCreate();
        dynamic_cast<IAsset *>(retVal.get())->m_typeName = SerializationManager::GetSerializableTypeName<T>();
        dynamic_cast<IAsset *>(retVal.get())->m_handle = Handle();
        if (!name.empty())
            retVal->m_name = name;
        return retVal;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> void AssetManager::Share(std::shared_ptr<T> resource)
{
    auto &resourceManager = GetInstance();
    const auto typeName = dynamic_cast<IAsset *>(resource.get())->m_typeName;
    if (typeName.empty())
    {
        UNIENGINE_ERROR("Resource not created with AssetManager!");
        return;
    }
    if (resourceManager.m_sharedAssets.find(typeName) != resourceManager.m_sharedAssets.end())
    {
        resourceManager.m_sharedAssets[typeName][std::dynamic_pointer_cast<IAsset>(resource)->GetHandle()] = resource;
        return;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> std::shared_ptr<T> AssetManager::Get(const Handle &handle)
{
    auto &assetManager = GetInstance();
    auto typeName = SerializationManager::GetSerializableTypeName<T>();
    auto typeSearch = assetManager.m_sharedAssets.find(typeName);
    if (typeSearch != assetManager.m_sharedAssets.end())
    {
        auto umap = typeSearch.second;
        auto search = umap.find(handle);
        if(search != umap.end())
        {
            return std::dynamic_pointer_cast<IAsset>(search.second);
        }

        auto search2 = assetManager.m_assetRegistry->m_assetRecords.find(handle);
        if(search2 != assetManager.m_assetRegistry->m_assetRecords.end()){
            if(search2->second.m_external){

            }else
            {
                auto retVal = CreateAsset<T>();
                //SerializationManager::Deserialize(search2->second.m_filePath, retVal);
                return retVal;
            }
        }
        UNIENGINE_ERROR("Asset not registered!");
        return nullptr;
    }else{
        UNIENGINE_ERROR("Type not registered!");
        return nullptr;
    }
}

template <typename T> void AssetManager::RemoveFromShared(const Handle &handle)
{
    if(handle < DefaultResources::GetInstance().m_currentHandle)
    {
        UNIENGINE_WARNING("Not allowed to remove internal assets!");
        return;
    }
    GetInstance().m_sharedAssets[SerializationManager::GetSerializableTypeName<T>()].erase(handle);
}
} // namespace UniEngine
