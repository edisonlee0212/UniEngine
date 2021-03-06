#pragma once
#include <EntityManager.hpp>
#include <IAsset.hpp>
#include <PhysicsMaterial.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
enum class UNIENGINE_API ShapeType
{
    Sphere,
    Box,
    Capsule
};
class UNIENGINE_API Collider : public IAsset
{
    friend class PhysicsManager;
    friend class RigidBody;
    PxShape *m_shape = nullptr;
    glm::vec3 m_shapeParam = glm::vec3(1.0f);
    ShapeType m_shapeType = ShapeType::Box;
    std::shared_ptr<PhysicsMaterial> m_material;

    bool m_attached = false;

  public:
    void OnGui();
    void OnCreate() override;
    ~Collider();
    void SetShapeType(const ShapeType& type);
    void SetShapeParam(const glm::vec3& param);
    void SetMaterial(std::shared_ptr<PhysicsMaterial>& material);
};
} // namespace UniEngine