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
  public:
    float m_gamma = 1.0f;
    [[nodiscard]] bool IsReady() const;
    void OnCreate() override;
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap);
    void OnInspect() override;
};

} // namespace UniEngine