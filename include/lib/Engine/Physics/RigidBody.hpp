#pragma once
#include <uniengine_export.h>
#include <PxPhysicsAPI.h>
#include <EntityManager.hpp>
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
    glm::mat4 m_shapeTransform;
    bool m_drawBounds;
    glm::vec3 m_shapeParam;
    ShapeType m_shapeType;
    bool m_isStatic;
    friend class PhysicsSimulationManager;
    PxRigidActor *m_rigidBody = nullptr;
    PxMaterial *m_material = nullptr;
    PxShape *m_shape = nullptr;
    float m_density;
    PxVec3 m_massCenter;
    bool m_currentRegistered;
    PxVec3 m_linearVelocity;
    PxVec3 m_angularVelocity;
    void RegisterCheck();

  public:
    RigidBody();
    void SetShapeType(ShapeType type);
    void SetShapeParam(glm::vec3 value);
    void SetStatic(bool value);
    void SetTransform(glm::mat4 value);
    void SetDensity(float value);
    ~RigidBody() override;
    void SetMaterial(PxMaterial *value);
    void UpdateShape();
    void UpdateBody();
    void Init() override;
    void OnEntityDisable() override;
    void OnEntityEnable() override;
    void OnDisable() override;
    void OnEnable() override;
    void OnGui() override;
};
} // namespace UniEngine
