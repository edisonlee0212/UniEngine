#pragma once
#include <EntityManager.hpp>
#include <Transform.hpp>
namespace UniEngine
{
class UNIENGINE_API TransformManager : public ISingleton<TransformManager>
{
    friend class PhysicsSystem;
    EntityQuery m_transformQuery;
    bool m_physicsSystemOverride = false;
    static void CalculateTransformGraph(std::vector<EntityMetadata>& entityInfos, const GlobalTransform &parentGlobalTransform, Entity parent);
  public:
    static void CalculateTransformGraphForDescendents(const Entity& entity);
    static void CalculateTransformGraphs(std::shared_ptr<Scene> scene);
    static void Init();
    static void PreUpdate();
};
} // namespace UniEngine