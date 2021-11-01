#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API ILayer
{
  private:
    friend class Application;
    virtual void PreUpdate() {}
    virtual void Update() {}
    virtual void LateUpdate() {}
    virtual void OnInspect() {}
};
}