#pragma once

#include <Utilities.hpp>
#include "OpenGLUtils.hpp"
#include "IAsset.hpp"
#include "Vertex.hpp"

namespace UniEngine {
    class UNIENGINE_API Strands : public IAsset {
    public:
        enum class SplineMode {
            Linear,
            Quadratic,
            Cubic
        };

        void SetSplineMode(SplineMode splineMode);

        [[nodiscard]] SplineMode GetSplineMode() const;

        [[nodiscard]] std::vector<int> &UnsafeGetStrands();

        [[nodiscard]] std::vector<int> &UnsafeGetSegments();

        [[nodiscard]] std::vector<StrandPoint> &UnsafeGetPoints();

        [[nodiscard]] std::vector<glm::vec2> &UnsafeGetStrandU();

        [[nodiscard]] std::vector<int> &UnsafeGetStrandIndices();

        [[nodiscard]] std::vector<glm::uvec2> &UnsafeGetStrandInfos();

        [[nodiscard]] size_t GetVersion() const;

        void OnInspect() override;

        void Serialize(YAML::Emitter &out) override;

        void Deserialize(const YAML::Node &in) override;

        void SetPoints(const std::vector<int> &strands,
                       const std::vector<StrandPoint> &points, SplineMode splineMode = SplineMode::Linear);

    protected:
        bool LoadInternal(const std::filesystem::path &path) override;

    private:
        friend class StrandsRenderer;

        Bound m_bound;

        [[nodiscard]] unsigned int CurveDegree() const;

        void PrepareStrands();

        size_t m_version = 0;
        //The starting index of point where this segment starts;
        std::vector<int> m_segments;
        //The start and end's U for current segment for entire strand.
        std::vector<glm::vec2> m_strandU;
        //The index of strand this segment belongs.
        std::vector<int> m_strandIndices;
        //Current strand's start index and number of segment in current strand
        std::vector<glm::uvec2> m_strandInfos;

        std::vector<int> m_strands;
        std::vector<StrandPoint> m_points;

        SplineMode m_splineMode = SplineMode::Cubic;
    };
}