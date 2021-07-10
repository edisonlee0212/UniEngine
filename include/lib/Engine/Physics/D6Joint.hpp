#pragma once
#include <RigidBody.hpp>
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API D6Joint : public PrivateComponentBase
{
    friend class PhysicsManager;
    PxD6Joint* m_joint;

    bool m_xLocked = true;
    bool m_yLocked = true;
    bool m_zLocked = true;

    PxD6JointDrive m_xDrive;
    bool m_xAccleration = true;
    PxD6JointDrive m_yDrive;
    bool m_yAccleration = true;
    PxD6JointDrive m_zDrive;
    bool m_zAccleration = true;
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