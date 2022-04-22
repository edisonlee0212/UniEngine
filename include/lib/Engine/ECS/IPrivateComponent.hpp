#pragma once
#include <AssetRef.hpp>
#include <Entity.hpp>
#include <ISerializable.hpp>
namespace UniEngine
{
class UNIENGINE_API IPrivateComponent : public ISerializable
{
    friend class Entities;
    friend class Editor;
    friend class EditorLayer;
    friend struct PrivateComponentElement;
    friend struct PrivateComponentStorage;
    friend class Serialization;
    friend class Scene;
    friend class Prefab;
    friend class EntityMetadata;
    bool m_enabled = true;
    Entity m_owner = Entity();
    bool m_started = false;
    size_t m_version = 0;
    std::weak_ptr<Scene> m_scene;

  public:
    [[nodiscard]] std::shared_ptr<Scene> GetScene() const;
    [[nodiscard]] Entity GetOwner() const;
    [[nodiscard]] size_t GetVersion() const;
    void SetEnabled(const bool &value);
    [[nodiscard]] bool IsEnabled() const;
    bool Started();
    virtual void OnInspect(){};
    virtual void FixedUpdate(){};
    virtual void Update(){};
    virtual void LateUpdate(){};

    virtual void OnCreate(){};
    virtual void Start(){};
    virtual void OnEnable(){};
    virtual void OnDisable(){};
    virtual void OnEntityEnable(){};
    virtual void OnEntityDisable(){};
    virtual void OnDestroy(){};

    virtual void CollectAssetRef(std::vector<AssetRef> &list){};
    virtual void Relink(const std::unordered_map<Handle, Handle> &map, const std::shared_ptr<Scene> &scene){};
    virtual void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target){};
};
struct UNIENGINE_API PrivateComponentElement
{
    size_t m_typeId;
    std::shared_ptr<IPrivateComponent> m_privateComponentData;
    PrivateComponentElement() = default;
    PrivateComponentElement(
        size_t id,
        const std::shared_ptr<IPrivateComponent> &data,
        const Entity &owner,
        const std::shared_ptr<Scene> &scene);
    void ResetOwner(const Entity &newOwner, const std::shared_ptr<Scene> &scene) const;
};

} // namespace UniEngine