#pragma once

#include "Animator.hpp"
#include "OpenGLUtils.hpp"
#include "Scene.hpp"
#include "Transform.hpp"
#include "Particles.hpp"
#include "Vertex.hpp"
#include "RenderGeometry.hpp"
namespace UniEngine
{
class UNIENGINE_API SkinnedMesh : public IAsset, public RenderGeometry
{
    static std::unique_ptr<OpenGLUtils::GLBuffer> m_matricesBuffer;

    std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
    size_t m_offset = 0;

    unsigned m_mask = 0;
    Bound m_bound;
    friend class SkinnedMeshRenderer;
    friend class Particles;
    friend class Graphics;
    friend class RenderLayer;
    friend class Editor;
    size_t m_version = 0;
    std::vector<SkinnedVertex> m_skinnedVertices;
    std::vector<glm::uvec3> m_triangles;
    friend struct SkinnedMeshBonesBlock;
    static std::unique_ptr<OpenGLUtils::GLBuffer> m_skinnedMeshBonesUniformBufferBlock;

    //Don't serialize.
    std::vector<std::shared_ptr<Bone>> m_bones;
    friend class Prefab;
  protected:
    bool SaveInternal(const std::filesystem::path &path) override;
  public:
    void Draw() const override;
    void DrawInstanced(const std::vector<glm::mat4>& matrices) const override;
    void DrawInstanced(const std::vector<GlobalTransform>& matrices) const override;
    void DrawInstanced(const std::shared_ptr<ParticleMatrices>& matrices) const override;
    void OnCreate() override;
    void FetchIndices();

    //Need serialize
    std::vector<unsigned> m_boneAnimatorIndices;
    static void TryInitialize();
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