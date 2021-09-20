#pragma once
#include <IAsset.hpp>
#include <ISingleton.hpp>

namespace UniEngine
{

struct UNIENGINE_API FileRecord
{
    std::string m_name = "";
    std::filesystem::path m_relativeFilePath = "";
    std::string m_typeName = "";
    void Serialize(YAML::Emitter &out) const;
    void Deserialize(const YAML::Node &in);
};

struct UNIENGINE_API FolderMetadata{
    std::unordered_map<Handle, FileRecord> m_fileRecords;
    std::map<std::string, Handle> m_fileMap;
    void Save(const std::filesystem::path &path);
    void Load(const std::filesystem::path &path);
};

class UNIENGINE_API AssetRegistry : public ISerializable
{
  public:
    bool m_needUpdate = false;
    std::unordered_map<Handle, FileRecord> m_assetRecords;
    std::unordered_map<std::string, Handle> m_fileMap;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

struct UNIENGINE_API Folder {
    std::string m_name;
    FolderMetadata m_folderMetadata;
    std::map<std::string, std::shared_ptr<Folder>> m_children;
    std::shared_ptr<Folder> m_parent;
};

class UNIENGINE_API Project : public ISerializable
{
  public:
    std::shared_ptr<Folder> m_projectFolder;
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
    static void ScanFolderHelper(const std::filesystem::path& folderPath, std::shared_ptr<Folder>& folder);
    static void FolderMetadataUpdater(const std::filesystem::path& folderPath, std::shared_ptr<Folder>& folder);
    std::shared_ptr<Folder> m_currentFocusedFolder;


  public:
    static bool IsInProjectFolder(const std::filesystem::path &target);
    static std::filesystem::path GetRelativePath(const std::filesystem::path &target);
    static void SetScenePostLoadActions(const std::function<void()> &actions);

    std::shared_ptr<AssetRegistry> m_assetRegistry;
    std::string m_currentProjectName = "New Project";
    static void Init(const std::filesystem::path& projectPath);
    static void OnInspect();
    static void CreateOrLoadProject(const std::filesystem::path &path);
    static void SaveProject();
    static void ScanProjectFolder();
    static void SaveAssetRegistry();
    static void LoadAssetRegistry();
    static std::filesystem::path GetProjectPath();
};
} // namespace UniEngine