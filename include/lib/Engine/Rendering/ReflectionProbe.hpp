#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Cubemap.hpp>
#include "Engine/ECS/Entities.hpp"
namespace UniEngine
{
class UNIENGINE_API ReflectionProbe : public IAsset
{
    friend class Graphics;
    friend class DefaultResources;
    friend class RenderLayer;
    std::unique_ptr<Cubemap> m_preFilteredMap;
    size_t m_preFilteredMapResolution = 512;
    bool m_ready = false;

  public:
    float m_gamma = 1.0f;
    void ConstructFromCubemap(const std::shared_ptr<Cubemap> &targetCubemap);
};
} // namespace UniEngine