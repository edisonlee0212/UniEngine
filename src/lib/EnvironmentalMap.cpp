#include <ResourceManager.hpp>

#include <EnvironmentalMap.hpp>
using namespace UniEngine;

bool EnvironmentalMap::IsReady() const
{
    return m_ready;
}

void EnvironmentalMap::OnCreate()
{
    m_name = "New environmental map";
}

void EnvironmentalMap::Construct(const std::shared_ptr<Cubemap> &targetCubemap)
{
    m_skybox = targetCubemap;
    m_lightProbe = ResourceManager::CreateResource<LightProbe>();
    m_lightProbe->ConstructFromCubemap(m_skybox);
    m_reflectionProbe = ResourceManager::CreateResource<ReflectionProbe>();
    m_reflectionProbe->ConstructFromCubemap(m_skybox);
    m_ready = true;
}

void EnvironmentalMap::Construct(
    const std::shared_ptr<Cubemap> &targetSkybox,
    const std::shared_ptr<Cubemap> &targetCubemapForProbes)
{
    m_skybox = targetSkybox;
    m_lightProbe = ResourceManager::CreateResource<LightProbe>();
    m_lightProbe->ConstructFromCubemap(m_skybox);
    m_reflectionProbe = ResourceManager::CreateResource<ReflectionProbe>();
    m_reflectionProbe->ConstructFromCubemap(m_skybox);
    m_ready = true;
}

void EnvironmentalMap::Construct(
    const std::shared_ptr<Cubemap> &targetCubemap,
    const std::shared_ptr<LightProbe> &targetlightProbe,
    const std::shared_ptr<ReflectionProbe> &targetReflectionProbe)
{
    m_skybox = targetCubemap;
    m_lightProbe = targetlightProbe;
    m_reflectionProbe = targetReflectionProbe;
    m_ready = true;
}
