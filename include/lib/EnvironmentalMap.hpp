#pragma once
#include <EntityManager.hpp>
#include <Cubemap.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API EnvironmentalMap : public ResourceBehaviour
{
    std::shared_ptr<Cubemap> m_targetCubemap;
    std::shared_ptr<Cubemap> m_irradianceMap;
    std::shared_ptr<Cubemap> m_preFilteredMap;
    std::shared_ptr<Texture2D> m_brdfLut;
    void PrepareIrradianceMap();
    void PreparePreFilteredMap();
    void PrepareBrdfLut();
  public:
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &cubemap);
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &cubemap, const std::shared_ptr<Cubemap> &irradianceMap);
};
}