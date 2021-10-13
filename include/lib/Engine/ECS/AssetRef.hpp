#pragma once
#include <IHandle.hpp>
#include <ISerializable.hpp>
#include <uniengine_export.h>
#include <IAsset.hpp>
namespace UniEngine
{
class UNIENGINE_API AssetRef : public ISerializable
{
    friend class Prefab;
    friend class Project;
    friend class EditorManager;
    friend class AssetManager;
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
    AssetRef()
    {
        m_assetHandle = Handle(0);
        m_assetTypeName = "";
        m_value = nullptr;
    }
    template <typename T = IAsset> AssetRef(const std::shared_ptr<T> &other)
    {
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
    void Set(const AssetRef &target);
    void Clear();
    [[nodiscard]] Handle GetAssetHandle() const
    {
        return m_assetHandle;
    }

    void Save(const std::string &name, YAML::Emitter &out)
    {
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string &name, const YAML::Node &in)
    {
        if (in[name])
        {
            Deserialize(in[name]);
            Update();
        }
    }
};
} // namespace UniEngine