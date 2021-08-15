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
    std::string m_name = "";
    std::filesystem::path m_filePath = "";
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
    friend class ClassRegistry;
    std::map<std::string, std::unordered_map<Handle, std::shared_ptr<IAsset>>> m_sharedAssets;
    std::unordered_map<Handle, std::weak_ptr<IAsset>> m_assets;
    std::shared_ptr<AssetRegistry> m_assetRegistry;
    friend class DefaultResources;
    friend class EditorManager;
    friend class IAsset;
    friend class Scene;
    static void RegisterAsset(std::shared_ptr<IAsset> resource);

    template <typename T> static std::shared_ptr<T> CreateAsset(const Handle &handle, const std::string &name);
    static std::shared_ptr<IAsset> CreateAsset(
        const std::string &typeName, const Handle &handle, const std::string &name);
    template <typename T> static void RegisterAssetType(const std::string &name);

  public:
    static void SetResourcePath(const std::filesystem::path &path);
    static std::filesystem::path GetAssetFolderPath();
    static std::filesystem::path GetResourceFolderPath();
    static void ScanAssetFolder();
    template <typename T> static void Share(std::shared_ptr<T> resource);

    template <typename T> static std::shared_ptr<T> CreateAsset(const std::string &name = "");

    static std::shared_ptr<IAsset> Get(const std::string &typeName, const Handle &handle);
    template <typename T> static void RemoveFromShared(const Handle &handle);
    static void RemoveFromShared(const std::string &typeName, const Handle &handle);
#pragma region Loaders
    template <typename T = IAsset> static std::shared_ptr<T> Import(const std::filesystem::path &path);
    template <typename T = IAsset>
    static void Export(const std::filesystem::path &path, const std::shared_ptr<T> &target);
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
template <typename T> void AssetManager::Share(std::shared_ptr<T> resource)
{
    GetInstance().m_sharedAssets[SerializationManager::GetSerializableTypeName<T>()][std::dynamic_pointer_cast<IAsset>(resource)->GetHandle()] = std::dynamic_pointer_cast<IAsset>(resource);
}
template <typename T> std::shared_ptr<T> AssetManager::Import(const std::filesystem::path &path)
{
    auto &assetManager = GetInstance();
    auto ptr = std::static_pointer_cast<IAsset>(CreateAsset<T>(path.filename().string()));
    ptr->m_path = path;
    ptr->Load();
    const auto typeName = ptr->m_typeName;
    assert(!typeName.empty());
    AssetRecord assetRecord;
    assetRecord.m_typeName = ptr->GetTypeName();
    assetRecord.m_filePath = ptr->m_path;
    assetManager.m_assetRegistry->m_assetRecords[ptr->GetHandle()] = assetRecord;
    return std::static_pointer_cast<T>(ptr);
}
template <typename T> void AssetManager::Export(const std::filesystem::path &path, const std::shared_ptr<T> &target)
{
    auto ptr = std::static_pointer_cast<IAsset>(target);
    ptr->Save(path);
}
template <typename T> void AssetManager::RegisterAssetType(const std::string &name)
{
    auto &resourceManager = GetInstance();
    SerializationManager::RegisterSerializableType<T>(name);
    resourceManager.m_sharedAssets[name] = std::unordered_map<Handle, std::shared_ptr<IAsset>>();
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const std::string &name)
{
    return CreateAsset<T>(Handle(), name);
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const Handle &handle, const std::string &name)
{
    return std::dynamic_pointer_cast<T>(CreateAsset(SerializationManager::GetSerializableTypeName<T>(), handle, name));
}

template <typename T> void AssetManager::RemoveFromShared(const Handle &handle)
{
    assert(handle != 0);
    GetInstance().m_sharedAssets[SerializationManager::GetSerializableTypeName<T>()].erase(handle);
}
} // namespace UniEngine
