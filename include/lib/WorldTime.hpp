#pragma once
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API WorldTime
{
    friend class World;
    friend class Application;
    double m_frameStartTime;
    double m_fixedDeltaTime;
    double m_deltaTime;
    double m_lastFrameTime;
    float m_timeStep;
    void AddFixedDeltaTime(const double &value);

  public:
    float TimeStep() const;
    float FixedDeltaTime() const;
    float DeltaTime() const;
    float LastFrameTime() const;
};
} // namespace UniEngine