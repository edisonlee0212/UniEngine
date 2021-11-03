#pragma once
#include <EntityManager.hpp>
#include <Transform.hpp>
#include "ILayer.hpp"
namespace UniEngine
{
class UNIENGINE_API AnimationLayer : public ILayer
{
  private:
    void PreUpdate() override;
    void LateUpdate() override;
};
} // namespace UniEngine