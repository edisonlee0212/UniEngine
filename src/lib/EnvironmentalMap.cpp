#include <AssetManager.hpp>

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
    m_targetCubemap = targetCubemap;
    m_gamma = targetCubemap->m_gamma;

    m_lightProbe = AssetManager::CreateAsset<LightProbe>();
    m_lightProbe->ConstructFromCubemap(m_targetCubemap);
    m_reflectionProbe = AssetManager::CreateAsset<ReflectionProbe>();
    m_reflectionProbe->ConstructFromCubemap(m_targetCubemap);
    m_ready = true;
}
void EnvironmentalMap::Load(const std::filesystem::path &path)
{
    Construct(AssetManager::Import<Cubemap>(path));
    m_name = path.filename().string();
}
