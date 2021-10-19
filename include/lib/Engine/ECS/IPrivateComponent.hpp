#pragma once
#include <Entity.hpp>
#include <AssetRef.hpp>
#include <ISerializable.hpp>
namespace UniEngine
{
class UNIENGINE_API IPrivateComponent : public ISerializable
{
    friend class EntityManager;
    friend class EditorManager;
    friend struct PrivateComponentElement;
    friend class SerializationManager;
    friend class Scene;
    friend class Prefab;
    friend class EntityMetadata;
    bool m_enabled = true;
    Entity m_owner = Entity();
    bool m_started = false;
    std::weak_ptr<Scene> m_scene;
  public:
    std::shared_ptr<Scene> GetScene() const;
    [[nodiscard]] Entity GetOwner() const;
    void SetEnabled(const bool &value);
    [[nodiscard]] bool IsEnabled() const;
    bool Started();
    virtual void OnInspect(){};
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

    virtual void CollectAssetRef(std::vector<AssetRef> &list){};
    virtual void Relink(const std::unordered_map<Handle, Handle> &map, const std::shared_ptr<Scene> &scene){};
    virtual void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target) {};
};

struct PrivateComponentElement
{
    size_t m_typeId;
    std::shared_ptr<IPrivateComponent> m_privateComponentData;
    UNIENGINE_API PrivateComponentElement() = default;
    UNIENGINE_API PrivateComponentElement(
        size_t id, const std::shared_ptr<IPrivateComponent> &data, const Entity &owner, const std::shared_ptr<Scene> &scene);
    UNIENGINE_API void ResetOwner(const Entity &newOwner, const std::shared_ptr<Scene> &scene) const;
};

} // namespace UniEngine