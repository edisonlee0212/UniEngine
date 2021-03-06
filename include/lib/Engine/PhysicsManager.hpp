#pragma once
#include <ISingleton.hpp>
#include <Joint.hpp>
#include <PhysicsMaterial.hpp>
#include <uniengine_export.h>
#include <Articulation.hpp>
#include <Collider.hpp>
using namespace physx;
namespace UniEngine
{
class RigidBody;

class UNIENGINE_API PhysicsManager : public ISingleton<PhysicsManager>
{
    PxPvdTransport *m_pvdTransport;
    PxDefaultAllocator m_allocator;
    PxDefaultErrorCallback m_errorCallback;
    PxFoundation *m_physicsFoundation;
    PxPhysics *m_physics;
    PxDefaultCpuDispatcher *m_dispatcher;
    PxPvd *m_physVisDebugger;
    friend class RigidBody;
    friend class Joint;
    friend class Articulation;
    friend class PhysicsSystem;
    friend class PhysicsMaterial;
    friend class Collider;
  public:
    std::shared_ptr<PhysicsMaterial> m_defaultMaterial;
    static void UploadTransform(const GlobalTransform &globalTransform, RigidBody &rigidBody);
    static void UploadTransform(const GlobalTransform &globalTransform, Articulation &rigidBody);
    static void PreUpdate();
    static void Init();
    static void Destroy();
};

class UNIENGINE_API PhysicsSystem : public ISystem
{
    PxScene *m_physicsScene = nullptr;
  public:
    void OnCreate() override;
    void OnDestroy() override;
    void FixedUpdate() override;
    void Simulate(float time) const;
};
} // namespace UniEngine
