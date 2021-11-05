#pragma once
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API ILayer
{
  private:
    friend class Application;
    virtual void OnCreate()
    {
    }
    virtual void OnDestroy()
    {
    }
    virtual void PreUpdate()
    {
    }
    virtual void FixedUpdate()
    {
    }
    virtual void Update()
    {
    }
    virtual void LateUpdate()
    {
    }
    virtual void OnInspect()
    {
    }
};
} // namespace UniEngine