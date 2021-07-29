#pragma once
#include <ISingleton.hpp>
#include <Joint.hpp>
#include <PhysicsMaterial.hpp>
#include <uniengine_export.h>
#include <Collider.hpp>
using namespace physx;
namespace YAML
{
class Node;
class Emitter;
template<>
struct convert<PxVec2> {
    static Node encode(const PxVec2& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node& node, PxVec2& rhs) {
        if(!node.IsSequence() || node.size() != 2) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
    }
};
template<>
struct convert<PxVec3> {
    static Node encode(const PxVec3& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node& node, PxVec3& rhs) {
        if(!node.IsSequence() || node.size() != 3) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
template<>
struct convert<PxVec4> {
    static Node encode(const PxVec4& rhs) {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node& node, PxVec4& rhs) {
        if(!node.IsSequence() || node.size() != 4) {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};
template<>
struct convert<PxMat44> {
    static Node encode(const PxMat44& rhs) {
        Node node;
        node.push_back(rhs[0]);
        node.push_back(rhs[1]);
        node.push_back(rhs[2]);
        node.push_back(rhs[3]);
        return node;
    }

    static bool decode(const Node& node, PxMat44& rhs) {
        if(!node.IsSequence() || node.size() != 4) {
            return false;
        }

        rhs[0] = node[0].as<PxVec4>();
        rhs[1] = node[1].as<PxVec4>();
        rhs[2] = node[2].as<PxVec4>();
        rhs[3] = node[3].as<PxVec4>();
        return true;
    }
};
} // namespace YAML


namespace UniEngine
{
class RigidBody;
YAML::Emitter &operator<<(YAML::Emitter &out, const PxVec2 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const PxVec3 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const PxVec4 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const PxMat44 &v);
class UNIENGINE_API PhysicsManager : public ISingleton<PhysicsManager>
{
    PxPvdTransport *m_pvdTransport;
    PxDefaultAllocator m_allocator;
    PxDefaultErrorCallback m_errorCallback;
    PxFoundation *m_physicsFoundation;
    PxPhysics *m_physics;
    PxDefaultCpuDispatcher *m_dispatcher;
    PxPvd *m_physVisDebugger;

    friend class RigidBody;
    friend class Joint;
    friend class Articulation;
    friend class PhysicsSystem;
    friend class PhysicsMaterial;
    friend class Collider;
  public:
    static void UploadTransforms(const bool& updateAll, const bool& freeze = false);

    static void UploadTransform(const GlobalTransform &globalTransform, RigidBody &rigidBody);
    static void PreUpdate();
    static void Init();
    static void Destroy();
};

class UNIENGINE_API PhysicsSystem : public ISystem
{
    PxScene *m_physicsScene = nullptr;
  public:
    void OnEnable();
    void OnCreate() override;
    void OnDestroy() override;
    void FixedUpdate() override;
    void Simulate(float time) const;
};
} // namespace UniEngine
