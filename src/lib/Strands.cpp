//
// Created by lllll on 9/26/2022.
//

#include "Strands.hpp"
#include "Console.hpp"
#include "ClassRegistry.hpp"

using namespace UniEngine;
AssetRegistration<Strands> StrandsReg("Strands", {".uestrands", ".hair"});

unsigned int Strands::CurveDegree() const {
    switch (m_splineMode) {
        case SplineMode::Linear:
            return 1;
        case SplineMode::Quadratic:
            return 2;
        case SplineMode::Cubic:
            return 3;
        default: UNIENGINE_ERROR("Invalid spline mode.");
    }
}

void Strands::SetSplineMode(Strands::SplineMode splineMode) {
    m_splineMode = splineMode;
    m_saved = false;
    PrepareStrands();
}

Strands::SplineMode Strands::GetSplineMode() const { return m_splineMode; }

std::vector<int> &Strands::UnsafeGetStrands() {
    return m_strands;
}

std::vector<glm::vec2> &Strands::UnsafeGetStrandU() {
    return m_strandU;
}

std::vector<int> &Strands::UnsafeGetStrandIndices() {
    return m_strandIndices;
}

std::vector<glm::uvec2> &Strands::UnsafeGetStrandInfos() {
    return m_strandInfos;
}

std::vector<glm::vec3> &Strands::UnsafeGetPoints() {
    return m_points;
}

std::vector<float> &Strands::UnsafeGetThickness() {
    return m_thickness;
}

std::vector<int> &Strands::UnsafeGetSegments() {
    return m_segments;
}

void Strands::PrepareStrands() {
    m_segments.clear();
    m_strandU.clear();
    m_strandIndices.clear();
    m_strandInfos.clear();
    // loop to one before end, as last strand value is the "past last valid vertex"
    // index
    int strandIndex = 0;
    unsigned int firstPrimitiveIndex = 0;
    for (auto strand = m_strands.begin(); strand != m_strands.end() - 1; ++strand) {
        const int start = *(strand);                      // first vertex in first segment
        const int end = *(strand + 1) - CurveDegree();  // second vertex of last segment
        for (int i = start; i < end; ++i) {
            m_segments.emplace_back(i);
        }
        //-----------------------
        const int segments = end - start;  // number of strand's segments
        const float scale = 1.0f / segments;
        for (int i = 0; i < segments; ++i) {
            m_strandU.emplace_back(i * scale, scale);
        }
        //-----------------------
        for (auto segment = start; segment != end; ++segment) {
            m_strandIndices.emplace_back(strandIndex);
        }
        //-----------------------
        ++strandIndex;
        glm::uvec2 info;
        info.x = firstPrimitiveIndex;                        // strand's start index
        info.y = segments;  // number of segments in strand
        firstPrimitiveIndex += info.y;                       // increment with number of primitives/segments in strand
        m_strandInfos.emplace_back(info);
    }
    m_version++;
    m_saved = false;
}

// .hair format spec here: http://www.cemyuksel.com/research/hairmodels/
struct HairHeader {
    // Bytes 0 - 3  Must be "HAIR" in ascii code(48 41 49 52)
    char magic[4];

    // Bytes 4 - 7  Number of hair strands as unsigned int
    uint32_t numStrands;

    // Bytes 8 - 11  Total number of points of all strands as unsigned int
    uint32_t numPoints;

    // Bytes 12 - 15  Bit array of data in the file
    // Bit - 5 to Bit - 31 are reserved for future extension(must be 0).
    uint32_t flags;

    // Bytes 16 - 19  Default number of segments of hair strands as unsigned int
    // If the file does not have a segments array, this default value is used.
    uint32_t defaultNumSegments;

    // Bytes 20 - 23  Default thickness hair strands as float
    // If the file does not have a thickness array, this default value is used.
    float defaultThickness;

    // Bytes 24 - 27  Default transparency hair strands as float
    // If the file does not have a transparency array, this default value is used.
    float defaultAlpha;

    // Bytes 28 - 39  Default color hair strands as float array of size 3
    // If the file does not have a color array, this default value is used.
    glm::vec3 defaultColor;

    // Bytes 40 - 127  File information as char array of size 88 in ascii
    char fileInfo[88];

    [[nodiscard]] bool hasSegments() const {
        return (flags & (0x1 << 0)) > 0;
    }

    [[nodiscard]] bool hasPoints() const {
        return (flags & (0x1 << 1)) > 0;
    }

    [[nodiscard]] bool hasThickness() const {
        return (flags & (0x1 << 2)) > 0;
    }

    [[nodiscard]] bool hasAlpha() const {
        return (flags & (0x1 << 3)) > 0;
    }

    [[nodiscard]] bool hasColor() const {
        return (flags & (0x1 << 4)) > 0;
    }
};

bool Strands::LoadInternal(const std::filesystem::path &path) {
    if (path.extension() == ".uestrands") {
        return IAsset::LoadInternal(path);
    } else if (path.extension() == ".hair") {
        try {
            std::string fileName = path.string();
            std::ifstream input(fileName.c_str(), std::ios::binary);
            HairHeader header;
            input.read(reinterpret_cast<char *>(&header), sizeof(HairHeader));
            assert(input);
            assert(strncmp(header.magic, "HAIR", 4) == 0);
            header.fileInfo[87] = 0;

            // Segments array(unsigned short)
            // The segements array contains the number of linear segments per strand;
            // thus there are segments + 1 control-points/vertices per strand.
            auto strandSegments = std::vector<unsigned short>(header.numStrands);
            if (header.hasSegments()) {
                input.read(reinterpret_cast<char *>( strandSegments.data()),
                           header.numStrands * sizeof(unsigned short));
                assert(input);
            } else {
                std::fill(strandSegments.begin(), strandSegments.end(), header.defaultNumSegments);
            }

            // Compute strands vector<unsigned int>. Each element is the index to the
            // first point of the first segment of the strand. The last entry is the
            // index "one beyond the last vertex".
            m_strands = std::vector<int>(strandSegments.size() + 1);
            auto strand = m_strands.begin();
            *strand++ = 0;
            for (auto segments: strandSegments) {
                *strand = *(strand - 1) + 1 + segments;
                strand++;
            }

            // Points array(float)
            assert(header.hasPoints());
            m_points = std::vector<glm::vec3>(header.numPoints);
            input.read(reinterpret_cast<char *>( m_points.data()), header.numPoints * sizeof(glm::vec3));
            assert(input);

            // Thickness array(float)
            m_thickness = std::vector<float>(header.numPoints);
            if (header.hasThickness()) {
                input.read(reinterpret_cast<char *>( m_thickness.data()), header.numPoints * sizeof(float));
                assert(input);
            } else {
                std::fill(m_thickness.begin(), m_thickness.end(), header.defaultThickness);
            }
            PrepareStrands();
            return true;
        } catch (std::exception &e) {
            return false;
        }

    }
    return false;
}

size_t Strands::GetVersion() const {
    return m_version;
}

static const char *SplineModes[]{"Linear", "Quadratic", "Cubic"};

void Strands::OnInspect() {
    ImGui::Text(("Point size: " + std::to_string(m_points.size())).c_str());
    bool changed = false;
    if (ImGui::Combo(
            "Spline Mode",
            reinterpret_cast<int *>(&m_splineMode),
            SplineModes,
            IM_ARRAYSIZE(SplineModes))) {
        SetSplineMode(m_splineMode);
        changed = true;
    }

    if (changed) m_saved = false;
}

void Strands::Serialize(YAML::Emitter &out) {
    out << YAML::Key << "m_splineMode" << YAML::Value << (int) m_splineMode;

    if (!m_strands.empty() && !m_points.empty() && m_thickness.size() == m_points.size()) {
        out << YAML::Key << "m_strands" << YAML::Value
            << YAML::Binary((const unsigned char *) m_strands.data(), m_strands.size() * sizeof(int));
        out << YAML::Key << "m_points" << YAML::Value
            << YAML::Binary((const unsigned char *) m_points.data(), m_points.size() * sizeof(glm::vec3));
        out << YAML::Key << "m_thickness" << YAML::Value
            << YAML::Binary((const unsigned char *) m_thickness.data(), m_thickness.size() * sizeof(float));
    }
}

void Strands::Deserialize(const YAML::Node &in) {
    if (in["m_splineMode"]) m_splineMode = (SplineMode) in["m_splineMode"].as<int>();
    if (in["m_strands"] && in["m_points"] && in["m_thickness"]){
        auto strandData = in["m_vertices"].as<YAML::Binary>();
        m_strands.resize(strandData.size() / sizeof(int));
        std::memcpy(m_strands.data(), strandData.data(), strandData.size());

        auto pointData = in["m_points"].as<YAML::Binary>();
        m_points.resize(pointData.size() / sizeof(glm::vec3));
        std::memcpy(m_points.data(), pointData.data(), pointData.size());

        auto thicknessData = in["m_thickness"].as<YAML::Binary>();
        m_thickness.resize(thicknessData.size() / sizeof(float));
        std::memcpy(m_thickness.data(), thicknessData.data(), thicknessData.size());

        PrepareStrands();
    }

}


