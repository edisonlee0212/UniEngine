#pragma once

#include <Utilities.hpp>
#include "OpenGLUtils.hpp"
#include "IAsset.hpp"

namespace UniEngine {

    class UNIENGINE_API Strands : public IAsset {
    public:
        enum class SplineMode {
            LINEAR_BSPLINE,
            QUADRATIC_BSPLINE,
            CUBIC_BSPLINE
        };
        enum class ShadeMode {
            SEGMENT_U,
            STRAND_U,
            STRAND_IDX
        };
        enum class RadiusMode {
            CONSTANT_R,
            TAPERED_R
        };

        void SetSplineMode(SplineMode splineMode);

        [[nodiscard]] SplineMode GetSplineMode() const;

        void SetShadeMode(ShadeMode shadeMode);

        [[nodiscard]] ShadeMode GetShadeMode() const;

        void SetRadiusMode(RadiusMode radiusMode);

        [[nodiscard]] RadiusMode GetRadiusMode() const;

        [[nodiscard]] const std::vector<int> &GetStrands() const;

        [[nodiscard]] const std::vector<int> &GetSegments() const;

        [[nodiscard]] const std::vector<glm::vec3> &GetPoints() const;

        [[nodiscard]] const std::vector<float> &GetThickness() const;

        [[nodiscard]] const std::vector<glm::vec2> &GetStrandU() const;

        [[nodiscard]] const std::vector<int> &GetStrandIndices() const;

        [[nodiscard]] const std::vector<glm::uvec2> &GetStrandInfos() const;

    protected:
        bool LoadInternal(const std::filesystem::path &path) override;

    private:
        friend class StrandsRenderer;

        Bound m_bound;

        [[nodiscard]] unsigned int CurveDegree() const;

        void PrepareStrands();

        size_t m_version = 0;
        std::vector<int> m_segments;
        std::vector<glm::vec2> m_strandU;
        std::vector<int> m_strandIndices;
        std::vector<glm::uvec2> m_strandInfos;

        std::vector<int> m_strands;
        std::vector<glm::vec3> m_points;
        std::vector<float> m_thickness;
        
        SplineMode m_splineMode = SplineMode::CUBIC_BSPLINE;
        ShadeMode m_shadeMode = ShadeMode::SEGMENT_U;
        RadiusMode m_radiusMode = RadiusMode::CONSTANT_R;
    };
}