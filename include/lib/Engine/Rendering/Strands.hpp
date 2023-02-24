#pragma once

#include <Utilities.hpp>
#include "OpenGLUtils.hpp"
#include "IAsset.hpp"
#include "Particles.hpp"
#include "Vertex.hpp"
#include "RenderGeometry.hpp"
namespace UniEngine {
    class UNIENGINE_API Strands : public IAsset, public RenderGeometry {
    public:
        enum class SplineMode {
            Linear = 2,
            Quadratic = 3,
            Cubic = 4
        };

        [[nodiscard]] SplineMode GetSplineMode() const;

        [[nodiscard]] std::vector<glm::uint> &UnsafeGetSegments();

        [[nodiscard]] std::vector<StrandPoint> &UnsafeGetPoints();

        [[nodiscard]] size_t GetVersion() const;

        void OnInspect() override;

        void Serialize(YAML::Emitter &out) override;

        void Deserialize(const YAML::Node &in) override;

        void SetSegments(const unsigned& mask, const std::vector<glm::uint>& segments,
                       const std::vector<StrandPoint> &points);

        void SetStrands(const unsigned& mask, const std::vector<glm::uint>& strands,
            const std::vector<StrandPoint>& points);
        void RecalculateNormal();
        void Draw() const override;
        void DrawInstanced(const std::vector<glm::mat4>& matrices) const override;
        void DrawInstanced(const std::shared_ptr<ParticleMatrices>& particleMatrices) const override;
        void DrawInstanced(const std::vector<GlobalTransform>& matrices) const override;

        void Upload();

        void OnCreate() override;
        void Enable() const;
        [[nodiscard]] std::shared_ptr<OpenGLUtils::GLVAO> Vao() const;

        [[nodiscard]] Bound GetBound();

        [[nodiscard]] size_t GetSegmentAmount() const;
        [[nodiscard]] size_t GetPointAmount() const;
    protected:
        bool LoadInternal(const std::filesystem::path &path) override;

    private:
        size_t m_offset = 0;
        unsigned m_segmentIndicesSize = 0;
        unsigned m_pointSize = 0;

        friend class StrandsRenderer;
        friend class RenderLayer;
        std::shared_ptr<OpenGLUtils::GLVAO> m_vao;
        Bound m_bound;

        [[nodiscard]] unsigned int CurveDegree() const;

        void PrepareStrands(const unsigned& mask);

        size_t m_version = 0;

        //The starting index of the point where this segment starts;
        std::vector<glm::uint> m_segments;

        std::vector<glm::uvec4> m_segmentIndices;
        std::vector<StrandPoint> m_points;

        SplineMode m_splineMode = SplineMode::Cubic;
    };
}