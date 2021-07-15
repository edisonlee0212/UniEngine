#pragma once
#include <uniengine_export.h>
namespace UniEngine
{
class ThreadPool;
class World;
class UNIENGINE_API ISystem
{
    friend class World;

  protected:
    bool m_enabled;
    World *m_world;
    virtual void OnStartRunning();
    virtual void OnStopRunning();

  public:
    ISystem();
    void Enable();
    void Disable();
    bool Enabled() const;
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void PreUpdate();
    virtual void Update();
    virtual void FixedUpdate();
    virtual void LateUpdate();

    //Will only exec when editor is enabled, and no matter application is running or not.
    virtual void OnGui();
};
} // namespace UniEngine