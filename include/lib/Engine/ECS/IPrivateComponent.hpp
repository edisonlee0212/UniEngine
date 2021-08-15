#pragma once
#include <Entity.hpp>
#include <IAsset.hpp>
#include <ISerializable.hpp>
#include <SerializationManager.hpp>
namespace UniEngine
{
class UNIENGINE_API IPrivateComponent : public ISerializable
{
    friend class EntityManager;
    friend class EditorManager;
    friend struct PrivateComponentElement;
    friend class Scene;
    friend class Prefab;
    bool m_enabled = true;
    Entity m_owner = Entity();
    bool m_started = false;

  public:
    [[nodiscard]] Entity GetOwner() const;
    void SetEnabled(const bool &value);
    [[nodiscard]] bool IsEnabled() const;

    virtual void OnGui(){};
    virtual void FixedUpdate(){};
    virtual void PreUpdate(){};
    virtual void Update(){};
    virtual void LateUpdate(){};

    virtual void OnCreate(){};
    virtual void Start(){};
    virtual void OnEnable(){};
    virtual void OnDisable(){};
    virtual void OnEntityEnable(){};
    virtual void OnEntityDisable(){};
    virtual void OnDestroy(){};

    virtual void CollectAssetRef(std::unordered_map<Handle, AssetRef> &map){};
    virtual void Relink(const std::unordered_map<Handle, Handle> &map){};
    virtual void Clone(const std::shared_ptr<IPrivateComponent> &target) = 0;
};

struct PrivateComponentElement
{
    size_t m_typeId;
    std::shared_ptr<IPrivateComponent> m_privateComponentData;
    UNIENGINE_API PrivateComponentElement(
        size_t id, const std::shared_ptr<IPrivateComponent> &data, const Entity &owner);
    UNIENGINE_API void ResetOwner(const Entity &newOwner) const;
};

class UNIENGINE_API PrivateComponentRef : public ISerializable
{
    friend class Prefab;
    std::optional<std::weak_ptr<IPrivateComponent>> m_value;
    Handle m_entityHandle = Handle(0);
    std::string m_privateComponentTypeName;
    bool Update();

  protected:
    void Serialize(YAML::Emitter &out) override
    {
        out << YAML::Key << "m_entityHandle" << YAML::Value << m_entityHandle;
        out << YAML::Key << "m_privateComponentTypeName" << YAML::Value << m_privateComponentTypeName;
    }
    void Deserialize(const YAML::Node &in) override
    {
        m_entityHandle = Handle(in["m_entityHandle"].as<uint64_t>());
        m_privateComponentTypeName = in["m_privateComponentTypeName"].as<std::string>();
    }
  public:
    PrivateComponentRef()
    {
        m_entityHandle = Handle(0);
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
        return m_entityHandle == rhs.m_entityHandle;
    }
    bool operator!=(const PrivateComponentRef &rhs) const
    {
        return m_entityHandle != rhs.m_entityHandle;
    }

    void Relink(const std::unordered_map<Handle, Handle> &map)
    {
        auto search = map.find(m_entityHandle);
        if (search != map.end())
            m_entityHandle = search->second;
        else
            m_entityHandle = Handle(0);
        m_value.reset();
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
            m_privateComponentTypeName = SerializationManager::GetSerializableTypeName<T>();
            m_entityHandle = std::dynamic_pointer_cast<IPrivateComponent>(target)->GetOwner().GetHandle();
        }
        else
            m_entityHandle = Handle(0);
        m_value = target;
    }
    void Clear();

    [[nodiscard]] Handle GetEntityHandle() const
    {
        return m_entityHandle;
    }



    void Save(const std::string& name, YAML::Emitter &out){
        out << YAML::Key << name << YAML::Value << YAML::BeginMap;
        Serialize(out);
        out << YAML::EndMap;
    }
    void Load(const std::string& name, const YAML::Node &in){
        Deserialize(in[name]);
    }
};
} // namespace UniEngine