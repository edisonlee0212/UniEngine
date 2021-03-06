#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include <EntityManager.hpp>
namespace UniEngine
{
class UNIENGINE_API ReflectionProbe : public IAsset
{
    friend class RenderManager;
    friend class DefaultResources;

    std::unique_ptr<Cubemap> m_preFilteredMap;
    size_t m_preFilteredMapResolution = 512;
    bool m_ready = false;

  public:
    float m_gamma = 1.0f;
    void OnCreate() override;
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap);
};
} // namespace UniEngine