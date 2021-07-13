#pragma once
#include <Application.hpp>
using namespace UniEngine;
namespace Planet
{
class TerrainConstructionStageBase
{
  public:
    virtual ~TerrainConstructionStageBase() = default;
    virtual void Process(glm::dvec3 point, double previousResult, double &elevation) = 0;
};
} // namespace Planet
