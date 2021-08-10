#include <IHandle.hpp>
using namespace UniEngine;

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

UniEngine::Handle::Handle() : m_value(s_UniformDistribution(eng))
{
}

UniEngine::Handle::Handle(uint64_t value) : m_value(value)
{
}

UniEngine::Handle::Handle(const Handle &other) : m_value(other.m_value)
{
}