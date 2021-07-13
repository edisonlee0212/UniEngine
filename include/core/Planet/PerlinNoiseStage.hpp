#pragma once
#include <Application.hpp>
#include <Planet/TerrainConstructionStageBase.hpp>
using namespace UniEngine;
namespace Planet
{
class PerlinNoiseStage : public TerrainConstructionStageBase
{
  public:
    void Process(glm::dvec3 point, double previousResult, double &elevation) override;
};

} // namespace Planet