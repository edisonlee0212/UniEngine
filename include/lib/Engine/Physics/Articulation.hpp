#pragma once
#include <RigidBody.hpp>
#include <EntityManager.hpp>
#include <uniengine_export.h>
using namespace physx;
namespace UniEngine
{
class UNIENGINE_API Articulation : public PrivateComponentBase{
  public:
    void Init();
};
class UNIENGINE_API ArticulationLink : public PrivateComponentBase{
    Entity m_linkedEntity = Entity();
  public:

};
}