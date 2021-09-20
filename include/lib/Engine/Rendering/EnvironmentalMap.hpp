#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include <LightProbe.hpp>
#include <ReflectionProbe.hpp>
#include "Texture2D.hpp"

namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public IAsset
{
    friend class RenderManager;
    friend class DefaultResources;

    AssetRef m_targetCubemap;
    AssetRef m_lightProbe;
    AssetRef m_reflectionProbe;
    bool m_ready = false;

  protected:
    bool LoadInternal(const std::filesystem::path & path) override;
  public:
    float m_gamma = 1.0f;
    [[nodiscard]] bool IsReady() const;
    void OnCreate() override;
    void Construct(const std::shared_ptr<Cubemap> &targetCubemap);

};

} // namespace UniEngine