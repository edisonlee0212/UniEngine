#pragma once
#include <uniengine_export.h>
namespace UniEngine
{
class ThreadPool;
class World;
class UNIENGINE_API SystemBase
{
    friend class World;

  protected:
    bool m_enabled;
    World *m_world;
    virtual void OnStartRunning();
    virtual void OnStopRunning();

  public:
    SystemBase();
    void Enable();
    void Disable();
    bool Enabled() const;
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual void PreUpdate();
    virtual void Update();
    virtual void FixedUpdate();
    virtual void LateUpdate();
};
} // namespace UniEngine