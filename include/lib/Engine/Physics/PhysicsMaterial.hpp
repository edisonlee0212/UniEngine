#pragma once
#include <EntityManager.hpp>
#include <uniengine_export.h>
#include <ResourceBehaviour.hpp>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API PhysicsMaterial : public ResourceBehaviour
{
    friend class PhysicsManager;
    PxMaterial* m_value;
    float m_staticFriction = 0.5f;
    float m_dynamicFriction = 0.5f;
    float m_restitution = 0.6f;
  public:
    void SetDynamicFriction(const float& value);
    void SetStaticFriction(const float& value);
    void SetRestitution(const float& value);
    void OnCreate() override;
    void OnGui();
    ~PhysicsMaterial();
};
} // namespace UniEngine