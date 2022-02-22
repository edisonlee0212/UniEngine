#include <AssetManager.hpp>
#include "Editor.hpp"
#include "Cubemap.hpp"
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
AssetRef EnvironmentalMap::GetCubemap() {
    return m_targetCubemap;
}
void EnvironmentalMap::ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap)
{
    m_targetCubemap = targetCubemap;
    m_gamma = targetCubemap->m_gamma;

    m_lightProbe = AssetManager::CreateAsset<LightProbe>();
    m_lightProbe.Get<LightProbe>()->ConstructFromCubemap(targetCubemap);
    m_reflectionProbe = AssetManager::CreateAsset<ReflectionProbe>();
    m_reflectionProbe.Get<ReflectionProbe>()->ConstructFromCubemap(targetCubemap);
    m_ready = true;
}

void EnvironmentalMap::OnInspect()
{
    static AssetRef targetTexture;
    if(Editor::DragAndDropButton<Cubemap>(targetTexture, "Convert from cubemap")){
        auto tex = targetTexture.Get<Cubemap>();
        if(tex)
            ConstructFromCubemap(tex);
        targetTexture.Clear();
    }
}
