#pragma once
#include "Collider.hpp"
#include "Entities.hpp"
#include "PhysicsMaterial.hpp"
#include "uniengine_export.h"
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API RigidBody : public IPrivateComponent
{
    glm::mat4 m_shapeTransform =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    bool m_drawBounds = false;

    std::vector<AssetRef> m_colliders;

    bool m_static = false;
    friend class PhysicsSystem;
    friend class PhysicsLayer;
    friend class TransformLayer;
    PxRigidActor *m_rigidActor = nullptr;

    float m_density = 10.0f;
    PxVec3 m_massCenter = PxVec3(0.0f);
    bool m_currentRegistered = false;
    PxVec3 m_linearVelocity = PxVec3(0.0f);
    PxVec3 m_angularVelocity = PxVec3(0.0f);
    friend class Joint;
    friend class Editor;
    bool m_kinematic = false;
    PxReal m_linearDamping = 0.5;
    PxReal m_angularDamping = 0.5;

    PxU32 m_minPositionIterations = 4;
    PxU32 m_minVelocityIterations = 1;
    bool m_gravity = true;

  public:
    [[nodiscard]] bool Registered() const;
    void AttachCollider(std::shared_ptr<Collider> &collider);
    void DetachCollider(size_t index);
    [[nodiscard]] bool IsKinematic() const;
    void SetSolverIterations(unsigned position = 4, unsigned velocity = 1);
    void SetEnableGravity(bool value);
    void SetLinearDamping(float value);
    void SetAngularDamping(float value);
    void SetKinematic(bool value);
    void SetDensityAndMassCenter(float value, const glm::vec3 &center = glm::vec3(0.0f));
    void SetLinearVelocity(const glm::vec3 &velocity);
    void SetAngularVelocity(const glm::vec3 &velocity);
    void SetStatic(bool value);
    bool IsStatic();
    void SetShapeTransform(const glm::mat4 &value);
    void OnDestroy() override;
    void RecreateBody();
    void OnCreate() override;
    void OnInspect() override;

    void AddForce(const glm::vec3& force);
    void AddTorque(const glm::vec3& torque);


    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
    void CollectAssetRef(std::vector<AssetRef> &list) override;
    void PostCloneAction(const std::shared_ptr<IPrivateComponent> &target) override;
};
} // namespace UniEngine
