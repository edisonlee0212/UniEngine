#pragma once
#include <Entity.hpp>
#include <IAsset.hpp>
#include <ISerializable.hpp>
#include <IPrivateComponent.hpp>
#include "SerializationManager.hpp"
namespace UniEngine
{
class UNIENGINE_API PrivateComponentRef : public ISerializable
{
    friend class Prefab;
    std::optional<std::weak_ptr<IPrivateComponent>> m_value;
    Handle m_entityHandle = Handle(0);
    Handle m_sceneHandle = Handle(0);
    std::string m_privateComponentTypeName;
    bool Update();

  public:
    void Serialize(YAML::Emitter &out) override
    {
        out << YAML::Key << "m_entityHandle" << YAML::Value << m_entityHandle;
        out << YAML::Key << "m_sceneHandle" << YAML::Value << m_sceneHandle;
        out << YAML::Key << "m_privateComponentTypeName" << YAML::Value << m_privateComponentTypeName;
    }
    void Deserialize(const YAML::Node &in) override
    {
        m_entityHandle = Handle(in["m_entityHandle"].as<uint64_t>());
        m_sceneHandle = Handle(in["m_sceneHandle"].as<uint64_t>());
        m_privateComponentTypeName = in["m_privateComponentTypeName"].as<std::string>();
    }

    PrivateComponentRef()
    {
        m_entityHandle = Handle(0);
        m_sceneHandle = Handle(0);
        m_privateComponentTypeName = "";
    }
    template <typename T = IPrivateComponent> PrivateComponentRef(const std::shared_ptr<T> &other)
    {
        Set(other);
    }
    template <typename T = IPrivateComponent> PrivateComponentRef &operator=(const std::shared_ptr<T> &other)
    {
        Set(other);
        return *this;
    }
    template <typename T = IPrivateComponent> PrivateComponentRef &operator=(std::shared_ptr<T> &&other) noexcept
    {
        Set(other);
        return *this;
    }

    bool operator==(const PrivateComponentRef &rhs) const
    {
        return m_entityHandle == rhs.m_entityHandle && m_sceneHandle == rhs.m_sceneHandle;
    }
    bool operator!=(const PrivateComponentRef &rhs) const
    {
        return m_entityHandle != rhs.m_entityHandle || m_sceneHandle != rhs.m_sceneHandle;
    }

    void Relink(const std::unordered_map<Handle, Handle> &map, const Handle &newSceneHandle)
    {
        auto search = map.find(m_entityHandle);
        if (search != map.end())
        {
            m_entityHandle = search->second;
            m_sceneHandle = newSceneHandle;
            m_value.reset();
        }
        else
            Clear();
    };

    template <typename T = IPrivateComponent> [[nodiscard]] std::shared_ptr<T> Get()
    {
        if (Update())
        {
            return std::static_pointer_cast<T>(m_value.value().lock());
        }
        return nullptr;
    }
    template <typename T = IPrivateComponent> void Set(const std::shared_ptr<T> &target)
    {
        if (target)
        {
            auto privateComponent = std::dynamic_pointer_cast<IPrivateComponent>(target);
            m_privateComponentTypeName = privateComponent->GetTypeName();
            m_entityHandle = privateComponent->GetOwner().GetHandle();
            m_sceneHandle = privateComponent->GetOwner().GetSceneHandle();
            m_value = privateComponent;
        }
        else
        {
            Clear();
        }
    }

    template <typename T = IPrivateComponent> void Set(const Entity &target)
    {
        if (!target.IsValid())
        {
            UNIENGINE_WARNING("Entity invalid!");
            return;
        }
        if (target.HasPrivateComponent<T>())
        {
            auto pc = target.GetOrSetPrivateComponent<T>().lock();
            m_privateComponentTypeName = pc->GetTypeName();
            auto entity = std::dynamic_pointer_cast<IPrivateComponent>(pc)->GetOwner();
            m_entityHandle = entity.GetHandle();
            m_sceneHandle = entity.GetSceneHandle();
            m_value = pc;
        }
        else
        {
            UNIENGINE_WARNING("Entity doesn't contain " + SerializationManager::GetSerializableTypeName<T>() + "!");
            return;
        }
    }

    void Clear();

    [[nodiscard]] Handle GetEntityHandle() const
    {
        return m_entityHandle;
    }

    [[nodiscard]] Handle GetSceneHandle() const
    {
        return m_sceneHandle;
    }

    void Save(const std::string &name, YAML::Emitter &out)
    {
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string &name, const YAML::Node &in)
    {
        Deserialize(in[name]);
    }
};
} // namespace UniEngine