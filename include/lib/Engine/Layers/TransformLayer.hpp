#pragma once
#include "Engine/ECS/Entities.hpp"
#include <Transform.hpp>
#include "ILayer.hpp"
namespace UniEngine
{
class UNIENGINE_API TransformLayer : public ILayer
{
    friend class PhysicsSystem;
    EntityQuery m_transformQuery;
    bool m_physicsSystemOverride = false;
    void CalculateTransformGraph(const std::shared_ptr<Scene>& scene, std::vector<EntityMetadata>& entityInfos, const GlobalTransform &parentGlobalTransform, Entity parent);
    void OnCreate() override;
    void PreUpdate() override;
  public:
    void CalculateTransformGraphForDescendents(const std::shared_ptr<Scene>& scene, const Entity& entity);
    void CalculateTransformGraphs(const std::shared_ptr<Scene>& scene, bool checkStatic = true);
};
} // namespace UniEngine