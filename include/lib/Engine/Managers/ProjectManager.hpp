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

class UNIENGINE_API AssetRegistry
{
    std::unordered_map<Handle, FileRecord> m_assetRecords;
    std::unordered_map<std::string, Handle> m_fileMap;
  public:
    void ResetFilePath(Handle handle, const std::filesystem::path &newFilePath);
    void AddOrResetFile(Handle handle, const FileRecord &newFileRecord);
    void RemoveFile(Handle handle);
    bool Find(Handle handle, FileRecord& target);
    bool Find(const std::filesystem::path &newRelativePath, Handle& handle);
    bool Find(const std::filesystem::path &newRelativePath);
    void Clear();
};

struct UNIENGINE_API Folder {
    std::filesystem::path m_relativePath;
    std::string m_name;
    FolderMetadata m_folderMetadata;
    std::map<std::string, std::shared_ptr<Folder>> m_children;
    std::shared_ptr<Folder> m_parent;
    void Rename(const std::string& newName);
};

class UNIENGINE_API Project : public ISerializable
{
  public:
    std::shared_ptr<Folder> m_projectFolder;
    std::filesystem::path m_startScenePath;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

};

class UNIENGINE_API ProjectManager : public ISingleton<ProjectManager>
{
    friend class AssetManager;
    friend class EditorManager;
    std::filesystem::path m_projectPath;
    std::shared_ptr<Project> m_currentProject;
    std::optional<std::function<void()>> m_newSceneCustomizer;
    std::shared_ptr<Folder> m_currentFocusedFolder;

    static void GenerateNewDefaultScene();
    static std::filesystem::path GenerateNewPath(const std::string &filestem, const std::string &extension);
    static void ScanFolderHelper(const std::filesystem::path& folderPath, const std::shared_ptr<Folder>& folder, bool updateMetaData = true);


    static void FindFolderHelper(const std::filesystem::path& folderPath, const std::shared_ptr<Folder>& walker, std::shared_ptr<Folder>& result);
  public:
    /**
     * Find a specific folder
     * @param folderPath The path relative to project.
     * @return The folder target. Null if not found.
     */
    static std::shared_ptr<Folder> FindFolder(const std::filesystem::path& folderPath);

    static void UpdateFolderMetadata(const std::shared_ptr<Folder>& folder);

    static bool IsInProjectFolder(const std::filesystem::path &target);
    static std::filesystem::path GetRelativePath(const std::filesystem::path &target);
    static void SetScenePostLoadActions(const std::function<void()> &actions);

    AssetRegistry m_assetRegistry;
    std::string m_currentProjectName = "New Project";
    static void Init(const std::filesystem::path& projectPath);
    static void OnInspect();
    static void CreateOrLoadProject(const std::filesystem::path &path);
    static void SaveProject();
    static void ScanProjectFolder(bool updateMetadata = true);
    static std::filesystem::path GetProjectPath();

};
} // namespace UniEngine