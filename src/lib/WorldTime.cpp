#include <WorldTime.hpp>
using namespace UniEngine;
void WorldTime::AddFixedDeltaTime(const double &value)
{
    m_fixedDeltaTime += value;
}

float WorldTime::TimeStep() const
{
    return m_timeStep;
}

float WorldTime::FixedDeltaTime() const
{
    return static_cast<float>(m_fixedDeltaTime);
}

float WorldTime::DeltaTime() const
{
    return static_cast<float>(m_deltaTime);
}

float WorldTime::LastFrameTime() const
{
    return static_cast<float>(m_lastFrameTime);
}
