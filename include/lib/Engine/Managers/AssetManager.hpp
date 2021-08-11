#pragma once
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <EnvironmentalMap.hpp>
#include <IAsset.hpp>
#include <LightProbe.hpp>
#include <OpenGLUtils.hpp>
#include <Prefab.hpp>
#include <ReflectionProbe.hpp>
namespace UniEngine
{
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
    std::map<std::string, std::unordered_map<Handle, std::weak_ptr<IAsset>>> m_assets;
    std::shared_ptr<AssetRegistry> m_assetRegistry;
    friend class DefaultResources;
    friend class EditorManager;
    friend class IAsset;
    template <typename T> static void RegisterAsset(std::shared_ptr<T> resource);

    template <typename T> static std::shared_ptr<T> CreateAsset(const Handle &handle, const std::string &name = "");

  public:
    static void SetResourcePath(const std::filesystem::path &path);
    static std::filesystem::path GetAssetFolderPath();
    static std::filesystem::path GetResourceFolderPath();
    static void ScanAssetFolder();

    template <typename T> static void RegisterAssetType(const std::string &name);
    template <typename T> static std::shared_ptr<T> CreateAsset(const std::string &name = "");

    template <typename T> static std::shared_ptr<T> Get(const Handle &handle);
    template <typename T> static void RemoveFromShared(const Handle &handle);
    static void RemoveFromShared(const std::string &typeName, const Handle &handle);
#pragma region Loaders
    template <typename T> static std::shared_ptr<T> Load(const std::filesystem::path &path);

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
    static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture);
#pragma endregion
    static void Init();
};

template <typename T> std::shared_ptr<T> AssetManager::Load(const std::filesystem::path &path)
{
    auto ptr = std::static_pointer_cast<IAsset>(CreateAsset<T>(path.filename().string()));
    ptr->m_path = path;
    ptr->Load();
    return std::static_pointer_cast<T>(ptr);
}

template <typename T> void AssetManager::RegisterAssetType(const std::string &name)
{
    auto &resourceManager = GetInstance();
    SerializationManager::RegisterSerializableType<T>(name);
    resourceManager.m_assets[name] = std::unordered_map<Handle, std::weak_ptr<IAsset>>();
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const std::string &name)
{
    auto retVal = CreateAsset<T>(Handle(), name);
    RegisterAsset<T>(retVal);
    return retVal;
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const Handle &handle, const std::string &name)
{
    auto &resourceManager = GetInstance();
    if (resourceManager.m_assets.find(SerializationManager::GetSerializableTypeName<T>()) !=
    resourceManager.m_assets.end())
    {
        auto retVal = std::make_shared<T>();
        dynamic_cast<IAsset *>(retVal.get())->OnCreate();
        dynamic_cast<IAsset *>(retVal.get())->m_typeName = SerializationManager::GetSerializableTypeName<T>();
        dynamic_cast<IAsset *>(retVal.get())->m_handle = handle;
        if (!name.empty())
            retVal->m_name = name;
        return retVal;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> void AssetManager::RegisterAsset(std::shared_ptr<T> resource)
{
    auto &resourceManager = GetInstance();
    const auto typeName = dynamic_cast<IAsset *>(resource.get())->m_typeName;
    if (typeName.empty())
    {
        UNIENGINE_ERROR("Resource not created with AssetManager!");
        return;
    }
    if (resourceManager.m_assets.find(typeName) != resourceManager.m_assets.end())
    {
        resourceManager.m_assets[typeName][std::dynamic_pointer_cast<IAsset>(resource)->GetHandle()] = resource;
        return;
    }
    UNIENGINE_ERROR("Resource type not registered!");
    throw 0;
}

template <typename T> std::shared_ptr<T> AssetManager::Get(const Handle &handle)
{
    auto &assetManager = GetInstance();
    auto typeName = SerializationManager::GetSerializableTypeName<T>();
    auto typeSearch = assetManager.m_assets.find(typeName);
    if (typeSearch != assetManager.m_assets.end())
    {
        auto umap = typeSearch.second;
        auto search = umap.find(handle);
        if (search != umap.end())
        {
            return std::dynamic_pointer_cast<IAsset>(search.second);
        }

        auto search2 = assetManager.m_assetRegistry->m_assetRecords.find(handle);
        if (search2 != assetManager.m_assetRegistry->m_assetRecords.end())
        {
            if (search2->second.m_external)
            {
            }
            else
            {
                auto retVal = CreateAsset<T>();
                // SerializationManager::Deserialize(search2->second.m_filePath, retVal);
                return retVal;
            }
        }
        UNIENGINE_ERROR("Asset not registered!");
        return nullptr;
    }
    else
    {
        UNIENGINE_ERROR("Type not registered!");
        return nullptr;
    }
}

template <typename T> void AssetManager::RemoveFromShared(const Handle &handle)
{
    assert(handle != 0);
    GetInstance().m_assets[SerializationManager::GetSerializableTypeName<T>()].erase(handle);
}
} // namespace UniEngine
