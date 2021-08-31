#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <SerializationManager.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class Texture2D;
class UNIENGINE_API IAsset : public ISerializable
{
  protected:
    friend class EditorManager;
    friend class AssetManager;
    friend class DefaultResources;

    std::shared_ptr<Texture2D> m_icon;
    std::filesystem::path m_path;
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
     * Get the file path of the asset.
     * @return The file path of the asset, may be empty.
     */
    [[nodiscard]] std::filesystem::path GetPath(){
        return m_path;
    }
    /**
     * Reset the path of the asset, resetting path will make changes to the asset registry of the project.
     * @param path The new file path of the asset.
     */
    void SetPath(const std::filesystem::path &path);
    /**
     * Save the asset to its file path, nothing happens if the path if empty.
     */
    void Save();
    /**
     * Load the asset from its file path, nothing happens if the path if empty.
     */
    void Load();
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
    /**
     * The function that handles serialization. May be invoked by Save() or AssetManager/ProjectManager. Function is virtual so user can define their own serialization procedure.
     * @param path The file path for saving the asset, may or may not be the local stored path.
     */
    virtual void Save(const std::filesystem::path &path);
    /**
     * The function that handles deserialization. May be invoked by Load() or AssetManager/ProjectManager. Function is virtual so user can define their own deserialization procedure.
     * @param path The file path for loading the asset, may or may not be the local stored path.
     */
    virtual void Load(const std::filesystem::path &path);
    /**
     * Whether the asset is saved or not.
     */
    bool m_saved = false;
};
class UNIENGINE_API AssetRef : public ISerializable
{
    friend class Prefab;
    friend class Project;
    std::shared_ptr<IAsset> m_value;
    Handle m_assetHandle = Handle(0);
    std::string m_assetTypeName;
    bool Update();
  public:
    void Serialize(YAML::Emitter &out) override
    {
      out << YAML::Key << "m_assetHandle" << YAML::Value << m_assetHandle;
      out << YAML::Key << "m_assetTypeName" << YAML::Value << m_assetTypeName;
    }
    void Deserialize(const YAML::Node &in) override
    {
      m_assetHandle = Handle(in["m_assetHandle"].as<uint64_t>());
      m_assetTypeName = in["m_assetTypeName"].as<std::string>();
      Update();
    }
    AssetRef(){
        m_assetHandle = Handle(0);
        m_assetTypeName = "";
        m_value = nullptr;
    }
    template <typename T = IAsset>
    AssetRef(const std::shared_ptr<T> &other){
        Set(other);
    }
    template <typename T = IAsset> AssetRef &operator=(const std::shared_ptr<T> &other)
    {
        Set(other);
        return *this;
    }
    template <typename T = IAsset> AssetRef &operator=(std::shared_ptr<T> &&other) noexcept
    {
        Set(other);
        return *this;
    }
    bool operator==(const AssetRef &rhs) const
    {
        return m_assetHandle == rhs.m_assetHandle;
    }
    bool operator!=(const AssetRef &rhs) const
    {
        return m_assetHandle != rhs.m_assetHandle;
    }

    template <typename T = IAsset> [[nodiscard]] std::shared_ptr<T> Get()
    {
        if (Update())
        {
            return std::static_pointer_cast<T>(m_value);
        }
        return nullptr;
    }
    template <typename T = IAsset> void Set(std::shared_ptr<T> target)
    {
        if (target)
        {
            auto asset = std::dynamic_pointer_cast<IAsset>(target);
            m_assetTypeName = asset->GetTypeName();
            m_assetHandle = asset->GetHandle();
            m_value = asset;
        }
        else
        {
            m_assetHandle = Handle(0);
            m_value.reset();
        }
    }
    void Clear();
    [[nodiscard]] Handle GetAssetHandle() const
    {
        return m_assetHandle;
    }

    void Save(const std::string& name, YAML::Emitter &out){
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string& name, const YAML::Node &in){
        if(in[name])
        {
            Deserialize(in[name]);
            Update();
        }
    }
};
} // namespace UniEngine
