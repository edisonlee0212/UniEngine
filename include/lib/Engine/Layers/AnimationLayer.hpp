#pragma once
#include "Engine/ECS/Entities.hpp"
#include <Transform.hpp>
#include "ILayer.hpp"
namespace UniEngine
{
class UNIENGINE_API AnimationLayer : public ILayer
{
  private:
    void PreUpdate() override;
};
} // namespace UniEngine