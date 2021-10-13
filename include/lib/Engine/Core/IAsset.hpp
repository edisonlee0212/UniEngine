#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class AssetRef;
class UNIENGINE_API IAsset : public ISerializable
{
  protected:
    friend class DefaultResources;
    friend class EditorManager;
    friend class AssetRegistry;
    friend class ProjectManager;
    std::filesystem::path m_projectRelativePath;

    /**
     * The function that handles serialization. May be invoked by SaveInternal() or AssetManager/ProjectManager. Function is virtual so user can define their own serialization procedure.
     * @param path The file path for saving the asset, may or may not be the local stored path.
     */
    virtual bool SaveInternal(const std::filesystem::path &path);
    /**
     * The function that handles deserialization. May be invoked by Load() or AssetManager/ProjectManager. Function is virtual so user can define their own deserialization procedure.
     * @param path The file path for loading the asset, may or may not be the local stored path.
     */
    virtual bool LoadInternal(const std::filesystem::path &path);
    /**
     * Whether the asset is saved or not.
     */
  public:
    /**
     * Function will be invoked right after asset creation.
     */
    virtual void OnCreate();
    /**
     * The name of the asset.
     */
    std::string m_name;
    /**
     * Get the file path of the asset (relative to project).
     * @return The file path of the asset, may be empty.
     */
    [[nodiscard]] std::filesystem::path GetPath(){
        return m_projectRelativePath;
    }
    /**
     * Reset the path of the asset, resetting path will make changes to the asset registry of the project.
     * @param path The new file path of the asset.
     */
    void SetPath(const std::filesystem::path &path);
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
     * @param path The target path of the asset, must be absolute path.
     * @return If the asset is successfully exported.
     */
    bool Export(const std::filesystem::path &path);
    /**
     * The function that handles serialization. May be invoked by SaveInternal() or AssetManager/ProjectManager. Function is virtual so user can define their own serialization procedure.
     * @param path The file path for saving the asset, may or may not be the local stored path.
     */
    bool SetPathAndSave(const std::filesystem::path &path);
    /**
     * The function that handles deserialization. May be invoked by Load() or AssetManager/ProjectManager. Function is virtual so user can define their own deserialization procedure.
     * @param path The file path for loading the asset, may or may not be the local stored path.
     */
    bool SetPathAndLoad(const std::filesystem::path &path);

    /**
     * The destructor, should be overwritten if explict handling is required when the lifecycle of the asset ends.
     */
    ~IAsset() override;
    /**
     * The GUI of the asset when inspected in the editor.
     */
    virtual void OnInspect() {};
    /**
     * During the serialization of the prefab and scene, user should mark all the AssetRef member in the class so they will be serialized and correctly restored during deserialization.
     * @param list The list for collecting the AssetRef of all members. You should push all the AssetRef of the class members to ensure correct behaviour.
     */
    virtual void CollectAssetRef(std::vector<AssetRef> &list){};

    bool m_saved = false;
};

} // namespace UniEngine
