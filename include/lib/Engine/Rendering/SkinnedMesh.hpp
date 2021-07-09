#pragma once
#include "DefaultResources.hpp"

#include <Animator.hpp>
#include <Core/OpenGLUtils.hpp>
#include <World.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
struct UNIENGINE_API SkinnedVertex
{
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec3 m_tangent;
    glm::vec4 m_color;
    glm::vec2 m_texCoords;

    glm::ivec4 m_bondId;
    glm::vec4 m_weight;
    glm::ivec4 m_bondId2;
    glm::vec4 m_weight2;
};

class UNIENGINE_API SkinnedMesh : public ResourceBehaviour
{
    std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
    size_t m_verticesSize = 0;
    size_t m_triangleSize = 0;
    unsigned m_mask = 0;
    Bound m_bound;
    friend class SkinnedMeshRenderer;
    friend class Particles;
    friend class EditorManager;
    size_t m_version = 0;
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec4> m_colors;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::ivec4> m_boneIds;
    std::vector<glm::vec4> m_weights;
    std::vector<glm::ivec4> m_boneId2s;
    std::vector<glm::vec4> m_weight2s;

    std::vector<glm::uvec3> m_triangles;
    friend class ResourceManager;
    friend struct SkinnedMeshBonesBlock;
    static std::unique_ptr<OpenGLUtils::GLSSBO> m_skinnedMeshBonesUniformBufferBlock;
    std::vector<std::shared_ptr<Bone>> m_bones;

  public:
    void OnCreate() override;
    std::shared_ptr<Animation> m_animation;
    void FetchIndices();
    std::vector<unsigned> m_boneAnimatorIndices;
    static void GenerateMatrices();
    static void UploadBones(std::vector<glm::mat4> &matrices);
    void OnGui() const;
    [[nodiscard]] glm::vec3 GetCenter() const;
    [[nodiscard]] Bound GetBound() const;
    void Upload();
    void SetVertices(const unsigned &mask, std::vector<SkinnedVertex> &vertices, std::vector<unsigned> &indices);
    void SetVertices(const unsigned &mask, std::vector<SkinnedVertex> &vertices, std::vector<glm::uvec3> &triangles);
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
    [[nodiscard]] std::vector<glm::ivec4> &UnsafeGetVertexBoneIds();
    [[nodiscard]] std::vector<glm::vec4> &UnsafeGetVertexWeights();
    [[nodiscard]] size_t &GetVersion();
    [[nodiscard]] std::vector<glm::uvec3> &UnsafeGetTriangles();
};
} // namespace UniEngine