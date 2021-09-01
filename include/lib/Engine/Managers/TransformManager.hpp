#pragma once
#include <EntityManager.hpp>
#include <Transform.hpp>
namespace UniEngine
{
class UNIENGINE_API TransformManager : public ISingleton<TransformManager>
{
    friend class PhysicsSystem;
    EntityQuery m_transformQuery;
    static void CalculateLtwRecursive(const GlobalTransform &pltw, Entity parent);
    bool m_physicsSystemOverride = false;
  public:
    static void CalculateTransformGraphs();

    static void Init();
    static void PreUpdate();
};
} // namespace UniEngine