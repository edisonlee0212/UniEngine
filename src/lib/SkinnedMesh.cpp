#include <Core/Debug.hpp>
#include <Gui.hpp>
#include <Mesh.hpp>
#include <SkinnedMesh.hpp>
using namespace UniEngine;


std::unique_ptr<OpenGLUtils::GLVBO> SkinnedMesh::m_matricesBuffer;

std::unique_ptr<OpenGLUtils::GLSSBO> SkinnedMesh::m_skinnedMeshBonesUniformBufferBlock;

void SkinnedMesh::OnGui() const
{
    ImGui::Text(("Vertices size: " + std::to_string(m_skinnedVertices.size())).c_str());
    ImGui::Text(("Triangle amount: " + std::to_string(m_triangles.size())).c_str());
}

glm::vec3 SkinnedMesh::GetCenter() const
{
    return m_bound.Center();
}
Bound SkinnedMesh::GetBound() const
{
    return m_bound;
}

void SkinnedMesh::FetchIndices()
{
    m_boneAnimatorIndices.resize(m_bones.size());
    for (int i = 0; i < m_bones.size(); i++)
    {
        m_boneAnimatorIndices[i] = m_bones[i]->m_index;
    }
}

void SkinnedMesh::GenerateMatrices()
{
    m_skinnedMeshBonesUniformBufferBlock = std::make_unique<OpenGLUtils::GLSSBO>();
    m_skinnedMeshBonesUniformBufferBlock->SetData(
        DefaultResources::ShaderIncludes::MaxBonesAmount * sizeof(glm::mat4), nullptr, GL_STREAM_DRAW);
    m_skinnedMeshBonesUniformBufferBlock->SetBase(8);
}

void SkinnedMesh::UploadBones(std::vector<glm::mat4> &matrices)
{
    m_skinnedMeshBonesUniformBufferBlock->SubData(0, matrices.size() * sizeof(glm::mat4), matrices.data());
}

void SkinnedMesh::OnCreate()
{
    m_vao = std::make_shared<OpenGLUtils::GLVAO>();
    m_bound = Bound();
    m_name = "New Skinned";
}
void SkinnedMesh::Upload()
{
    if (m_skinnedVertices.empty())
    {
        UNIENGINE_ERROR("Vertices empty!")
        return;
    }
    if (m_triangles.empty())
    {
        UNIENGINE_ERROR("Triangles empty!")
        return;
    }

#pragma region Data
    m_vao->SetData((GLsizei)(m_skinnedVertices.size() * sizeof(SkinnedVertex)), nullptr, GL_STATIC_DRAW);
    m_vao->SubData(0, m_skinnedVertices.size() * sizeof(SkinnedVertex), m_skinnedVertices.data());
#pragma endregion
#pragma region AttributePointer
    m_vao->EnableAttributeArray(0);
    m_vao->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), 0);
    m_vao->EnableAttributeArray(1);
    m_vao->SetAttributePointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_normal)));
    m_vao->EnableAttributeArray(2);
    m_vao->SetAttributePointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_tangent)));
    m_vao->EnableAttributeArray(3);
    m_vao->SetAttributePointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_color)));
    m_vao->EnableAttributeArray(4);
    m_vao->SetAttributePointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_texCoords)));

    m_vao->EnableAttributeArray(5);
    m_vao->SetAttributeIntPointer(5, 4, GL_INT, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_bondId)));
    m_vao->EnableAttributeArray(6);
    m_vao->SetAttributePointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_weight)));
    m_vao->EnableAttributeArray(7);
    m_vao->SetAttributeIntPointer(7, 4, GL_INT, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_bondId2)));
    m_vao->EnableAttributeArray(8);
    m_vao->SetAttributePointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_weight2)));
#pragma endregion
    m_vao->Ebo()->SetData((GLsizei)m_triangles.size() * sizeof(glm::uvec3), m_triangles.data(), GL_STATIC_DRAW);
    m_version++;
}

void SkinnedMesh::SetVertices(
    const unsigned &mask, std::vector<SkinnedVertex> &skinnedVertices, std::vector<unsigned> &indices)
{
    if (indices.size() % 3 != 0)
    {
        UNIENGINE_ERROR("Triangle size wrong!");
        return;
    }
    std::vector<glm::uvec3> triangles;
    triangles.resize(indices.size() / 3);
    memcpy(triangles.data(), indices.data(), indices.size() * sizeof(unsigned));
    SetVertices(mask, skinnedVertices, triangles);
}

void SkinnedMesh::SetVertices(
    const unsigned &mask, std::vector<SkinnedVertex> &skinnedVertices, std::vector<glm::uvec3> &triangles)
{
    if (skinnedVertices.empty() || triangles.empty())
    {
        UNIENGINE_LOG("Vertices or triangles empty!");
        return;
    }
    if (!(mask & (unsigned)VertexAttribute::Position))
    {
        UNIENGINE_ERROR("No Position Data!");
        return;
    }
    m_mask = mask;
    m_skinnedVertices = skinnedVertices;
    m_triangles = triangles;
#pragma region Bound
    glm::vec3 minBound = m_skinnedVertices.at(0).m_position;
    glm::vec3 maxBound = m_skinnedVertices.at(0).m_position;
    for (size_t i = 0; i < m_skinnedVertices.size(); i++)
    {
        minBound = glm::vec3(
            (glm::min)(minBound.x, m_skinnedVertices[i].m_position.x),
            (glm::min)(minBound.y, m_skinnedVertices[i].m_position.y),
            (glm::min)(minBound.z, m_skinnedVertices[i].m_position.z));
        maxBound = glm::vec3(
            (glm::max)(maxBound.x, m_skinnedVertices[i].m_position.x),
            (glm::max)(maxBound.y, m_skinnedVertices[i].m_position.y),
            (glm::max)(maxBound.z, m_skinnedVertices[i].m_position.z));
    }
    m_bound.m_max = maxBound;
    m_bound.m_min = minBound;
#pragma endregion
    if (!(m_mask & (unsigned)VertexAttribute::Normal))
        RecalculateNormal();
    if (!(m_mask & (unsigned)VertexAttribute::Tangent))
        RecalculateTangent();
    Upload();
}

size_t SkinnedMesh::GetSkinnedVerticesAmount() const
{
    return m_skinnedVertices.size();
}

size_t SkinnedMesh::GetTriangleAmount() const
{
    return m_triangles.size();
}

void SkinnedMesh::RecalculateNormal()
{
    std::vector<std::vector<glm::vec3>> normalLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_skinnedVertices.size();
    for (auto i = 0; i < size; i++)
    {
        normalLists.push_back(std::vector<glm::vec3>());
    }
    for (size_t i = 0; i < m_triangles.size(); i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto v1 = m_skinnedVertices[i1].m_position;
        auto v2 = m_skinnedVertices[i2].m_position;
        auto v3 = m_skinnedVertices[i3].m_position;
        auto normal = glm::normalize(glm::cross(v1 - v2, v1 - v3));
        normalLists[i1].push_back(normal);
        normalLists[i2].push_back(normal);
        normalLists[i3].push_back(normal);
    }
    for (auto i = 0; i < size; i++)
    {
        auto normal = glm::vec3(0.0f);
        for (auto j : normalLists[i])
        {
            normal += j;
        }
        m_skinnedVertices[i].m_normal = glm::normalize(normal);
    }
}

void SkinnedMesh::RecalculateTangent()
{
    auto tangentLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_skinnedVertices.size();
    for (auto i = 0; i < size; i++)
    {
        tangentLists.emplace_back();
    }
    for (size_t i = 0; i < m_triangles.size(); i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto p1 = m_skinnedVertices[i1].m_position;
        auto p2 = m_skinnedVertices[i2].m_position;
        auto p3 = m_skinnedVertices[i3].m_position;
        auto uv1 = m_skinnedVertices[i1].m_texCoords;
        auto uv2 = m_skinnedVertices[i2].m_texCoords;
        auto uv3 = m_skinnedVertices[i3].m_texCoords;

        auto e21 = p2 - p1;
        auto d21 = uv2 - uv1;
        auto e31 = p3 - p1;
        auto d31 = uv3 - uv1;
        float f = 1.0f / (d21.x * d31.y - d31.x * d21.y);
        auto tangent =
            f * glm::vec3(d31.y * e21.x - d21.y * e31.x, d31.y * e21.y - d21.y * e31.y, d31.y * e21.z - d21.y * e31.z);
        tangentLists[i1].push_back(tangent);
        tangentLists[i2].push_back(tangent);
        tangentLists[i3].push_back(tangent);
    }
    for (auto i = 0; i < size; i++)
    {
        auto tangent = glm::vec3(0.0f);
        for (auto j : tangentLists[i])
        {
            tangent += j;
        }
        m_skinnedVertices[i].m_tangent = glm::normalize(tangent);
    }
}

std::shared_ptr<OpenGLUtils::GLVAO> SkinnedMesh::Vao() const
{
    return m_vao;
}

void SkinnedMesh::Enable() const
{
    m_vao->Bind();
}

size_t &SkinnedMesh::GetVersion()
{
    return m_version;
}

std::vector<glm::uvec3> &SkinnedMesh::UnsafeGetTriangles()
{
    return m_triangles;
}
std::vector<SkinnedVertex> &SkinnedMesh::UnsafeGetSkinnedVertices()
{
    return m_skinnedVertices;
}

void SkinnedMesh::Draw() const
{
    m_vao->Bind();
    m_vao->DisableAttributeArray(12);
    m_vao->DisableAttributeArray(13);
    m_vao->DisableAttributeArray(14);
    m_vao->DisableAttributeArray(15);

    glDrawElements(
        GL_TRIANGLES, (GLsizei)m_triangles.size() * 3, GL_UNSIGNED_INT, (GLvoid *)(sizeof(GLuint) * m_offset));
    OpenGLUtils::GLVAO::BindDefault();
}
void SkinnedMesh::DrawInstanced(const std::vector<glm::mat4>& matrices) const
{
    auto count = matrices.size();
    m_matricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices.data(), GL_DYNAMIC_DRAW);
    m_vao->Bind();
    m_vao->EnableAttributeArray(12);
    m_vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    m_vao->EnableAttributeArray(13);
    m_vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    m_vao->EnableAttributeArray(14);
    m_vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    m_vao->EnableAttributeArray(15);
    m_vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    m_vao->SetAttributeDivisor(12, 1);
    m_vao->SetAttributeDivisor(13, 1);
    m_vao->SetAttributeDivisor(14, 1);
    m_vao->SetAttributeDivisor(15, 1);

    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)m_triangles.size() * 3, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());
}
void SkinnedMesh::DrawInstanced(const std::vector<GlobalTransform> &matrices) const
{
    auto count = matrices.size();
    m_matricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices.data(), GL_DYNAMIC_DRAW);
    m_vao->Bind();
    m_vao->EnableAttributeArray(12);
    m_vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    m_vao->EnableAttributeArray(13);
    m_vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    m_vao->EnableAttributeArray(14);
    m_vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    m_vao->EnableAttributeArray(15);
    m_vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    m_vao->SetAttributeDivisor(12, 1);
    m_vao->SetAttributeDivisor(13, 1);
    m_vao->SetAttributeDivisor(14, 1);
    m_vao->SetAttributeDivisor(15, 1);

    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)m_triangles.size() * 3, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());
}
