#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include <LightProbe.hpp>
#include <ReflectionProbe.hpp>
namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public IAsset
{
    friend class RenderManager;
    friend class DefaultResources;

    std::shared_ptr<Cubemap> m_targetCubemap;
    std::shared_ptr<LightProbe> m_lightProbe;
    std::shared_ptr<ReflectionProbe> m_reflectionProbe;
    bool m_ready = false;

  public:
    float m_gamma = 1.0f;
    bool IsReady() const;
    void OnCreate() override;
    void Construct(const std::shared_ptr<Cubemap> &targetCubemap);
};
} // namespace UniEngine