#include <IAsset.hpp>
using namespace UniEngine;
#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

AssetHandle::AssetHandle() : m_value(s_UniformDistribution(eng))
{
}

AssetHandle::AssetHandle(uint64_t value) : m_value(value)
{
}

AssetHandle::AssetHandle(const AssetHandle &other) : m_value(other.m_value)
{
}