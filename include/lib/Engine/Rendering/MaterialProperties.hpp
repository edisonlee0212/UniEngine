#pragma once
#include <glm/glm.hpp>
namespace UniEngine
{
    struct MaterialProperties{
        glm::vec3 m_albedoColor = glm::vec3(1.0f);
        glm::vec3 m_subsurfaceColor = glm::vec3(1.0f);
        float m_subsurfaceFactor = 0.0f;
        glm::vec3 m_subsurfaceRadius = glm::vec3(1.0f, 0.2f, 0.1f);
        float m_metallic = 0.3f;
        float m_specular = 0.5f;
        float m_specularTint = 0.0f;
        float m_roughness = 0.3f;
        float m_sheen = 0.0f;
        float m_sheenTint = 0.5f;
        float m_clearCoat = 0.0f;
        float m_clearCoatRoughness = 0.03f;
        float m_IOR = 1.45f;
        float m_transmission = 0.0f;
        float m_transmissionRoughness = 0.0f;
        float m_emission = 0.0f;
    };
}