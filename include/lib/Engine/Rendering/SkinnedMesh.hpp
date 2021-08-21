#pragma once
#include "DefaultResources.hpp"

#include <Animator.hpp>
#include <Core/OpenGLUtils.hpp>
#include <Scene.hpp>
#include <Transform.hpp>
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

class UNIENGINE_API SkinnedMesh : public IAsset
{
    static std::unique_ptr<OpenGLUtils::GLVBO> m_matricesBuffer;

    std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
    size_t m_offset = 0;

    unsigned m_mask = 0;
    Bound m_bound;
    friend class SkinnedMeshRenderer;
    friend class Particles;
    friend class RenderManager;
    friend class EditorManager;
    size_t m_version = 0;
    std::vector<SkinnedVertex> m_skinnedVertices;
    std::vector<glm::uvec3> m_triangles;
    friend class AssetManager;
    friend struct SkinnedMeshBonesBlock;
    static std::unique_ptr<OpenGLUtils::GLSSBO> m_skinnedMeshBonesUniformBufferBlock;

    //Don't serialize.
    std::vector<std::shared_ptr<Bone>> m_bones;
    friend class Prefab;
  public:
    void Draw() const;
    void DrawInstanced(const std::vector<glm::mat4>& matrices) const;
    void DrawInstanced(const std::vector<GlobalTransform>& matrices) const;

    void OnCreate() override;
    AssetRef m_animation;
    void FetchIndices();

    //Need serialize
    std::vector<unsigned> m_boneAnimatorIndices;
    static void GenerateMatrices();
    static void UploadBones(const std::vector<glm::mat4> &matrices);
    void OnInspect() override;
    [[nodiscard]] glm::vec3 GetCenter() const;
    [[nodiscard]] Bound GetBound() const;
    void Upload();
    void SetVertices(const unsigned &mask, std::vector<SkinnedVertex> &skinnedVertices, std::vector<unsigned> &indices);
    void SetVertices(const unsigned &mask, std::vector<SkinnedVertex> &skinnedVertices, std::vector<glm::uvec3> &triangles);
    [[nodiscard]] size_t GetSkinnedVerticesAmount() const;
    [[nodiscard]] size_t GetTriangleAmount() const;
    void RecalculateNormal();
    void RecalculateTangent();
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLVAO> Vao() const;
    void Enable() const;
    [[nodiscard]] size_t &GetVersion();
    [[nodiscard]] std::vector<SkinnedVertex> &UnsafeGetSkinnedVertices();
    [[nodiscard]] std::vector<glm::uvec3> &UnsafeGetTriangles();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine