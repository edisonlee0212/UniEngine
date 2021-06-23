#pragma once
#include <EntityManager.hpp>
#include <Cubemap.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public ResourceBehaviour
{
    friend class RenderManager;
    void PrepareIrradianceMap();
    void PreparePreFilteredMap();
    void PrepareBrdfLut();
    std::shared_ptr<Cubemap> m_targetCubemap;
    std::unique_ptr<Cubemap> m_irradianceMap;
    std::unique_ptr<Cubemap> m_preFilteredMap;
    std::unique_ptr<Texture2D> m_brdfLut;
    size_t m_irradianceMapResolution = 32;
    size_t m_preFilteredMapResolution = 128;
    bool m_ready = false;
  public:
    static void RenderCube();
    static void RenderQuad();
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &cubemap);
};
}