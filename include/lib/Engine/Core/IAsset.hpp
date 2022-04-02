#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class AssetRef;
class AssetRecord;
class UNIENGINE_API IAsset : public ISerializable
{
  protected:
    friend class DefaultResources;
    friend class Editor;
    friend class EditorLayer;
    friend class AssetRegistry;
    friend class ProjectManager;
    friend class AssetRecord;
    friend class Folder;
    std::weak_ptr<AssetRecord> m_assetRecord;
    std::weak_ptr<IAsset> m_self;
    /**
     * The function that handles serialization. May be invoked by SaveInternal() or ProjectManager.
     * Function is virtual so user can define their own serialization procedure.
     * @param path The file path for saving the asset, may or may not be the local stored path.
     */
    virtual bool SaveInternal(const std::filesystem::path &path);
    /**
     * The function that handles deserialization. May be invoked by Load() or ProjectManager. Function is
     * virtual so user can define their own deserialization procedure.
     * @param path The file path for loading the asset, may or may not be the local stored path.
     */
    virtual bool LoadInternal(const std::filesystem::path &path);
    /**
     * Whether the asset is saved or not.
     */
    bool m_saved = false;
  public:
    [[nodiscard]] bool SetPathAndSave(const std::filesystem::path &projectRelativePath);
    [[nodiscard]] std::filesystem::path GetProjectRelativePath() const;
    [[nodiscard]] std::filesystem::path GetAbsolutePath() const;
    [[nodiscard]] std::string GetTitle() const;
    [[nodiscard]] bool IsTemporary() const;
    [[nodiscard]] std::weak_ptr<AssetRecord> GetAssetRecord() const;
    /**
     * Function will be invoked right after asset creation.
     */
    virtual void OnCreate();

    /**
     * SaveInternal the asset to its file path, nothing happens if the path if empty.
     */
    bool Save();
    /**
     * Load the asset from its file path, nothing happens if the path if empty.
     */
    bool Load();
    /**
     * Export current asset. Will not affect the path member of the asset.
     * @param path The target path of the asset, must be absolute path and outside project folder.
     * @return If the asset is successfully exported.
     */
    bool Export(const std::filesystem::path &path);
    /**
     * Import current asset. Will not affect the path member of the asset.
     * @param path The target path of the asset, must be absolute path and outside project folder.
     * @return If the asset is successfully imported.
     */
    bool Import(const std::filesystem::path &path);

    /**
     * The GUI of the asset when inspected in the editor.
     */
    virtual void OnInspect(){};
    /**
     * During the serialization of the prefab and scene, user should mark all the AssetRef member in the class so they
     * will be serialized and correctly restored during deserialization.
     * @param list The list for collecting the AssetRef of all members. You should push all the AssetRef of the class
     * members to ensure correct behaviour.
     */
    virtual void CollectAssetRef(std::vector<AssetRef> &list){};
    /**
     * Notify asset to be saved later.
     */
    void SetUnsaved();
    [[nodiscard]] bool Saved() const;
    ~IAsset();
};

} // namespace UniEngine
