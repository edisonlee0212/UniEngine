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
    virtual void OnCreate();
    std::string m_name;

    void Save();
    void Load();

    virtual ~IAsset();
    virtual void Save(const std::filesystem::path &path);
    virtual void Load(const std::filesystem::path &path);
    bool m_saved = false;
};
class UNIENGINE_API AssetRef : public ISerializable
{
    friend class Prefab;
    std::shared_ptr<IAsset> m_value;
    Handle m_assetHandle = Handle(0);
    std::string m_assetTypeName;
    bool Update();

  public:
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
            m_assetTypeName = SerializationManager::GetSerializableTypeName<T>();
            m_assetHandle = std::dynamic_pointer_cast<IAsset>(target)->GetHandle();
        }
        else
            m_assetHandle = Handle(0);
        m_value = std::dynamic_pointer_cast<IAsset>(target);
    }
    void Clear();
    [[nodiscard]] Handle GetAssetHandle() const
    {
        return m_assetHandle;
    }
    void Serialize(YAML::Emitter &out) override
    {
        out << YAML::BeginMap;
        out << YAML::Key << "AssetRef" << YAML::Value << m_assetHandle;
        out << YAML::Key << "AssetTypeName" << YAML::Value << m_assetTypeName;
        out << YAML::EndMap;
    }
    void Deserialize(const YAML::Node &in) override
    {
        m_assetHandle = Handle(in["AssetRef"].as<uint64_t>());
        m_assetTypeName = in["AssetTypeName"].as<std::string>();
    }
};
} // namespace UniEngine
