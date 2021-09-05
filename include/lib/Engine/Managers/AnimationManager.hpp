#pragma once
#include <EntityManager.hpp>
#include <Transform.hpp>
namespace UniEngine
{
class UNIENGINE_API AnimationManager : public ISingleton<AnimationManager>
{
  public:
    static void PreUpdate();
    static void LateUpdate();
};
} // namespace UniEngine