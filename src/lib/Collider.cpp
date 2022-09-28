#include <Collider.hpp>
#include "Editor.hpp"
#include <PhysicsLayer.hpp>
#include "Engine/Rendering/Graphics.hpp"
#include "Application.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;
AssetRegistration<Collider> ColliderReg("Collider", {"uecollider"});
void Collider::OnCreate()
{
    auto physicsLayer = Application::GetLayer<PhysicsLayer>();
    if(!physicsLayer) return;
    if(!m_physicsMaterial.Get<PhysicsMaterial>()) m_physicsMaterial = physicsLayer->m_defaultPhysicsMaterial;
    m_shape = physicsLayer->m_physics->createShape(
        PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z),
        *m_physicsMaterial.Get<PhysicsMaterial>()->m_value);
}

static const char *RigidBodyShape[]{"Sphere", "Box", "Capsule"};
void Collider::OnGui()
{
    bool statusChanged = false;
    if(ImGui::Combo(
        "Shape", reinterpret_cast<int *>(&m_shapeType), RigidBodyShape, IM_ARRAYSIZE(RigidBodyShape))){
        statusChanged = true;
    }
    Editor::DragAndDropButton<PhysicsMaterial>(m_physicsMaterial, "Physics Mat");
    auto mat = m_physicsMaterial.Get<PhysicsMaterial>();
    if(mat)
    {
        if (ImGui::TreeNode("Material"))
        {
            mat->OnGui();
            ImGui::TreePop();
        }
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
    auto physicsLayer = Application::GetLayer<PhysicsLayer>();
    if(!physicsLayer) return;
    if(m_attachCount != 0){
        UNIENGINE_ERROR("Unable to modify collider, attached to rigidbody!");
    }
    m_shapeType = type;
    switch (m_shapeType)
    {
    case ShapeType::Sphere:
        m_shape = physicsLayer->m_physics->createShape(PxSphereGeometry(m_shapeParam.x), *m_physicsMaterial.Get<PhysicsMaterial>()->m_value, false);
        break;
    case ShapeType::Box:
        m_shape = physicsLayer->m_physics->createShape(PxBoxGeometry(m_shapeParam.x, m_shapeParam.y, m_shapeParam.z), *m_physicsMaterial.Get<PhysicsMaterial>()->m_value, false);
        break;
    case ShapeType::Capsule:
        m_shape = physicsLayer->m_physics->createShape(PxCapsuleGeometry(m_shapeParam.x, m_shapeParam.y), *m_physicsMaterial.Get<PhysicsMaterial>()->m_value, false);
        break;
    }
}
void Collider::SetMaterial(const std::shared_ptr<PhysicsMaterial> &material)
{
    if(m_attachCount != 0){
        UNIENGINE_ERROR("Unable to modify collider, attached to rigidbody!");
    }
    m_physicsMaterial = material;
    PxMaterial* materials[1];
    materials[0] = material->m_value;
    m_shape->setMaterials(materials, 1);
}
void Collider::SetShapeParam(const glm::vec3 &param)
{
    if(m_attachCount != 0){
        UNIENGINE_ERROR("Unable to modify collider, attached to rigidbody!");
    }
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
void Collider::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_physicsMaterial);
}
void Collider::Serialize(YAML::Emitter &out)
{
    m_physicsMaterial.Save("m_physicsMaterial", out);
    out << YAML::Key << "m_shapeParam" << YAML::Value << m_shapeParam;
    out << YAML::Key << "m_attachCount" << YAML::Value << m_attachCount;
    out << YAML::Key << "m_shapeType" << YAML::Value << (unsigned)m_shapeType;
}
void Collider::Deserialize(const YAML::Node &in)
{
    m_physicsMaterial.Load("m_physicsMaterial", in);
    m_shapeParam = in["m_shapeParam"].as<glm::vec3>();
    m_shapeType = (ShapeType)in["m_shapeType"].as<unsigned>();
    SetShapeType(m_shapeType);
    SetShapeParam(m_shapeParam);
    auto mat = m_physicsMaterial.Get<PhysicsMaterial>();
    if(!mat){
        auto physicsLayer = Application::GetLayer<PhysicsLayer>();
        if(!physicsLayer) return;
        mat = physicsLayer->m_defaultPhysicsMaterial;
    }
    SetMaterial(mat);
    m_attachCount = in["m_attachCount"].as<size_t>();
}
