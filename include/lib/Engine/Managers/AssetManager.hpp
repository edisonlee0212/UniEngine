#pragma once
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <EnvironmentalMap.hpp>
#include <IAsset.hpp>
#include <LightProbe.hpp>
#include <OpenGLUtils.hpp>
#include <Prefab.hpp>
#include <ProjectManager.hpp>
#include <ReflectionProbe.hpp>
namespace UniEngine
{
struct UNIENGINE_API AssetCreateHelper
{
    std::string m_typeName;
    Handle m_handle;
    std::string m_name;
};
class UNIENGINE_API AssetManager : public ISingleton<AssetManager>
{
    bool m_enableAssetMenu = false;
    friend class ClassRegistry;
    std::map<std::string, std::unordered_map<Handle, std::shared_ptr<IAsset>>> m_sharedAssets;
    std::unordered_map<Handle, std::weak_ptr<IAsset>> m_assets;

    std::unordered_map<std::string, std::vector<std::string>> m_defaultExtensions;
    std::unordered_map<std::string, std::string> m_typeNames;
    friend class DefaultResources;
    friend class AssetRegistry;
    friend class ProjectManager;
    friend class EditorManager;
    friend class EditorLayer;
    friend class IAsset;
    friend class Scene;
    friend class Prefab;
    static void RegisterAsset(std::shared_ptr<IAsset> resource);
    template <typename T> static void RegisterAssetType(const std::string &name, const std::string &extension);
  public:
    static std::shared_ptr<IAsset> UnsafeCreateAsset(
        const std::string &typeName, const Handle &handle, const std::string &name);
    template <typename T> static std::shared_ptr<T> UnsafeCreateAsset(const Handle &handle, const std::string &name);

    static bool IsAsset(const std::string &typeName);
    template <typename T> static void RegisterExternalAssetTypeExtensions(std::vector<std::string> extensions);

    template <typename T> static std::shared_ptr<T> Import(const std::filesystem::path &path);
    template <typename T> static void Share(std::shared_ptr<T> resource);

    template <typename T> static std::shared_ptr<T> CreateAsset(const std::string &name = "");

    template <typename T> static std::shared_ptr<T> Get(const Handle &handle);

    static std::shared_ptr<IAsset> Get(const std::string &typeName, const Handle &handle);
    static std::shared_ptr<IAsset> Get(const Handle &handle);
    template <typename T> static void RemoveFromShared(const Handle &handle);
    static void RemoveFromShared(const std::string &typeName, const Handle &handle);
#pragma region Loaders
    static std::shared_ptr<Material> LoadMaterial(const std::shared_ptr<OpenGLUtils::GLProgram> &program);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
        const std::shared_ptr<OpenGLUtils::GLShader> &vertex, const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
        const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
        const std::shared_ptr<OpenGLUtils::GLShader> &geometry,
        const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
#pragma endregion
    static void OnInspect();
#pragma region ToEntity
    static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture);
#pragma endregion
    static void Init();
    template <typename T = IAsset> static std::vector<std::string> GetExtension();
    static std::vector<std::string> GetExtension(const std::string &typeName);
};
template <typename T> std::shared_ptr<T> AssetManager::Import(const std::filesystem::path &path)
{
    auto asset = UnsafeCreateAsset<T>(path.filename().string());
    std::dynamic_pointer_cast<IAsset>(asset)->SetPathAndLoad(path);
    return asset;
}
template <typename T> std::shared_ptr<T> Scene::GetOrCreateSystem(float rank)
{
    const auto search = m_indexedSystems.find(typeid(T).hash_code());
    if (search != m_indexedSystems.end())
        return std::dynamic_pointer_cast<T>(search->second);
    auto self = AssetManager::Get<Scene>(m_handle);
    auto ptr = SerializationManager::ProduceSerializable<T>();
    auto system = std::dynamic_pointer_cast<ISystem>(ptr);
    system->m_scene = self;
    system->m_handle = Handle();
    system->m_rank = rank;
    m_systems.insert({rank, system});
    m_indexedSystems[typeid(T).hash_code()] = system;
    m_mappedSystems[system->m_handle] = system;
    system->m_started = false;
    system->OnCreate();
    m_saved = false;
    return ptr;
}
template <typename T> std::vector<std::string> AssetManager::GetExtension()
{
    return GetInstance().m_defaultExtensions[SerializationManager::GetSerializableTypeName<T>()];
}

template <typename T> void AssetManager::Share(std::shared_ptr<T> resource)
{
    auto ptr = std::dynamic_pointer_cast<IAsset>(resource);
    GetInstance().m_sharedAssets[ptr->GetTypeName()][ptr->GetHandle()] = std::dynamic_pointer_cast<IAsset>(resource);
}
template <typename T> std::shared_ptr<T> AssetManager::Get(const Handle &handle)
{
    return std::dynamic_pointer_cast<T>(Get(SerializationManager::GetSerializableTypeName<T>(), handle));
}

template <typename T> void AssetManager::RegisterAssetType(const std::string &name, const std::string &extension)
{
    auto &resourceManager = GetInstance();
    SerializationManager::RegisterSerializableType<T>(name);
    resourceManager.m_sharedAssets[name] = std::unordered_map<Handle, std::shared_ptr<IAsset>>();
    resourceManager.m_defaultExtensions[name].insert(resourceManager.m_defaultExtensions[name].begin(), extension);
    resourceManager.m_typeNames[extension] = name;
}

template <typename T> void AssetManager::RegisterExternalAssetTypeExtensions(std::vector<std::string> extensions)
{
    auto &resourceManager = GetInstance();
    auto name = SerializationManager::GetSerializableTypeName<T>();
    resourceManager.m_defaultExtensions[name].insert(
        resourceManager.m_defaultExtensions[name].end(), extensions.begin(), extensions.end());
    for (const auto &extension : extensions)
    {
        resourceManager.m_typeNames[extension] = name;
    }
}

template <typename T> std::shared_ptr<T> AssetManager::CreateAsset(const std::string &name)
{
    return UnsafeCreateAsset<T>(Handle(), name);
}

template <typename T> std::shared_ptr<T> AssetManager::UnsafeCreateAsset(const Handle &handle, const std::string &name)
{
    return std::dynamic_pointer_cast<T>(
        UnsafeCreateAsset(SerializationManager::GetSerializableTypeName<T>(), handle, name));
}

template <typename T> void AssetManager::RemoveFromShared(const Handle &handle)
{
    assert(handle != 0);
    GetInstance().m_sharedAssets[SerializationManager::GetSerializableTypeName<T>()].erase(handle);
}

} // namespace UniEngine
