#pragma once
#include <Core/OpenGLUtils.hpp>
#include <IAsset.hpp>
#include <Scene.hpp>
#include <Transform.hpp>
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
    Color = 1 << 3,    // 8
    TexCoord = 1 << 4, // 16
};

enum class MeshStoreType{
    //Meshes with persistent type will be combined into a single mesh to improve performance, removing meshes will have a bigger penalty.
    Persistent,
    //Meshes here will have it's own VAO. It's relatively more lightweight to make changes here.
    Temporal
};

class UNIENGINE_API MeshStorage{
    std::unique_ptr<OpenGLUtils::GLVAO> m_persistentMeshesVAO;
};
class ParticleMatrices;
class UNIENGINE_API Mesh : public IAsset
{
    static std::unique_ptr<OpenGLUtils::GLVBO> m_matricesBuffer;

    std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
    size_t m_offset = 0;

    unsigned m_mask = 0;
    Bound m_bound;
    friend class RenderManager;
    friend class RenderLayer;
    friend class MeshRenderer;
    friend class Particles;
    friend class EditorManager;
    size_t m_version = 0;

    std::vector<Vertex> m_vertices;
    std::vector<glm::uvec3> m_triangles;

  protected:
    bool SaveInternal(const std::filesystem::path &path) override;

  public:
    void Draw() const;
    void DrawInstanced(const std::vector<glm::mat4>& matrices) const;
    void DrawInstanced(const std::shared_ptr<ParticleMatrices>& particleMatrices) const;
    void DrawInstanced(const std::vector<GlobalTransform>& matrices) const;

    void OnInspect() override;
    [[nodiscard]] glm::vec3 GetCenter() const;
    [[nodiscard]] Bound GetBound() const;
    void OnCreate() override;
    void Upload();
    void SetVertices(const unsigned &mask, std::vector<Vertex> &vertices, std::vector<unsigned> &indices);
    void SetVertices(const unsigned &mask, std::vector<Vertex> &vertices, std::vector<glm::uvec3> &triangles);
    [[nodiscard]] size_t GetVerticesAmount() const;
    [[nodiscard]] size_t GetTriangleAmount() const;
    void RecalculateNormal();
    void RecalculateTangent();
    [[nodiscard]] std::shared_ptr<OpenGLUtils::GLVAO> Vao() const;
    void Enable() const;
    [[nodiscard]] size_t &GetVersion();
    [[nodiscard]] std::vector<Vertex> &UnsafeGetVertices();
    [[nodiscard]] std::vector<glm::uvec3> &UnsafeGetTriangles();

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};
} // namespace UniEngine