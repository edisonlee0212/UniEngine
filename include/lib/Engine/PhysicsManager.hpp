#pragma once
#include <FixedJoint.hpp>
#include <ISingleton.hpp>
#include <RigidBody.hpp>
#include <uniengine_export.h>
#include <DistanceJoint.hpp>


using namespace physx;

#define PX_RELEASE(x)                                                                                                  \
    if (x)                                                                                                             \
    {                                                                                                                  \
        x->release();                                                                                                  \
        x = nullptr;                                                                                                   \
    }
namespace UniEngine
{
class UNIENGINE_API PhysicsManager : public ISingleton<PhysicsManager>
{
  public:
    PxDefaultAllocator m_allocator;
    PxDefaultErrorCallback m_errorCallback;
    PxFoundation *m_physicsFoundation;
    PxPhysics *m_physics;
    PxDefaultCpuDispatcher *m_dispatcher;
    PxPvd *m_physVisDebugger;
    PxMaterial *m_defaultMaterial;
    static void UploadTransform(const GlobalTransform& globalTransform, std::unique_ptr<RigidBody> &rigidBody);
    static void PreUpdate();
    static void Init();
    static void Destroy();
    static void UpdateShape(std::unique_ptr<RigidBody> &rigidBody);
};

class UNIENGINE_API PhysicsSystem : public SystemBase
{
    PxScene *m_physicsScene = nullptr;

  public:
    void OnCreate() override;
    void OnDestroy() override;
    void FixedUpdate() override;
    void Simulate(float time) const;
    void CalculateGlobalTransformRecursive(const GlobalTransform &pltw, Entity entity) const;
};
} // namespace UniEngine
