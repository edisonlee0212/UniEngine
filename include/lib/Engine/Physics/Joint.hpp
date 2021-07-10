#pragma once
#include <RigidBody.hpp>
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API Joint : public PrivateComponentBase
{
    friend class PhysicsManager;
    PxFixedJoint* m_joint;
  public:
    Entity m_linkedEntity;
    void Unlink();
    bool Linked();
    bool SafetyCheck();
    void Init() override;
    void Link();
    void OnGui() override;
    ~D6Joint();
};
}