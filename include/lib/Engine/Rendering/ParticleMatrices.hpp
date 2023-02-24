#pragma once
#include "OpenGLUtils.hpp"
namespace UniEngine
{
    class UNIENGINE_API ParticleMatrices : ISerializable {
        std::shared_ptr<OpenGLUtils::GLBuffer> m_buffer;
        std::shared_ptr<OpenGLUtils::GLBuffer> m_colorBuffer;
        bool m_bufferReady = false;
        friend class Mesh;
        friend class Strands;
        friend class SkinnedMesh;
        size_t m_version = 0;

        std::vector<glm::vec4> m_colors;
        std::vector<glm::mat4> m_matrices;
    public:
        void Reset();
        [[nodiscard]] size_t GetVersion() const;
        ParticleMatrices();
        std::vector<glm::vec4>& RefColors();
        std::vector<glm::mat4>& RefMatrices();
        const std::vector<glm::vec4>& PeekColors() const;
        const std::vector<glm::mat4>& PeekMatrices() const;

        void SetValue(const std::vector<glm::vec4>& colors, const std::vector<glm::mat4>& matrices);
        void Serialize(YAML::Emitter& out) override;
        void Deserialize(const YAML::Node& in) override;
        void Update();
    };
    
}