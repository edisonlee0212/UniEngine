#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <uniengine_export.h>
#include <SerializationManager.hpp>
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
    std::optional<std::weak_ptr<IAsset>> m_value;
    Handle m_assetHandle = Handle(0);
    std::string m_assetTypeName;
    bool Update();
  public:
    template <typename T = IAsset> [[nodiscard]] std::shared_ptr<T> Get()
    {
        if(Update()){
            return std::static_pointer_cast<T>(m_value.value().lock());
        }
        return nullptr;
    }
    template <typename T = IAsset> void Set(const std::shared_ptr<T> &target)
    {
        if (target)
        {
            m_assetTypeName = SerializationManager::GetSerializableTypeName<T>();
            m_assetHandle = std::dynamic_pointer_cast<IAsset>(target)->GetOwner().GetHandle();
        }
        else
            m_assetHandle = Handle(0);
        m_value = target;
    }
    void Clear();
    [[nodiscard]] Handle GetAssetHandle() const {
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
