
#include "RenderGeometry.hpp"

#include "OpenGLUtils.hpp"
using namespace UniEngine;
void ParticleMatrices::Serialize(YAML::Emitter &out)
{
    if (!m_matrices.empty())
    {
        out << YAML::Key << "m_matrices" << YAML::Value
            << YAML::Binary((const unsigned char *)m_matrices.data(), m_matrices.size() * sizeof(glm::mat4));
        out << YAML::Key << "m_colors" << YAML::Value
            << YAML::Binary((const unsigned char*)m_colors.data(), m_colors.size() * sizeof(glm::vec4));
    }
}
void ParticleMatrices::Deserialize(const YAML::Node &in)
{
    if (in["m_matrices"])
    {
        const auto& vertexData = in["m_matrices"].as<YAML::Binary>();
        m_matrices.resize(vertexData.size() / sizeof(glm::mat4));
        std::memcpy(m_matrices.data(), vertexData.data(), vertexData.size());
    }
    if (in["m_colors"])
    {
        const auto& vertexData = in["m_colors"].as<YAML::Binary>();
        m_colors.resize(vertexData.size() / sizeof(glm::vec4));
        std::memcpy(m_colors.data(), vertexData.data(), vertexData.size());
    }
    Update();
}
void ParticleMatrices::Update()
{
    if(m_matrices.empty() || m_matrices.size() != m_colors.size()){
        m_bufferReady = false;
        return;
    }
    m_buffer->SetData((GLsizei)m_matrices.size() * sizeof(glm::mat4), m_matrices.data(), GL_DYNAMIC_DRAW);
    m_colorBuffer->SetData((GLsizei)m_colors.size() * sizeof(glm::vec4), m_colors.data(), GL_DYNAMIC_DRAW);
    m_bufferReady = true;
    m_version++;
}
ParticleMatrices::ParticleMatrices()
{
    m_buffer = std::make_shared<OpenGLUtils::GLBuffer>(OpenGLUtils::GLBufferTarget::Array);
    m_colorBuffer = std::make_shared<OpenGLUtils::GLBuffer>(OpenGLUtils::GLBufferTarget::Array);
}

std::vector<glm::vec4>& ParticleMatrices::RefColors()
{
    return m_colors;
}

std::vector<glm::mat4>& ParticleMatrices::RefMatrices()
{
    return m_matrices;
}

const std::vector<glm::vec4>& ParticleMatrices::PeekColors() const
{
    return m_colors;
}

const std::vector<glm::mat4>& ParticleMatrices::PeekMatrices() const
{
    return m_matrices;
}

void ParticleMatrices::SetValue(const std::vector<glm::vec4>& colors, const std::vector<glm::mat4>& matrices)
{
    m_colors = colors;
    m_matrices = matrices;
    if(m_colors.size() != m_matrices.size())
    {
        m_colors.resize(m_matrices.size());
        if(!m_colors.empty()) std::fill(m_colors.begin(), m_colors.end(), glm::vec4(1.0f));
    }
}

void ParticleMatrices::Reset()
{
    m_colors.clear();
    m_matrices.clear();
    m_version = 0;
}

size_t ParticleMatrices::GetVersion() const
{
    return m_version;
}
