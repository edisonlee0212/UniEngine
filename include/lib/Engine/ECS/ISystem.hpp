#pragma once
#include <ISerializable.hpp>
#include <AssetRef.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class ThreadPool;
class Scene;
class UNIENGINE_API ISystem : public ISerializable
{
    friend class Scene;
    friend class Entities;
    friend class Serialization;
    bool m_enabled;
    float m_rank = 0.0f;
    bool m_started = false;
    std::weak_ptr<Scene> m_scene;
  protected:
    virtual void OnEnable(){};
    virtual void OnDisable(){};
  public:
    [[nodiscard]] std::shared_ptr<Scene> GetScene() const;
    [[nodiscard]] float GetRank();
    ISystem();
    void Enable();
    void Disable();
    [[nodiscard]] bool Enabled() const;
    virtual void OnCreate(){};
    virtual void Start(){};
    virtual void OnDestroy(){};
    virtual void Update(){};
    virtual void FixedUpdate(){};
    virtual void LateUpdate(){};
    // Will only exec when editor is enabled, and no matter application is running or not.
    virtual void OnInspect(){};
    virtual void CollectAssetRef(std::vector<AssetRef> &list){};

    virtual void PostCloneAction(const std::shared_ptr<ISystem>& target){};
};

class UNIENGINE_API SystemRef : public ISerializable
{
    friend class Prefab;
    std::optional<std::weak_ptr<ISystem>> m_value;
    Handle m_systemHandle = Handle(0);
    std::string m_systemTypeName;
    bool Update();

  protected:
    void Serialize(YAML::Emitter &out) override
    {
        out << YAML::Key << "m_systemHandle" << YAML::Value << m_systemHandle;
        out << YAML::Key << "m_systemTypeName" << YAML::Value << m_systemTypeName;
    }
    void Deserialize(const YAML::Node &in) override
    {
        m_systemHandle = Handle(in["m_systemHandle"].as<uint64_t>());
        m_systemTypeName = in["m_systemTypeName"].as<std::string>();
        Update();
    }

  public:
    SystemRef()
    {
        m_systemHandle = Handle(0);
        m_systemTypeName = "";
    }
    template <typename T = ISystem> SystemRef(const std::shared_ptr<T> &other)
    {
        Set(other);
    }
    template <typename T = ISystem> SystemRef &operator=(const std::shared_ptr<T> &other)
    {
        Set(other);
        return *this;
    }
    template <typename T = ISystem> SystemRef &operator=(std::shared_ptr<T> &&other) noexcept
    {
        Set(other);
        return *this;
    }

    bool operator==(const SystemRef &rhs) const
    {
        return m_systemHandle == rhs.m_systemHandle;
    }
    bool operator!=(const SystemRef &rhs) const
    {
        return m_systemHandle != rhs.m_systemHandle;
    }

    void Relink(const std::unordered_map<Handle, Handle> &map)
    {
        auto search = map.find(m_systemHandle);
        if (search != map.end())
            m_systemHandle = search->second;
        else
            m_systemHandle = Handle(0);
        m_value.reset();
    };

    template <typename T = ISystem> [[nodiscard]] std::shared_ptr<T> Get()
    {
        if (Update())
        {
            return std::static_pointer_cast<T>(m_value.value().lock());
        }
        return nullptr;
    }
    template <typename T = ISystem> void Set(const std::shared_ptr<T> &target)
    {
        if (target)
        {
            auto system = std::dynamic_pointer_cast<ISystem>(target);
            m_systemTypeName = system->GetTypeName();
            m_systemHandle = system->GetHandle();
            m_value = system;
        }
        else
        {
            m_systemHandle = Handle(0);
            m_value.reset();
        }
    }

    void Clear()
    {
        m_value.reset();
        m_systemHandle = Handle(0);
    }

    [[nodiscard]] Handle GetEntityHandle() const
    {
        return m_systemHandle;
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