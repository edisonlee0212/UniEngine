#pragma once
#include <IAsset.hpp>
#include <ISingleton.hpp>

namespace UniEngine
{
class Folder;
class UNIENGINE_API AssetRecord
{
    friend class Folder;
    std::string m_assetFileName;
    std::string m_assetExtension;
    std::string m_assetTypeName;
    Handle m_assetHandle = 0;
    std::weak_ptr<IAsset> m_asset;
    std::weak_ptr<Folder> m_folder;

  public:
    [[nodiscard]] Handle GetAssetHandle() const;
    [[nodiscard]] std::shared_ptr<IAsset> GetAsset() const;
    [[nodiscard]] std::string GetAssetTypeName() const;
    [[nodiscard]] std::string GetAssetFileName() const;
    [[nodiscard]] std::string GetAssetExtension() const;
    void DeleteMetadata() const;

    [[nodiscard]] std::filesystem::path GetProjectRelativePath() const;
    [[nodiscard]] std::filesystem::path GetAbsolutePath() const;
    void SetAssetFileName(const std::string &newName);
    void SetAssetExtension(const std::string &newExtension);

    void Save() const;
    void Load(const std::filesystem::path& path);
};

class UNIENGINE_API Folder
{
    std::string m_name;
    std::unordered_map<Handle, std::shared_ptr<AssetRecord>> m_assetRecords;
    std::map<Handle, std::shared_ptr<Folder>> m_children;
    std::weak_ptr<Folder> m_parent;
    Handle m_handle = 0;

    void Refresh(const std::filesystem::path &parentAbsolutePath);
  public:
    void DeleteMetadata() const;
    [[nodiscard]] Handle GetHandle() const;
    [[nodiscard]] std::filesystem::path GetProjectRelativePath() const;
    [[nodiscard]] std::filesystem::path GetAbsolutePath() const;
    [[nodiscard]] std::string GetName() const;

    void Rename(const std::string &newName);

    void MoveChild(const Handle &childHandle, const std::shared_ptr<Folder> &dest);
    void DeleteChild(const Handle &childHandle);
    [[nodiscard]] std::weak_ptr<Folder> GetChild(const Handle &childHandle);
    [[nodiscard]] std::weak_ptr<Folder> GetOrCreateChild(const std::string &folderName);

    void MoveAssetRecord(const Handle &assetHandle, const std::shared_ptr<Folder> &dest);
    void DeleteAssetRecord(const Handle &assetHandle);
    [[nodiscard]] std::weak_ptr<AssetRecord> GetOrCreateAssetRecord(
        const std::string &fileName, const std::string &extension);
    [[nodiscard]] std::weak_ptr<AssetRecord> GetAssetRecord(const Handle &assetHandle);

    void Save() const;
    void Load(const std::filesystem::path& path);
};

class UNIENGINE_API ProjectManager
{
    friend class AssetManager;
    friend class Editor;
    friend class EditorLayer;
    friend class AssetRecord;
    friend class Folder;
    std::shared_ptr<Folder> m_projectFolder;
    std::filesystem::path m_projectPath;
    std::optional<std::function<void()>> m_newSceneCustomizer;
    std::weak_ptr<Folder> m_currentFocusedFolder;

    std::unordered_map<Handle, std::weak_ptr<AssetRecord>> m_assetRegistry;

  public:
    [[nodiscard]] std::weak_ptr<Folder> GetOrCreateFolder(const std::filesystem::path &projectRelativePath);

    void OnInspect();
};
} // namespace UniEngine