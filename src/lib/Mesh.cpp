#include <Core/Debug.hpp>
#include <Gui.hpp>
#include <Mesh.hpp>
using namespace UniEngine;

std::unique_ptr<OpenGLUtils::GLVBO> Mesh::m_matricesBuffer;

void Mesh::OnGui()
{
    ImGui::Text(("Name: " + m_name).c_str());
    ImGui::Text(("Vertices size: " + std::to_string(m_vertices.size())).c_str());
    ImGui::Text(("Triangle amount: " + std::to_string(m_triangles.size())).c_str());
    if (ImGui::BeginPopupContextItem(m_name.c_str()))
    {
        if (ImGui::BeginMenu("Rename"))
        {
            static char newName[256];
            ImGui::InputText("New name", newName, 256);
            if (ImGui::Button("Confirm"))
                m_name = std::string(newName);
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

glm::vec3 Mesh::GetCenter() const
{
    return m_bound.Center();
}
Bound Mesh::GetBound() const
{
    return m_bound;
}

void Mesh::OnCreate()
{
    m_vao = std::make_shared<OpenGLUtils::GLVAO>();
    m_bound = Bound();
    m_name = "New mesh";
}

void Mesh::Upload()
{
    if (m_vertices.empty())
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
    m_vao->SetData((GLsizei)(m_vertices.size() * sizeof(Vertex)), nullptr, GL_STATIC_DRAW);
    m_vao->SubData(0, m_vertices.size() * sizeof(Vertex), m_vertices.data());
#pragma endregion

#pragma region AttributePointer
    m_vao->EnableAttributeArray(0);
    m_vao->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    m_vao->EnableAttributeArray(1);
    m_vao->SetAttributePointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, m_normal)));
    m_vao->EnableAttributeArray(2);
    m_vao->SetAttributePointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, m_tangent)));
    m_vao->EnableAttributeArray(3);
    m_vao->SetAttributePointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, m_color)));

    m_vao->EnableAttributeArray(4);
    m_vao->SetAttributePointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, m_texCoords)));
#pragma endregion
    m_vao->Ebo()->SetData((GLsizei)m_triangles.size() * sizeof(glm::uvec3), m_triangles.data(), GL_STATIC_DRAW);
    m_version++;
}

void Mesh::SetVertices(const unsigned &mask, std::vector<Vertex> &vertices, std::vector<unsigned> &indices)
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

void Mesh::SetVertices(const unsigned &mask, std::vector<Vertex> &vertices, std::vector<glm::uvec3> &triangles)
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
    m_vertices = vertices;
    m_triangles = triangles;
#pragma region Bound
    glm::vec3 minBound = m_vertices.at(0).m_position;
    glm::vec3 maxBound = m_vertices.at(0).m_position;
    for (size_t i = 0; i < m_vertices.size(); i++)
    {
        minBound = glm::vec3(
            (glm::min)(minBound.x, m_vertices[i].m_position.x),
            (glm::min)(minBound.y, m_vertices[i].m_position.y),
            (glm::min)(minBound.z, m_vertices[i].m_position.z));
        maxBound = glm::vec3(
            (glm::max)(maxBound.x, m_vertices[i].m_position.x),
            (glm::max)(maxBound.y, m_vertices[i].m_position.y),
            (glm::max)(maxBound.z, m_vertices[i].m_position.z));
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

size_t Mesh::GetVerticesAmount() const
{
    return m_vertices.size();
}

size_t Mesh::GetTriangleAmount() const
{
    return m_triangles.size();
}

void Mesh::RecalculateNormal()
{
    std::vector<std::vector<glm::vec3>> normalLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_vertices.size();
    for (auto i = 0; i < size; i++)
    {
        normalLists.push_back(std::vector<glm::vec3>());
    }
    for (size_t i = 0; i < m_triangles.size(); i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto v1 = m_vertices[i1].m_position;
        auto v2 = m_vertices[i2].m_position;
        auto v3 = m_vertices[i3].m_position;
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
        m_vertices[i].m_normal = glm::normalize(normal);
    }
}

void Mesh::RecalculateTangent()
{
    auto tangentLists = std::vector<std::vector<glm::vec3>>();
    auto size = m_vertices.size();
    for (auto i = 0; i < size; i++)
    {
        tangentLists.emplace_back();
    }
    for (size_t i = 0; i < m_triangles.size(); i++)
    {
        const auto i1 = m_triangles[i].x;
        const auto i2 = m_triangles[i].y;
        const auto i3 = m_triangles[i].z;
        auto p1 = m_vertices[i1].m_position;
        auto p2 = m_vertices[i2].m_position;
        auto p3 = m_vertices[i3].m_position;
        auto uv1 = m_vertices[i1].m_texCoords;
        auto uv2 = m_vertices[i2].m_texCoords;
        auto uv3 = m_vertices[i3].m_texCoords;

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
        m_vertices[i].m_tangent = glm::normalize(tangent);
    }
}

std::shared_ptr<OpenGLUtils::GLVAO> Mesh::Vao() const
{
    return m_vao;
}

void Mesh::Enable() const
{
    m_vao->Bind();
}

size_t &Mesh::GetVersion()
{
    return m_version;
}

std::vector<glm::uvec3> &Mesh::UnsafeGetTriangles()
{
    return m_triangles;
}
std::vector<Vertex> &Mesh::UnsafeGetVertices()
{
    return m_vertices;
}
void Mesh::Draw() const
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
void Mesh::DrawInstanced(const std::vector<glm::mat4>& matrices) const
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
void Mesh::DrawInstanced(const std::vector<GlobalTransform> &matrices) const
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
