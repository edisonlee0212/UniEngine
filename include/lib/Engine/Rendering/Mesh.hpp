#pragma once
#include <OpenGLUtils.hpp>
#include <ResourceBehaviour.hpp>
#include <World.hpp>
#include <uniengine_export.h>

namespace UniEngine
{
struct UNIENGINE_API Vertex
{
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec3 m_tangent;
    glm::vec4 m_color;
    glm::vec2 m_texCoords;
};
enum class UNIENGINE_API VertexAttribute
{
    Position = 1,
    Normal = 1 << 1, // 2
    Tangent = 1 << 2,
    Color = 1 << 3,     // 8
    TexCoord = 1 << 4, // 16
};

class UNIENGINE_API Mesh : public ResourceBehaviour
{
    std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
    size_t m_verticesSize = 0;
    size_t m_triangleSize = 0;
    unsigned m_mask = 0;
    Bound m_bound;
    friend class MeshRenderer;
    friend class Particles;
    friend class EditorManager;
    size_t m_version = 0;
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec4> m_colors;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::uvec3> m_triangles;
  public:
    void OnGui();
    [[nodiscard]] glm::vec3 GetCenter() const;
    [[nodiscard]] Bound GetBound() const;
    void OnCreate() override;
    void Upload();
    void SetVertices(
        const unsigned &mask, std::vector<Vertex> &vertices, std::vector<unsigned> &indices);
    void SetVertices(
        const unsigned &mask,
        std::vector<Vertex> &vertices,
        std::vector<glm::uvec3> &triangles);
    [[nodiscard]] size_t GetVerticesAmount() const;
    [[nodiscard]] size_t GetTriangleAmount() const;
    void RecalculateNormal();
    void RecalculateTangent();
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLVAO> Vao() const;
    void Enable() const;
    [[nodiscard]] bool HasVertexColors();
    [[nodiscard]] std::vector<glm::vec3> &UnsafeGetVertexPositions();
    [[nodiscard]] std::vector<glm::vec3> &UnsafeGetVertexNormals();
    [[nodiscard]] std::vector<glm::vec3> &UnsafeGetVertexTangents();
    [[nodiscard]] std::vector<glm::vec4> &UnsafeGetVertexColors();
    [[nodiscard]] std::vector<glm::vec2> &UnsafeGetVertexTexCoords();
    [[nodiscard]] size_t &GetVersion();
    [[nodiscard]] std::vector<glm::uvec3> &UnsafeGetTriangles();
};
} // namespace UniEngine