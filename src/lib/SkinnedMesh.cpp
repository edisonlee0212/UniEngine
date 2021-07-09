#include <Core/Debug.hpp>
#include <Gui.hpp>
#include <Mesh.hpp>
#include <SkinnedMesh.hpp>
using namespace UniEngine;

std::unique_ptr<OpenGLUtils::GLSSBO> SkinnedMesh::m_skinnedMeshBonesUniformBufferBlock;

void SkinnedMesh::OnGui() const
{
    ImGui::Text(("Vertices size: " + std::to_string(m_verticesSize)).c_str());
    ImGui::Text(("Triangle amount: " + std::to_string(m_triangleSize)).c_str());
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
    m_triangleSize = 0;
    m_bound = Bound();
    m_name = "New Skinned";
}
void SkinnedMesh::Upload()
{
    if (m_verticesSize == 0)
    {
        UNIENGINE_ERROR("Vertices empty!")
        return;
    }
    if (m_triangleSize == 0)
    {
        UNIENGINE_ERROR("Triangles empty!")
        return;
    }
    if (m_positions.size() != m_verticesSize || m_normals.size() != m_verticesSize ||
        m_tangents.size() != m_verticesSize || m_colors.size() != m_verticesSize ||
        m_texCoords.size() != m_verticesSize || m_triangles.size() != m_triangleSize)
    {
        UNIENGINE_ERROR("Data corrupted!")
        return;
    }
#pragma region DataAllocation
    size_t attributeSize =
        3 * sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2) + 2 * (sizeof(glm::ivec4) + sizeof(glm::vec4));
    m_vao->SetData((GLsizei)(m_verticesSize * attributeSize), nullptr, GL_STATIC_DRAW);
#pragma endregion
#pragma region ToGPU
    m_vao->SubData(0, m_verticesSize * sizeof(glm::vec3), &m_positions[0]);
    m_vao->SubData(m_verticesSize * sizeof(glm::vec3), m_verticesSize * sizeof(glm::vec3), &m_normals[0]);
    m_vao->SubData(m_verticesSize * 2 * sizeof(glm::vec3), m_verticesSize * sizeof(glm::vec3), &m_tangents[0]);
    attributeSize = 3 * sizeof(glm::vec3);
    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::vec4), &m_colors[0]);
    attributeSize += sizeof(glm::vec4);
    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::vec2), &m_texCoords[0]);
    attributeSize += sizeof(glm::vec2);

    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::ivec4), &m_boneIds[0]);
    attributeSize += sizeof(glm::ivec4);
    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::vec4), &m_weights[0]);
    attributeSize += sizeof(glm::vec4);

    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::ivec4), &m_boneId2s[0]);
    attributeSize += sizeof(glm::ivec4);
    m_vao->SubData(m_verticesSize * attributeSize, m_verticesSize * sizeof(glm::vec4), &m_weight2s[0]);
    attributeSize += sizeof(glm::vec4);
#pragma endregion
#pragma region AttributePointer
    m_vao->EnableAttributeArray(0);
    m_vao->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    m_vao->EnableAttributeArray(1);
    m_vao->SetAttributePointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)(sizeof(glm::vec3) * m_verticesSize));
    m_vao->EnableAttributeArray(2);
    m_vao->SetAttributePointer(
        2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)(2 * sizeof(glm::vec3) * m_verticesSize));
    attributeSize = 3 * sizeof(glm::vec3);

    m_vao->EnableAttributeArray(3);
    m_vao->SetAttributePointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::vec4);

    m_vao->EnableAttributeArray(4);
    m_vao->SetAttributePointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::vec2);

    m_vao->EnableAttributeArray(5);
    m_vao->SetAttributeIntPointer(5, 4, GL_INT, sizeof(glm::ivec4), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::ivec4);

    m_vao->EnableAttributeArray(6);
    m_vao->SetAttributePointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::vec4);

    m_vao->EnableAttributeArray(7);
    m_vao->SetAttributeIntPointer(7, 4, GL_INT, sizeof(glm::ivec4), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::ivec4);

    m_vao->EnableAttributeArray(8);
    m_vao->SetAttributePointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)(attributeSize * m_verticesSize));
    attributeSize += sizeof(glm::vec4);
#pragma endregion
    m_vao->Ebo()->SetData((GLsizei)m_triangleSize * sizeof(glm::uvec3), m_triangles.data(), GL_STATIC_DRAW);
    m_version++;
}

void SkinnedMesh::SetVertices(
    const unsigned &mask, std::vector<SkinnedVertex> &vertices, std::vector<unsigned> &indices)
{
    if (indices.size() % 3 != 0)
    {
        UNIENGINE_ERROR("Triangle size wrong!");
        return;
    }
    std::vector<glm::uvec3> triangles;
    triangles.resize(indices.size() / 3);
    memcpy(triangles.data(), indices.data(), indices.size() * sizeof(unsigned));
    SetVertices(mask, vertices, triangles);
}

void SkinnedMesh::SetVertices(
    const unsigned &mask, std::vector<SkinnedVertex> &vertices, std::vector<glm::uvec3> &triangles)
{
    if (vertices.empty() || triangles.empty())
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
    m_triangleSize = triangles.size();
    m_verticesSize = vertices.size();
    m_positions.resize(vertices.size());
    m_normals.resize(vertices.size());
    m_tangents.resize(vertices.size());
    m_colors.resize(vertices.size());
    m_texCoords.resize(vertices.size());
    m_boneIds.resize(vertices.size());
    m_weights.resize(vertices.size());
    m_boneId2s.resize(vertices.size());
    m_weight2s.resize(vertices.size());
#pragma region Copy
    glm::vec3 minBound = vertices.at(0).m_position;
    glm::vec3 maxBound = vertices.at(0).m_position;
    for (size_t i = 0; i < m_verticesSize; i++)
    {
        m_positions[i] = vertices.at(i).m_position;
        minBound = glm::vec3(
            (glm::min)(minBound.x, m_positions[i].x),
            (glm::min)(minBound.y, m_positions[i].y),
            (glm::min)(minBound.z, m_positions[i].z));
        maxBound = glm::vec3(
            (glm::max)(maxBound.x, m_positions[i].x),
            (glm::max)(maxBound.y, m_positions[i].y),
            (glm::max)(maxBound.z, m_positions[i].z));
        if (mask & (unsigned)VertexAttribute::Normal)
        {
            m_normals[i] = vertices.at(i).m_normal;
        }
        if (mask & (unsigned)VertexAttribute::Tangent)
        {
            m_tangents[i] = vertices.at(i).m_tangent;
        }
        if (mask & (unsigned)VertexAttribute::Color)
        {
            m_colors[i] = vertices.at(i).m_color;
        }
        else
        {
            m_colors[i] = glm::vec4(1.0f);
        }
        if (mask & (unsigned)VertexAttribute::TexCoord)
        {
            m_texCoords[i] = vertices.at(i).m_texCoords;
        }
        else
        {
            m_texCoords[i] = glm::vec2(0.0f);
        }
        m_boneIds[i] = vertices[i].m_bondId;
        m_weights[i] = vertices[i].m_weight;
        m_boneId2s[i] = vertices[i].m_bondId2;
        m_weight2s[i] = vertices[i].m_weight2;
    }
    m_bound.m_max = maxBound;
    m_bound.m_min = minBound;
    m_triangles.clear();
    m_triangles.insert(m_triangles.begin(), triangles.begin(), triangles.end());

#pragma endregion
    if (!(m_mask & (unsigned)VertexAttribute::Normal))
        RecalculateNormal();
    if (!(m_mask & (unsigned)VertexAttribute::Tangent))
        RecalculateTangent();
    Upload();
}

size_t SkinnedMesh::GetVerticesAmount() const
{
    return m_verticesSize;
}

size_t SkinnedMesh::GetTriangleAmount() const
{
    return m_triangleSize;
}

void SkinnedMesh::RecalculateNormal()
{
    std::vector<std::vector<glm::vec3>> normalLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_positions.size();
    for (auto i = 0; i < size; i++)
    {
        normalLists.push_back(std::vector<glm::vec3>());
    }
    for (size_t i = 0; i < m_triangleSize; i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto v1 = m_positions[i1];
        auto v2 = m_positions[i2];
        auto v3 = m_positions[i3];
        auto normal = glm::normalize(glm::cross(v1 - v2, v1 - v3));
        normalLists[i1].push_back(normal);
        normalLists[i2].push_back(normal);
        normalLists[i3].push_back(normal);
    }
    std::vector<glm::vec3> normals = std::vector<glm::vec3>();
    for (auto i = 0; i < size; i++)
    {
        auto normal = glm::vec3(0.0f);
        for (auto j : normalLists[i])
        {
            normal += j;
        }
        normals.push_back(glm::normalize(normal));
        m_normals[i] = normal;
    }
}

void SkinnedMesh::RecalculateTangent()
{
    auto tangentLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_positions.size();
    for (auto i = 0; i < size; i++)
    {
        tangentLists.emplace_back();
    }
    for (size_t i = 0; i < m_triangleSize; i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto p1 = m_positions[i1];
        auto p2 = m_positions[i2];
        auto p3 = m_positions[i3];
        auto uv1 = m_texCoords[i1];
        auto uv2 = m_texCoords[i2];
        auto uv3 = m_texCoords[i3];

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
    auto tangents = std::vector<glm::vec3>();
    for (auto i = 0; i < size; i++)
    {
        auto tangent = glm::vec3(0.0f);
        for (auto j : tangentLists[i])
        {
            tangent += j;
        }
        tangents.push_back(glm::normalize(tangent));
        m_tangents[i] = tangent;
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

bool SkinnedMesh::HasVertexColors()
{
    return m_mask & (unsigned)VertexAttribute::Color;
}

std::vector<glm::vec3> &SkinnedMesh::UnsafeGetVertexPositions()
{
    return m_positions;
}

std::vector<glm::vec3> &SkinnedMesh::UnsafeGetVertexNormals()
{
    return m_normals;
}

std::vector<glm::vec3> &SkinnedMesh::UnsafeGetVertexTangents()
{
    return m_tangents;
}

std::vector<glm::vec4> &SkinnedMesh::UnsafeGetVertexColors()
{
    return m_colors;
}

std::vector<glm::vec2> &SkinnedMesh::UnsafeGetVertexTexCoords()
{
    return m_texCoords;
}

std::vector<glm::ivec4> &SkinnedMesh::UnsafeGetVertexBoneIds()
{
    return m_boneIds;
}

std::vector<glm::vec4> &SkinnedMesh::UnsafeGetVertexWeights()
{
    return m_weights;
}

size_t &SkinnedMesh::GetVersion()
{
    return m_version;
}

std::vector<glm::uvec3> &SkinnedMesh::UnsafeGetTriangles()
{
    return m_triangles;
}
