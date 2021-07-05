#pragma once
#include <Cubemap.hpp>
#include <LightProbe.hpp>
#include <ReflectionProbe.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public ResourceBehaviour
{
    friend class RenderManager;
    friend class DefaultResources;

    std::shared_ptr<Cubemap> m_skybox;
    std::shared_ptr<LightProbe> m_lightProbe;
    std::shared_ptr<ReflectionProbe> m_reflectionProbe;
    bool m_ready = false;
  public:
    bool IsReady() const;
    void OnCreate() override;
    void Construct(const std::shared_ptr<Cubemap> &targetCubemap);
    void Construct(const std::shared_ptr<Cubemap> &targetSkybox, const std::shared_ptr<Cubemap> &targetCubemapForProbes);
    void Construct(
        const std::shared_ptr<Cubemap> &targetCubemap,
        const std::shared_ptr<LightProbe> &targetlightProbe,
        const std::shared_ptr<ReflectionProbe> &targetReflectionProbe);
};
} // namespace UniEngine