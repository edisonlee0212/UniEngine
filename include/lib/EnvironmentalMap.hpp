#pragma once
#include <EntityManager.hpp>
#include <Cubemap.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public ResourceBehaviour
{
    friend class RenderManager;
    friend class DefaultResources;
    void PrepareIrradianceMap();
    void PreparePreFilteredMap();
    std::shared_ptr<Cubemap> m_targetCubemap;
    std::unique_ptr<Cubemap> m_irradianceMap;
    std::unique_ptr<Cubemap> m_preFilteredMap;
    
    size_t m_irradianceMapResolution = 32;
    size_t m_preFilteredMapResolution = 128;
    bool m_ready = false;
  public:
    static void RenderCube();
    static void RenderQuad();
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &cubemap);
};
}