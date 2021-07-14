#pragma once
#include <EntityManager.hpp>
#include <RigidBody.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
enum class ArticulationType{
    Maximal,
    Reduced
};

class UNIENGINE_API Articulation : public PrivateComponentBase
{
    friend class PhysicsManager;
    friend class PhysicsSystem;
    ArticulationType m_type = ArticulationType::Reduced;


    bool m_currentRegistered = false;
    glm::mat4 m_shapeTransform =
        glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    bool m_drawBounds = false;
    glm::vec3 m_shapeParam = glm::vec3(1.0f);
    ShapeType m_shapeType = ShapeType::Box;
    std::shared_ptr<PhysicsMaterial> m_material;
    PxShape *m_shape = nullptr;
    float m_density = 10.0f;
    PxVec3 m_massCenter = PxVec3(0.0f);
    PxVec3 m_linearVelocity = PxVec3(0.0f);
    PxVec3 m_angularVelocity = PxVec3(0.0f);
    bool m_shapeUpdated = false;

    PxU32 m_minPositionIterations = 4;
    PxU32 m_minVelocityIterations = 1;


    PxArticulationBase* m_articulation = nullptr;
    PxArticulationLink* m_root = nullptr;

  public:
    void OnCreate() override;
    void SetSolverIterations(const unsigned &position = 4, const unsigned &velocity = 1);
    void SetDensityAndMassCenter(const float &value, const glm::vec3 &center = glm::vec3(0.0f));
    void SetLinearVelocity(const glm::vec3 &velocity);
    void SetAngularVelocity(const glm::vec3 &velocity);
    void ApplyMeshBound();
    void SetShapeType(ShapeType type);
    void SetShapeParam(glm::vec3 value);
    void SetShapeTransform(glm::mat4 value);
    void SetMaterial(const std::shared_ptr<PhysicsMaterial> &value);
    void UpdateBody();
    [[nodiscard]] ArticulationType GetType() const;
    void SetType(const ArticulationType& mType);
    void OnGui() override;
    void OnDestroy() override;
};

} // namespace UniEngine