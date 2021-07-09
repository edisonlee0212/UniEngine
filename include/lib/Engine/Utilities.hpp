#pragma once
#include <Entity.hpp>

namespace UniEngine
{
struct UNIENGINE_API Bound
{
    glm::vec3 m_min;
    glm::vec3 m_max;
    Bound();
    [[nodiscard]] glm::vec3 Size() const;
    [[nodiscard]] glm::vec3 Center() const;
    [[nodiscard]] bool InBound(const glm::vec3 &position) const;
    void ApplyTransform(const glm::mat4 &transform);
    void PopulateCorners(std::vector<glm::vec3> &corners) const;
};
struct UNIENGINE_API Ray : ComponentDataBase
{
    glm::vec3 m_start;
    glm::vec3 m_direction;
    float m_length;
    Ray() = default;
    Ray(glm::vec3 start, glm::vec3 end);
    Ray(glm::vec3 start, glm::vec3 direction, float length);
    [[nodiscard]] bool Intersect(const glm::vec3 &position, float radius) const;
    [[nodiscard]] bool Intersect(const glm::mat4 &transform, const Bound &bound) const;
    [[nodiscard]] glm::vec3 GetEnd() const;
};
struct UNIENGINE_API BezierCubic2D : ComponentDataBase
{
    bool m_fixed = true;
    glm::vec2 m_controlPoints[4];
    BezierCubic2D();
    [[nodiscard]] glm::vec2 GetPoint(const float &t) const;
    bool Graph(const std::string &label);
};
} // namespace UniEngine
