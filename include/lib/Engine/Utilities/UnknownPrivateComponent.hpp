#pragma once
#include <Application.hpp>
namespace UniEngine{
class UNIENGINE_API UnknownPrivateComponent : public IPrivateComponent
{
  public:
    void OnInspect() override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target) override;
};

}