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
    friend class SerializationManager;
    bool m_enabled;
    float m_rank = 0.0f;
  protected:
    virtual void OnStartRunning();
    virtual void OnStopRunning();
  public:
    [[nodiscard]] float GetRank();
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