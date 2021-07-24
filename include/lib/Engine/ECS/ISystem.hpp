#pragma once
#include <uniengine_export.h>
#include <ISerializable.hpp>
namespace UniEngine
{
class ThreadPool;
class Scene;
class UNIENGINE_API ISystem : public ISerializable
{
    friend class Scene;
    friend class EntityManager;

    std::shared_ptr<Scene> m_scene;
    bool m_enabled;
  protected:
    virtual void OnStartRunning();
    virtual void OnStopRunning();
  public:
    std::shared_ptr<Scene> GetOwner();
    ISystem();
    void Enable();
    void Disable();
    [[nodiscard]] bool Enabled() const;
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void PreUpdate();
    virtual void Update();
    virtual void FixedUpdate();
    virtual void LateUpdate();
    // Will only exec when editor is enabled, and no matter application is running or not.
    virtual void OnGui();
};
} // namespace UniEngine