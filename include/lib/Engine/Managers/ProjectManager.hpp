#pragma once
#include <IAsset.hpp>
#include <ISingleton.hpp>

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
    bool m_needUpdate = false;
    std::unordered_map<Handle, AssetRecord> m_assetRecords;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

class UNIENGINE_API Project : public ISerializable
{
  public:
    std::filesystem::path m_assetRegistryPath;
    AssetRef m_startScene;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

class UNIENGINE_API ProjectManager : public ISingleton<ProjectManager>
{
    friend class AssetManager;
    std::filesystem::path m_projectPath;
    std::shared_ptr<Project> m_currentProject;
    std::optional<std::function<void()>> m_newSceneCustomizer;
  public:
    static void SetScenePostLoadActions(const std::function<void()> &actions);

    std::shared_ptr<AssetRegistry> m_assetRegistry;
    std::string m_currentProjectName = "New Project";
    static void Init();
    static void OnGui();
    static void CreateOrLoadProject(const std::filesystem::path &path);
    static void SaveProject();
    static void RefreshAssetRegistry();
    static void SaveAssetRegistry();
    static void LoadAssetRegistry();
    static std::filesystem::path GetProjectPath();
};
} // namespace UniEngine