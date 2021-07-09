#pragma once
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
enum class UNIENGINE_API ShapeType
{
    Sphere,
    Box,
    Capsule
};
class UNIENGINE_API RigidBody : public PrivateComponentBase
{
    glm::mat4 m_shapeTransform = glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    bool m_drawBounds = false;
    glm::vec3 m_shapeParam = glm::vec3(1.0f);
    ShapeType m_shapeType = ShapeType::Box;
    bool m_isStatic = false;
    friend class PhysicsSystem;
    friend class PhysicsManager;
    PxRigidActor *m_rigidActor = nullptr;
    PxMaterial *m_material = nullptr;
    PxShape *m_shape = nullptr;
    float m_density = 10.0f;
    PxVec3 m_massCenter = PxVec3(0.0f);
    bool m_currentRegistered = false;
    PxVec3 m_linearVelocity = PxVec3(0.0f);
    PxVec3 m_angularVelocity = PxVec3(0.0f);
    bool m_shapeUpdated = false;
    friend class D6Joint;
  public:
    void ApplyMeshBound();
    void SetShapeType(ShapeType type);
    void SetShapeParam(glm::vec3 value);
    void SetStatic(bool value);
    void SetTransform(glm::mat4 value);
    void SetDensity(float value);
    ~RigidBody() override;
    void SetMaterial(PxMaterial *value);
    void UpdateBody();
    void Init() override;
    void OnGui() override;
};
} // namespace UniEngine
