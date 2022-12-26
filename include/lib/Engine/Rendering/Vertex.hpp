#pragma once
#include <uniengine_export.h>
#include <glm/glm.hpp>

namespace UniEngine
{
    struct UNIENGINE_API Vertex
    {
        glm::vec3 m_position = glm::vec3(0.0f);
        float m_positionPadding = 0.0f;
        glm::vec3 m_normal = glm::vec3(0.0f);
        float m_normalPadding = 0.0f;
        glm::vec3 m_tangent = glm::vec3(0.0f);
        float m_tangentPadding = 0.0f;
        glm::vec4 m_color = glm::vec4(1.0f);
        glm::vec2 m_texCoord = glm::vec2(0.0f);
        glm::vec2 m_texCoordPadding = glm::vec2(0.0f);
    };
    struct UNIENGINE_API SkinnedVertex
    {
        glm::vec3 m_position = glm::vec3(0.0f);
        float m_positionPadding = 0.0f;
        glm::vec3 m_normal = glm::vec3(0.0f);
        float m_normalPadding = 0.0f;
        glm::vec3 m_tangent = glm::vec3(0.0f);
        float m_tangentPadding = 0.0f;
        glm::vec4 m_color = glm::vec4(1.0f);
        glm::vec2 m_texCoord = glm::vec2(0.0f);
        glm::vec2 m_texCoordPadding = glm::vec2(0.0f);

        glm::ivec4 m_bondId;
        glm::vec4 m_weight;
        glm::ivec4 m_bondId2;
        glm::vec4 m_weight2;
    };

    struct UNIENGINE_API StrandPoint
    {
        glm::vec3 m_position;
        float m_thickness = 0.1f;
        glm::vec3 m_color = glm::vec3(1.0f);
        float m_alpha = 1.0f;
        glm::vec2 m_texCoord = glm::vec2(0.0f);


    };
}