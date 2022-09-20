#pragma once
#include <uniengine_export.h>
#include <glm/glm.hpp>

namespace UniEngine
{
    struct UNIENGINE_API Vertex
    {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec3 m_tangent;
        glm::vec3 m_color = glm::vec3(1.0f);
        glm::vec2 m_texCoords = glm::vec2(0.0f);
    };
    struct UNIENGINE_API SkinnedVertex
    {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec3 m_tangent;
        glm::vec3 m_color;
        glm::vec2 m_texCoords;

        glm::ivec4 m_bondId;
        glm::vec4 m_weight;
        glm::ivec4 m_bondId2;
        glm::vec4 m_weight2;
    };
}