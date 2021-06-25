#pragma once
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API ReflectionProbe : public ResourceBehaviour
{
    friend class RenderManager;
    friend class DefaultResources;
    
    std::unique_ptr<Cubemap> m_preFilteredMap;
    size_t m_preFilteredMapResolution = 128;
    bool m_ready = false;
  public:
    void OnCreate() override;
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap);
};
} // namespace UniEngine