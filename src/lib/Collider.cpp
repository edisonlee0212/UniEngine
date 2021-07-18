#include <Collider.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;
void Collider::OnCreate()
{
    m_name = "New Collider";

    m_material = PhysicsManager::GetInstance().m_defaultMaterial;

    m_shape = PhysicsManager::GetInstance().m_physics->createShape(
        PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z),
        *m_material->m_value);
}

static const char *RigidBodyShape[]{"Sphere", "Box", "Capsule"};
void Collider::OnGui()
{
    bool statusChanged = false;
    if(ImGui::Combo(
        "Shape", reinterpret_cast<int *>(&m_shapeType), RigidBodyShape, IM_ARRAYSIZE(RigidBodyShape))){
        statusChanged = true;
    }

    if (ImGui::TreeNode("Material"))
    {
        m_material->OnGui();
        ImGui::TreePop();
    }
    glm::vec3 newParam = m_shapeParam;
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        if (ImGui::DragFloat("Radius", &newParam.x, 0.01f, 0.0001f))
            statusChanged = true;
        break;
    case ShapeType::Box:
        if (ImGui::DragFloat3("XYZ Size", &newParam.x, 0.01f, 0.0f))
            statusChanged = true;
        break;
    case ShapeType::Capsule:
        if (ImGui::DragFloat2("R/HalfH", &newParam.x, 0.01f, 0.0001f))
            statusChanged = true;
        break;
    }
    if(statusChanged){
        SetShapeParam(newParam);
    }
}

Collider::~Collider()
{
    if (m_shape != nullptr)
    {
        m_shape->release();
    }
}
void Collider::SetShapeType(const ShapeType& type)
{
    m_shapeType = type;
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        m_shape = PhysicsManager::GetInstance().m_physics->createShape(PxSphereGeometry(m_shapeParam.x), *m_material->m_value, true);
        break;
    case ShapeType::Box:
        m_shape = PhysicsManager::GetInstance().m_physics->createShape(PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z), *m_material->m_value, true);
        break;
    case ShapeType::Capsule:
        m_shape = PhysicsManager::GetInstance().m_physics->createShape(PxCapsuleGeometry(m_shapeParam.x, m_shapeParam.y), *m_material->m_value, true);
        break;
    }
}
void Collider::SetMaterial(std::shared_ptr<PhysicsMaterial> &material)
{
    if(m_material == material) return;
    m_material = material;
    PxMaterial* materials[1];
    materials[0] = material->m_value;
    m_shape->setMaterials(materials, 1);
}
void Collider::SetShapeParam(const glm::vec3 &param)
{
    m_shapeParam = param;
    m_shapeParam = glm::max(glm::vec3(0.001f), m_shapeParam);
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        m_shape->setGeometry(PxSphereGeometry(m_shapeParam.x));
        break;
    case ShapeType::Box:
        m_shape->setGeometry(PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z));
        break;
    case ShapeType::Capsule:
        m_shape->setGeometry(PxCapsuleGeometry(m_shapeParam.x, m_shapeParam.y));
        break;
    }
}
