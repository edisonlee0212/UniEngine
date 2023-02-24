#include "Engine/Utilities/Console.hpp"
#include "Editor.hpp"
#include <Mesh.hpp>
#include <Particles.hpp>
#include <SkinnedMesh.hpp>
#include "DefaultResources.hpp"
using namespace UniEngine;

std::unique_ptr<OpenGLUtils::GLBuffer> SkinnedMesh::m_matricesBuffer;
std::unique_ptr<OpenGLUtils::GLBuffer> SkinnedMesh::m_skinnedMeshBonesUniformBufferBlock;

void SkinnedMesh::OnInspect()
{
    ImGui::Text(("Vertices size: " + std::to_string(m_skinnedVertices.size())).c_str());
    ImGui::Text(("Triangle amount: " + std::to_string(m_triangles.size())).c_str());

    if(!m_skinnedVertices.empty()){
        FileUtils::SaveFile(
            "Export as OBJ",
            "Mesh",
            {".obj"},
            [&](const std::filesystem::path &path) { Export(path); },
            false);
    }
}
bool SkinnedMesh::SaveInternal(const std::filesystem::path &path)
{
    if(path.extension() == ".ueskinnedmesh"){
        return IAsset::SaveInternal(path);
    }else if(path.extension() == ".obj"){
        std::ofstream of;
        of.open(path.string(), std::ofstream::out | std::ofstream::trunc);
        if (of.is_open()) {
            std::string start = "#Mesh exporter, by Bosheng Li";
            start += "\n";
            of.write(start.c_str(), start.size());
            of.flush();
            unsigned startIndex = 1;
            if (!m_triangles.empty()) {
                std::string header =
                    "#Vertices: " + std::to_string(m_skinnedVertices.size()) +
                    ", tris: " + std::to_string(m_triangles.size());
                header += "\n";
                of.write(header.c_str(), header.size());
                of.flush();
                std::string data;
#pragma region Data collection
                for (auto i = 0; i < m_skinnedVertices.size(); i++) {
                    auto &vertexPosition = m_skinnedVertices.at(i).m_position;
                    auto &color = m_skinnedVertices.at(i).m_color;
                    data += "v " + std::to_string(vertexPosition.x) + " " +
                            std::to_string(vertexPosition.y) + " " +
                            std::to_string(vertexPosition.z) + " " +
                            std::to_string(color.x) + " " + std::to_string(color.y) + " " +
                            std::to_string(color.z) + "\n";
                }
                for (const auto &vertex : m_skinnedVertices) {
                    data += "vn " + std::to_string(vertex.m_normal.x) + " " +
                            std::to_string(vertex.m_normal.y) + " " +
                            std::to_string(vertex.m_normal.z) + "\n";
                }

                for (const auto &vertex : m_skinnedVertices) {
                    data += "vt " + std::to_string(vertex.m_texCoord.x) + " " +
                            std::to_string(vertex.m_texCoord.y) + "\n";
                }
                // data += "s off\n";
                data += "# List of indices for faces vertices, with (x, y, z).\n";
                auto &triangles = m_triangles;
                for (auto i = 0; i < m_triangles.size(); i++) {
                    const auto triangle = triangles[i];
                    const auto f1 = triangle.x + startIndex;
                    const auto f2 = triangle.y + startIndex;
                    const auto f3 = triangle.z + startIndex;
                    data += "f " + std::to_string(f1) + "/" + std::to_string(f1) + "/" +
                            std::to_string(f1) + " " + std::to_string(f2) + "/" +
                            std::to_string(f2) + "/" + std::to_string(f2) + " " +
                            std::to_string(f3) + "/" + std::to_string(f3) + "/" +
                            std::to_string(f3) + "\n";
                }
                startIndex += m_skinnedVertices.size();
#pragma endregion
                of.write(data.c_str(), data.size());
                of.flush();
            }
            of.close();
            return true;
        } else {
            UNIENGINE_ERROR("Can't open file!");
            return false;
        }
    }
    return false;
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

void SkinnedMesh::TryInitialize()
{
    m_skinnedMeshBonesUniformBufferBlock = std::make_unique<OpenGLUtils::GLBuffer>(OpenGLUtils::GLBufferTarget::ShaderStorage, 8);
    m_skinnedMeshBonesUniformBufferBlock->SetData(
        DefaultResources::ShaderIncludes::MaxBonesAmount * sizeof(glm::mat4), nullptr, GL_STREAM_DRAW);
    m_skinnedMeshBonesUniformBufferBlock->Bind();
}

void SkinnedMesh::UploadBones(const std::vector<glm::mat4> &matrices)
{
    m_skinnedMeshBonesUniformBufferBlock->SubData(0, matrices.size() * sizeof(glm::mat4), matrices.data());
}

void SkinnedMesh::OnCreate()
{
    TryInitialize();
    m_vao = std::make_shared<OpenGLUtils::GLVAO>();
    m_bound = Bound();
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
    m_vao->SetAttributePointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_normal)));
    m_vao->EnableAttributeArray(2);
    m_vao->SetAttributePointer(
        2, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_tangent)));
    m_vao->EnableAttributeArray(3);
    m_vao->SetAttributePointer(
        3, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_color)));
    m_vao->EnableAttributeArray(4);
    m_vao->SetAttributePointer(
        4, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_texCoord)));

    m_vao->EnableAttributeArray(5);
    m_vao->SetAttributeIntPointer(5, 4, GL_INT, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_bondId)));
    m_vao->EnableAttributeArray(6);
    m_vao->SetAttributePointer(
        6, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_weight)));
    m_vao->EnableAttributeArray(7);
    m_vao->SetAttributeIntPointer(7, 4, GL_INT, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_bondId2)));
    m_vao->EnableAttributeArray(8);
    m_vao->SetAttributePointer(
        8, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void *)(offsetof(SkinnedVertex, m_weight2)));
#pragma endregion
    m_vao->Ebo().SetData((GLsizei)m_triangles.size() * sizeof(glm::uvec3), m_triangles.data(), GL_STATIC_DRAW);
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
        auto uv1 = m_skinnedVertices[i1].m_texCoord;
        auto uv2 = m_skinnedVertices[i2].m_texCoord;
        auto uv3 = m_skinnedVertices[i3].m_texCoord;

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
void SkinnedMesh::DrawInstanced(const std::vector<glm::mat4> &matrices) const
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

    glDrawElementsInstanced(
        GL_TRIANGLES, (GLsizei)m_triangles.size() * 3, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());
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

    glDrawElementsInstanced(
        GL_TRIANGLES, (GLsizei)m_triangles.size() * 3, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());
}

void SkinnedMesh::DrawInstanced(const std::shared_ptr<ParticleMatrices> &matrices) const
{
    if(!matrices->m_bufferReady) return;

}


void SkinnedMesh::Serialize(YAML::Emitter &out)
{
    if (!m_boneAnimatorIndices.empty())
    {
        out << YAML::Key << "m_boneAnimatorIndices" << YAML::Value
            << YAML::Binary(
                   (const unsigned char *)m_boneAnimatorIndices.data(),
                   m_boneAnimatorIndices.size() * sizeof(unsigned));
    }
    out << YAML::Key << "m_mask" << YAML::Value << m_mask;
    out << YAML::Key << "m_offset" << YAML::Value << m_offset;
    out << YAML::Key << "m_version" << YAML::Value << m_version;

    if (!m_skinnedVertices.empty() && !m_triangles.empty())
    {
        out << YAML::Key << "m_skinnedVertices" << YAML::Value
            << YAML::Binary(
                   (const unsigned char *)m_skinnedVertices.data(), m_skinnedVertices.size() * sizeof(SkinnedVertex));
        out << YAML::Key << "m_triangles" << YAML::Value
            << YAML::Binary((const unsigned char *)m_triangles.data(), m_triangles.size() * sizeof(glm::uvec3));
    }
}
void SkinnedMesh::Deserialize(const YAML::Node &in)
{
    if(in["m_boneAnimatorIndices"])
    {
        YAML::Binary boneIndices = in["m_boneAnimatorIndices"].as<YAML::Binary>();
        m_boneAnimatorIndices.resize(boneIndices.size() / sizeof(unsigned));
        std::memcpy(m_boneAnimatorIndices.data(), boneIndices.data(), boneIndices.size());
    }
    m_mask = in["m_mask"].as<unsigned>();
    m_offset = in["m_offset"].as<size_t>();
    m_version = in["m_version"].as<size_t>();

    if(in["m_skinnedVertices"] && in["m_triangles"])
    {
        YAML::Binary skinnedVertexData = in["m_skinnedVertices"].as<YAML::Binary>();
        std::vector<SkinnedVertex> skinnedVertices;
        skinnedVertices.resize(skinnedVertexData.size() / sizeof(SkinnedVertex));
        std::memcpy(skinnedVertices.data(), skinnedVertexData.data(), skinnedVertexData.size());

        YAML::Binary triangleData = in["m_triangles"].as<YAML::Binary>();
        std::vector<glm::uvec3> triangles;
        triangles.resize(triangleData.size() / sizeof(glm::uvec3));
        std::memcpy(triangles.data(), triangleData.data(), triangleData.size());

        SetVertices(m_mask, skinnedVertices, triangles);
    }
}