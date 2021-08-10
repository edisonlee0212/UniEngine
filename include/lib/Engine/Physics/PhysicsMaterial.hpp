#pragma once
#include <EntityManager.hpp>
#include <IAsset.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API PhysicsMaterial : public IAsset
{
    friend class PhysicsManager;
    friend class Collider;
    PxMaterial *m_value;
    float m_staticFriction = 0.02f;
    float m_dynamicFriction = 0.02f;
    float m_restitution = 0.8f;

  public:
    void SetDynamicFriction(const float &value);
    void SetStaticFriction(const float &value);
    void SetRestitution(const float &value);
    void OnCreate() override;
    void OnGui();
    ~PhysicsMaterial();
};
} // namespace UniEngine