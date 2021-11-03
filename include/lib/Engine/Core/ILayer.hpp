#pragma once
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API ILayer
{
    bool m_enabled = true;
    friend class Application;
  private:

    friend class Application;
    virtual void OnCreate()
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

  public:
    void SetEnable(bool value)
    {
        m_enabled = value;
    }
    [[nodiscard]] bool IsEnabled() const
    {
        return m_enabled;
    };
};
} // namespace UniEngine