#pragma once
#include <Application.hpp>
namespace UniEngine{
class UNIENGINE_API UnknownPrivateComponent : public IPrivateComponent
{
  public:
    void OnGui() override;
    void Clone(const std::shared_ptr<IPrivateComponent> &target) override;
};

}