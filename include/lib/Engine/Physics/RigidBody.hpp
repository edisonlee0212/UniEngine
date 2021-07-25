#pragma once
#include <EntityManager.hpp>
#include <PhysicsMaterial.hpp>
#include <uniengine_export.h>
#include <Collider.hpp>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API RigidBody : public IPrivateComponent
{
    glm::mat4 m_shapeTransform =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    bool m_drawBounds = false;

    std::vector<std::shared_ptr<Collider>> m_colliders;

    bool m_static = false;
    friend class PhysicsSystem;
    friend class PhysicsManager;
    friend class TransformManager;
    PxRigidActor *m_rigidActor = nullptr;

    float m_density = 10.0f;
    PxVec3 m_massCenter = PxVec3(0.0f);
    bool m_currentRegistered = false;
    PxVec3 m_linearVelocity = PxVec3(0.0f);
    PxVec3 m_angularVelocity = PxVec3(0.0f);
    friend class Joint;
    friend class EditorManager;
    bool m_kinematic = false;
    PxReal m_linearDamping = 0.5;
    PxReal m_angularDamping = 0.5;

    PxU32 m_minPositionIterations = 4;
    PxU32 m_minVelocityIterations = 1;

    std::vector<Entity> m_linkedEntities;
    bool m_gravity = true;
  public:
    void AttachCollider(std::shared_ptr<Collider>& collider);
    void DetachCollider(const size_t& index);
    [[nodiscard]] bool IsKinematic();
    void SetSolverIterations(const unsigned &position = 4, const unsigned &velocity = 1);
    void SetEnableGravity(const bool& value);
    void SetLinearDamping(const float &value);
    void SetAngularDamping(const float &value);
    void SetKinematic(const bool &value);
    void SetDensityAndMassCenter(const float &value, const glm::vec3 &center = glm::vec3(0.0f));
    void SetLinearVelocity(const glm::vec3 &velocity);
    void SetAngularVelocity(const glm::vec3 &velocity);
    void SetStatic(bool value);
    bool IsStatic();
    void SetShapeTransform(glm::mat4 value);
    void OnDestroy() override;
    void UpdateBody();
    void OnCreate() override;
    void OnGui() override;

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine
