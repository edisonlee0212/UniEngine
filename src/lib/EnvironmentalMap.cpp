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
    m_lightProbe.Get<LightProbe>()->ConstructFromCubemap(targetCubemap);
    m_reflectionProbe = AssetManager::CreateAsset<ReflectionProbe>();
    m_reflectionProbe.Get<ReflectionProbe>()->ConstructFromCubemap(targetCubemap);
    m_ready = true;
}
void EnvironmentalMap::LoadInternal(const std::filesystem::path &path)
{
    auto cubemap = AssetManager::CreateAsset<Cubemap>();
    cubemap->SetPathAndLoad(path);
    Construct(cubemap);
    m_name = path.filename().string();
}
