#include "Editor.hpp"
#include <Joint.hpp>
#include <PhysicsLayer.hpp>
#include <RigidBody.hpp>
using namespace UniEngine;
#pragma region Fixed
void Joint::FixedGui()
{
}
#pragma endregion
/*
#pragma region Distance
void Joint::DistanceGui()
{
    if (ImGui::DragFloat("Min", &m_minDistance, 0.1f, FLT_MIN, m_maxDistance))
    {
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Enabled", &m_minDistanceEnabled))
    {
        SetMin(m_minDistance, m_minDistanceEnabled);
    }
    if (ImGui::DragFloat("Max", &m_maxDistance, 0.1f, m_minDistance, FLT_MAX))
    {
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Enabled", &m_maxDistanceEnabled))
    {
        SetMax(m_maxDistance, m_maxDistanceEnabled);
    }

    if (ImGui::DragFloat("Stiffness", &m_stiffness))
    {
        SetStiffness(m_stiffness);
    }
    if (ImGui::DragFloat("Damping", &m_damping))
    {
        SetDamping(m_damping);
    }
}
void Joint::SetMax(const float &value, const bool &enabled)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_maxDistance != value || m_maxDistanceEnabled != enabled)
    {
        m_maxDistance = value;
        m_maxDistanceEnabled = enabled;
        static_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, m_maxDistanceEnabled);
        static_cast<PxDistanceJoint *>(m_joint)->setMaxDistance(m_maxDistance);
    }
}
void Joint::SetMin(const float &value, const bool &enabled)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_minDistance != value || m_maxDistanceEnabled != enabled)
    {
        m_minDistance = value;
        m_minDistanceEnabled = enabled;
        static_cast<PxDistanceJoint *>(m_joint)->setDistanceJointFlag(
            PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, m_minDistanceEnabled);
        static_cast<PxDistanceJoint *>(m_joint)->setMinDistance(m_minDistance);
    }
}
void Joint::SetStiffness(const float &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_stiffness != value)
    {
        m_stiffness = value;
        static_cast<PxDistanceJoint *>(m_joint)->setStiffness(m_stiffness);
    }
}
void Joint::SetDamping(const float &value)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::Distance))
        return;
    if (m_damping != value)
    {
        m_damping = value;
        static_cast<PxDistanceJoint *>(m_joint)->setDamping(m_damping);
    }
}

#pragma endregion
#pragma region Spherical
void Joint::SphericalGui()
{
}
#pragma endregion
#pragma region Revolute
void Joint::RevoluteGui()
{
}
#pragma endregion
#pragma region Prismatic
void Joint::PrismaticGui()
{
}
#pragma endregion
 */
#pragma region D6

void Joint::D6Gui()
{
    auto *joint = static_cast<PxD6Joint *>(m_joint);
}
#pragma endregion
void Joint::Unlink()
{
    if(!m_linked) return;
    if (m_joint)
    {
        m_joint->release();
        m_joint = nullptr;
    }
    m_linked = false;
}
bool Joint::Linked()
{
    return m_linked;
}

void Joint::OnCreate()
{
}
static const char *JointTypeNames[]{"Fixed", "D6"};
void Joint::OnInspect()
{
    static int type = 0;
    type = (int)m_jointType;
    if (ImGui::Combo("Joint Type", &type, JointTypeNames, IM_ARRAYSIZE(JointTypeNames)))
    {
        SetType((JointType)type);
    }
    auto storedRigidBody1 = m_rigidBody1.Get<RigidBody>();
    auto storedRigidBody2 = m_rigidBody2.Get<RigidBody>();
    Editor::DragAndDropButton<RigidBody>(m_rigidBody1, "Link 1");
    Editor::DragAndDropButton<RigidBody>(m_rigidBody2, "Link 2");
    if (m_rigidBody1.Get<RigidBody>() != storedRigidBody1 || m_rigidBody2.Get<RigidBody>() != storedRigidBody2)
    {
        Unlink();
    }
    if (m_joint)
    {
        switch (m_jointType)
        {
        case JointType::Fixed:
            FixedGui();
            break;
            /*
        case JointType::Distance:
            DistanceGui();
            break;
        case JointType::Spherical:
            SphericalGui();
            break;
        case JointType::Revolute:
            RevoluteGui();
            break;
        case JointType::Prismatic:
            PrismaticGui();
            break;
             */
        case JointType::D6:
            D6Gui();
            break;
        }
    }
}

void Joint::OnDestroy()
{
    Unlink();
    m_rigidBody1.Clear();
    m_rigidBody2.Clear();
}


bool Joint::TypeCheck(const JointType &type)
{
    if (m_jointType != type)
    {
        UNIENGINE_ERROR("Wrong joint type!");
        return false;
    }
    return true;
}
void Joint::SetType(const JointType &type)
{
    if (type != m_jointType)
    {
        m_jointType = type;
        Unlink();
    }
}

void Joint::SetMotion(const MotionAxis &axis, const MotionType &type)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    m_motionTypes[static_cast<int>(axis)] = static_cast<PxD6Motion::Enum>(type);
    static_cast<PxD6Joint *>(m_joint)->setMotion(
        static_cast<PxD6Axis::Enum>(axis), static_cast<PxD6Motion::Enum>(type));
}
void Joint::SetDrive(const DriveType &type, const float &stiffness, const float &damping, const bool &isAcceleration)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    m_drives[static_cast<int>(type)].stiffness = stiffness;
    m_drives[static_cast<int>(type)].damping = damping;
    m_drives[static_cast<int>(type)].flags =
        static_cast<PxD6JointDriveFlag::Enum>(isAcceleration ? PxU32(PxD6JointDriveFlag::eACCELERATION) : 0);
    static_cast<PxD6Joint *>(m_joint)->setDrive(static_cast<PxD6Drive::Enum>(type), m_drives[static_cast<int>(type)]);
}
void Joint::SetDistanceLimit(
    const float &toleranceLength, const float &toleranceSpeed, const float &extent, const float &contactDist)
{
    if (!m_joint)
        return;
    if (!TypeCheck(JointType::D6))
        return;
    auto scale = PxTolerancesScale();
    scale.length = toleranceLength;
    scale.speed = toleranceSpeed;
    static_cast<PxD6Joint *>(m_joint)->setDistanceLimit(PxJointLinearLimit(scale, extent, contactDist));
}
void Joint::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
    m_joint = nullptr;
    m_linked = false;
}
void Joint::Relink(const std::unordered_map<Handle, Handle> &map, const std::shared_ptr<Scene> &scene)
{
    m_rigidBody1.Relink(map, scene);
    m_rigidBody2.Relink(map, scene);
}

void Joint::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_jointType" << YAML::Value << (unsigned)m_jointType;
    out << YAML::Key << "m_localPosition1" << YAML::Value << m_localPosition1;
    out << YAML::Key << "m_localPosition2" << YAML::Value << m_localPosition2;
    out << YAML::Key << "m_localRotation1" << YAML::Value << m_localRotation1;
    out << YAML::Key << "m_localRotation2" << YAML::Value << m_localRotation2;

    m_rigidBody1.Save("m_rigidBody1", out);
    m_rigidBody2.Save("m_rigidBody2", out);


    switch (m_jointType)
    {
    case JointType::Fixed:

        break;
    /*
case JointType::Distance:
    DistanceGui();
    break;
case JointType::Spherical:
    SphericalGui();
    break;
case JointType::Revolute:
    RevoluteGui();
    break;
case JointType::Prismatic:
    PrismaticGui();
    break;
     */
    case JointType::D6:
        out << YAML::Key << "m_motionTypes" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < 6; i++)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Index" << YAML::Value << i;
            out << YAML::Key << "MotionType" << YAML::Value << (unsigned)m_motionTypes[i];
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::Key << "m_drives" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < 6; i++)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Index" << YAML::Value << i;
            out << YAML::Key << "Stiffness" << YAML::Value << (float)m_drives[i].stiffness;
            out << YAML::Key << "Damping" << YAML::Value << (float)m_drives[i].damping;
            out << YAML::Key << "Flags" << YAML::Value << (unsigned)m_drives[i].flags;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        break;
    }
}
void Joint::Deserialize(const YAML::Node &in)
{
    m_jointType = (JointType)in["m_jointType"].as<unsigned>();
    m_localPosition1 = in["m_localPosition1"].as<glm::vec3>();
    m_localPosition2 = in["m_localPosition2"].as<glm::vec3>();
    m_localRotation1 = in["m_localRotation1"].as<glm::quat>();
    m_localRotation2 = in["m_localRotation2"].as<glm::quat>();

    m_rigidBody1.Load("m_rigidBody1", in, GetScene());
    m_rigidBody2.Load("m_rigidBody2", in, GetScene());

    switch (m_jointType)
    {
    case JointType::Fixed:
        break;
    /*
case JointType::Distance:
    DistanceGui();
    break;
case JointType::Spherical:
    SphericalGui();
    break;
case JointType::Revolute:
    RevoluteGui();
    break;
case JointType::Prismatic:
    PrismaticGui();
    break;
     */
    case JointType::D6:
        auto inMotionTypes = in["m_motionTypes"];
        for (const auto &inMotionType : inMotionTypes)
        {
            int index = inMotionType["Index"].as<int>();
            m_motionTypes[index] = (PxD6Motion::Enum)inMotionType["MotionType"].as<unsigned>();
        }
        auto inDrives = in["m_drives"];
        for (const auto &inDrive : inDrives)
        {
            int index = inDrive["Index"].as<int>();
            m_drives[index].stiffness = inDrive["Stiffness"].as<float>();
            m_drives[index].damping = inDrive["Damping"].as<float>();
            m_drives[index].flags = (PxD6JointDriveFlag::Enum)inDrive["Flags"].as<unsigned>();
        }
        break;
    }

    m_linked = false;
    m_joint = nullptr;
}
void Joint::Link(const Entity &entity, bool reverse)
{
    auto scene = GetScene();
    const auto owner = GetOwner();
    if (scene->HasPrivateComponent<RigidBody>(owner) && scene->HasPrivateComponent<RigidBody>(entity))
    {
        if(!reverse)
        {
            m_rigidBody1.Set(scene->GetOrSetPrivateComponent<RigidBody>(owner).lock());
            m_rigidBody2.Set(scene->GetOrSetPrivateComponent<RigidBody>(entity).lock());
        }else{
            m_rigidBody2.Set(scene->GetOrSetPrivateComponent<RigidBody>(owner).lock());
            m_rigidBody1.Set(scene->GetOrSetPrivateComponent<RigidBody>(entity).lock());
        }
        Unlink();
    }
}
