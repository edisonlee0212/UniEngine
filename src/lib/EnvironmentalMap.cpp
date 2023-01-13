#include <ProjectManager.hpp>
#include "Editor.hpp"
#include "Cubemap.hpp"
#include <EnvironmentalMap.hpp>
#include "ClassRegistry.hpp"
using namespace UniEngine;

bool EnvironmentalMap::IsReady() const
{
    return m_ready;
}

AssetRef EnvironmentalMap::GetCubemap() {
    return m_targetCubemap;
}
void EnvironmentalMap::ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap)
{
    m_targetCubemap = targetCubemap;
    m_gamma = targetCubemap->m_gamma;

    m_lightProbe = ProjectManager::CreateTemporaryAsset<LightProbe>();
    m_lightProbe.Get<LightProbe>()->ConstructFromCubemap(targetCubemap);
    m_reflectionProbe = ProjectManager::CreateTemporaryAsset<ReflectionProbe>();
    m_reflectionProbe.Get<ReflectionProbe>()->ConstructFromCubemap(targetCubemap);
    m_ready = true;
}

void EnvironmentalMap::OnInspect()
{
    static AssetRef targetTexture;
    ImGui::DragFloat("Gamma", &m_gamma);

    if(Editor::DragAndDropButton<Cubemap>(targetTexture, "Convert from cubemap")){
        auto tex = targetTexture.Get<Cubemap>();
        if(tex)
            ConstructFromCubemap(tex);
        targetTexture.Clear();
    }
}

AssetRegistration<EnvironmentalMap> EnvironmentalMapReg("EnvironmentalMap", {".ueenvirmap"});