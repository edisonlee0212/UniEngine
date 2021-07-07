#pragma once
#include <EntityManager.hpp>
#include <Transform.hpp>
namespace UniEngine
{
class UNIENGINE_API TransformManager : public ISingleton<TransformManager>
{
    friend class PhysicsSystem;
    EntityQuery m_transformQuery;
    static void CalculateLtwRecursive(const GlobalTransform &pltw, Entity entity);

  public:
    static void Init();
    static void LateUpdate();
};
} // namespace UniEngine