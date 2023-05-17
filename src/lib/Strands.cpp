//
// Created by lllll on 9/26/2022.
//

#include "Strands.hpp"
#include "Console.hpp"
#include "ClassRegistry.hpp"
#include "RenderLayer.hpp"
using namespace UniEngine;
AssetRegistration<Strands> StrandsReg("Strands", { ".uestrands", ".hair" });

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

Strands::SplineMode Strands::GetSplineMode() const { return m_splineMode; }


std::vector<StrandPoint>& Strands::UnsafeGetPoints() {
	return m_points;
}

std::vector<glm::uint>& Strands::UnsafeGetSegments() {
	return m_segments;
}

void Strands::PrepareStrands(const unsigned& mask) {
	m_splineMode = SplineMode::Cubic;
	m_segmentIndices.resize(m_segments.size());
	std::vector<std::shared_future<void>> results;
	Jobs::ParallelFor(m_segments.size(), [&](unsigned i)
		{
			m_segmentIndices[i].x = m_segments[i];
	m_segmentIndices[i].y = m_segments[i] + 1;
	m_segmentIndices[i].z = m_segments[i] + 2;
	m_segmentIndices[i].w = m_segments[i] + 3;
		}, results);
	for (auto& result : results) result.wait();


#pragma region Bound
	glm::vec3 minBound = m_points.at(0).m_position;
	glm::vec3 maxBound = m_points.at(0).m_position;
	for (auto& vertex : m_points)
	{
		minBound = glm::vec3(
			(glm::min)(minBound.x, vertex.m_position.x),
			(glm::min)(minBound.y, vertex.m_position.y),
			(glm::min)(minBound.z, vertex.m_position.z));
		maxBound = glm::vec3(
			(glm::max)(maxBound.x, vertex.m_position.x),
			(glm::max)(maxBound.y, vertex.m_position.y),
			(glm::max)(maxBound.z, vertex.m_position.z));
	}
	m_bound.m_max = maxBound;
	m_bound.m_min = minBound;
#pragma endregion

	if (!(mask & static_cast<unsigned>(StrandPointAttribute::Normal))) RecalculateNormal();

	Upload();
	/*
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


		const int segments = end - start;  // number of strand's segments
		const float scale = 1.0f / segments;
		for (int i = 0; i < segments; ++i) {
			m_strandU.emplace_back(i * scale, scale);
		}

		for (auto segment = start; segment != end; ++segment) {
			m_strandIndices.emplace_back(strandIndex);
		}

		++strandIndex;
		glm::uvec2 info;
		info.x = firstPrimitiveIndex;                        // strand's start index
		info.y = segments;  // number of segments in strand
		firstPrimitiveIndex += info.y;                       // increment with number of primitives/segments in strand
		m_strandInfos.emplace_back(info);

	}
	*/
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

bool Strands::LoadInternal(const std::filesystem::path& path) {
	if (path.extension() == ".uestrands") {
		return IAsset::LoadInternal(path);
	}
	else if (path.extension() == ".hair") {
		try {
			std::string fileName = path.string();
			std::ifstream input(fileName.c_str(), std::ios::binary);
			HairHeader header;
			input.read(reinterpret_cast<char*>(&header), sizeof(HairHeader));
			assert(input);
			assert(strncmp(header.magic, "HAIR", 4) == 0);
			header.fileInfo[87] = 0;

			// Segments array(unsigned short)
			// The segements array contains the number of linear segments per strand;
			// thus there are segments + 1 control-points/vertices per strand.
			auto strandSegments = std::vector<unsigned short>(header.numStrands);
			if (header.hasSegments()) {
				input.read(reinterpret_cast<char*>(strandSegments.data()),
					header.numStrands * sizeof(unsigned short));
				assert(input);
			}
			else {
				std::fill(strandSegments.begin(), strandSegments.end(), header.defaultNumSegments);
			}

			// Compute strands vector<unsigned int>. Each element is the index to the
			// first point of the first segment of the strand. The last entry is the
			// index "one beyond the last vertex".
			auto strands = std::vector<glm::uint>(strandSegments.size() + 1);
			auto strand = strands.begin();
			*strand++ = 0;
			for (auto segments : strandSegments) {
				*strand = *(strand - 1) + 1 + segments;
				strand++;
			}

			// Points array(float)
			assert(header.hasPoints());
			auto points = std::vector<glm::vec3>(header.numPoints);
			input.read(reinterpret_cast<char*>(points.data()), header.numPoints * sizeof(glm::vec3));
			assert(input);

			// Thickness array(float)
			auto thickness = std::vector<float>(header.numPoints);
			if (header.hasThickness()) {
				input.read(reinterpret_cast<char*>(thickness.data()), header.numPoints * sizeof(float));
				assert(input);
			}
			else {
				std::fill(thickness.begin(), thickness.end(), header.defaultThickness);
			}

			// Color array(float)
			auto color = std::vector<glm::vec3>(header.numPoints);
			if (header.hasColor()) {
				input.read(reinterpret_cast<char*>(color.data()), header.numPoints * sizeof(glm::vec3));
				assert(input);
			}
			else {
				std::fill(color.begin(), color.end(), header.defaultColor);
			}

			// Alpha array(float)
			auto alpha = std::vector<float>(header.numPoints);
			if (header.hasAlpha()) {
				input.read(reinterpret_cast<char*>(alpha.data()), header.numPoints * sizeof(float));
				assert(input);
			}
			else {
				std::fill(alpha.begin(), alpha.end(), header.defaultAlpha);
			}
			std::vector<StrandPoint> strandPoints;
			strandPoints.resize(header.numPoints);
			for (int i = 0; i < header.numPoints; i++) {
				strandPoints[i].m_position = points[i];
				strandPoints[i].m_thickness = thickness[i];
				strandPoints[i].m_color = glm::vec4(color[i], alpha[i]);
				strandPoints[i].m_texCoord = 0.0f;
			}

			SetStrands(1 + 2 + 16, strands, strandPoints);
			return true;
		}
		catch (std::exception& e) {
			return false;
		}

	}
	return false;
}

size_t Strands::GetVersion() const {
	return m_version;
}

static const char* SplineModes[]{ "Linear", "Quadratic", "Cubic" };

void Strands::OnInspect() {
	bool changed = false;
	ImGui::Text(("Point size: " + std::to_string(m_points.size())).c_str());
	if (changed) m_saved = false;
}

void Strands::Serialize(YAML::Emitter& out) {
	out << YAML::Key << "m_offset" << YAML::Value << m_offset;
	out << YAML::Key << "m_version" << YAML::Value << m_version;

	if (!m_segments.empty() && !m_points.empty()) {
		out << YAML::Key << "m_segments" << YAML::Value
			<< YAML::Binary((const unsigned char*)m_segments.data(), m_segments.size() * sizeof(glm::uint));

		out << YAML::Key << "m_scatteredPoints" << YAML::Value
			<< YAML::Binary((const unsigned char*)m_points.data(), m_points.size() * sizeof(StrandPoint));
	}
}

void Strands::Deserialize(const YAML::Node& in) {
	if (in["m_offset"]) m_offset = in["m_offset"].as<size_t>();
	if (in["m_version"]) m_version = in["m_version"].as<size_t>();

	if (in["m_segments"] && in["m_scatteredPoints"]) {
		auto segmentData = in["m_segments"].as<YAML::Binary>();
		m_segments.resize(segmentData.size() / sizeof(glm::uint));
		std::memcpy(m_segments.data(), segmentData.data(), segmentData.size());

		auto pointData = in["m_scatteredPoints"].as<YAML::Binary>();
		m_points.resize(pointData.size() / sizeof(StrandPoint));
		std::memcpy(m_points.data(), pointData.data(), pointData.size());

		PrepareStrands(31);
	}

}

void Strands::Draw() const
{
	OpenGLUtils::PatchParameter(GL_PATCH_VERTICES, 4);

	m_vao->Bind();

	m_vao->DisableAttributeArray(12);
	m_vao->DisableAttributeArray(13);
	m_vao->DisableAttributeArray(14);
	m_vao->DisableAttributeArray(15);


	glDrawElements(GL_PATCHES, (GLsizei)m_segmentIndicesSize * 4, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * m_offset));

	OpenGLUtils::GLVAO::BindDefault();
}

void Strands::DrawInstanced(const std::vector<glm::mat4>& matrices) const
{
	OpenGLUtils::PatchParameter(GL_PATCH_VERTICES, 4);

	auto count = matrices.size();
	Application::GetLayer<RenderLayer>()->m_instancedMatricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices.data(), GL_DYNAMIC_DRAW);
	m_vao->Bind();

	m_vao->EnableAttributeArray(12);
	m_vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	m_vao->EnableAttributeArray(13);
	m_vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(14);
	m_vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(15);
	m_vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	m_vao->SetAttributeDivisor(12, 1);
	m_vao->SetAttributeDivisor(13, 1);
	m_vao->SetAttributeDivisor(14, 1);
	m_vao->SetAttributeDivisor(15, 1);


	glDrawElementsInstanced(GL_PATCHES, (GLsizei)m_segmentIndicesSize * 4, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());

}

void Strands::DrawInstanced(const std::shared_ptr<ParticleMatrices>& particleMatrices) const
{
	OpenGLUtils::PatchParameter(GL_PATCH_VERTICES, 4);

	if (!particleMatrices->m_bufferReady) return;
	auto count = particleMatrices->m_matrices.size();
	particleMatrices->m_buffer->Bind();
	m_vao->Bind();

	m_vao->EnableAttributeArray(12);
	m_vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	m_vao->EnableAttributeArray(13);
	m_vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(14);
	m_vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(15);
	m_vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	m_vao->SetAttributeDivisor(12, 1);
	m_vao->SetAttributeDivisor(13, 1);
	m_vao->SetAttributeDivisor(14, 1);
	m_vao->SetAttributeDivisor(15, 1);
	glDrawElementsInstanced(GL_PATCHES, (GLsizei)m_segmentIndicesSize * 4, GL_UNSIGNED_INT, 0, (GLsizei)count);
}

void Strands::DrawInstanced(const std::vector<GlobalTransform>& matrices) const
{
	OpenGLUtils::PatchParameter(GL_PATCH_VERTICES, 4);

	auto count = matrices.size();
	Application::GetLayer<RenderLayer>()->m_instancedMatricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices.data(), GL_DYNAMIC_DRAW);
	m_vao->Bind();

	m_vao->EnableAttributeArray(12);
	m_vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	m_vao->EnableAttributeArray(13);
	m_vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(14);
	m_vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	m_vao->EnableAttributeArray(15);
	m_vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	m_vao->SetAttributeDivisor(12, 1);
	m_vao->SetAttributeDivisor(13, 1);
	m_vao->SetAttributeDivisor(14, 1);
	m_vao->SetAttributeDivisor(15, 1);


	glDrawElementsInstanced(GL_PATCHES, (GLsizei)m_segmentIndicesSize * 4, GL_UNSIGNED_INT, 0, (GLsizei)matrices.size());
}

void Strands::Upload()
{
	if (m_segmentIndices.empty())
	{
		UNIENGINE_ERROR("Indices empty!")
			return;
	}
	if (m_points.empty())
	{
		UNIENGINE_ERROR("Points empty!")
			return;
	}
#pragma region Data
	m_vao->SetData((GLsizei)(m_points.size() * sizeof(StrandPoint)), nullptr, GL_STATIC_DRAW);
	m_vao->SubData(0, m_points.size() * sizeof(StrandPoint), m_points.data());
#pragma endregion

#pragma region AttributePointer
	m_vao->EnableAttributeArray(0);
	m_vao->SetAttributePointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StrandPoint), 0);

	m_vao->EnableAttributeArray(1);
	m_vao->SetAttributePointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(StrandPoint), (void*)(offsetof(StrandPoint, m_thickness)));

	m_vao->EnableAttributeArray(2);
	m_vao->SetAttributePointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(StrandPoint), (void*)(offsetof(StrandPoint, m_normal)));

	m_vao->EnableAttributeArray(3);
	m_vao->SetAttributePointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(StrandPoint), (void*)(offsetof(StrandPoint, m_texCoord)));

	m_vao->EnableAttributeArray(4);
	m_vao->SetAttributePointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(StrandPoint), (void*)(offsetof(StrandPoint, m_color)));
#pragma endregion
	m_vao->Ebo().SetData((GLsizei)m_segmentIndices.size() * sizeof(glm::uvec4), m_segmentIndices.data(), GL_STATIC_DRAW);
	m_pointSize = m_points.size();
	m_segmentIndicesSize = m_segmentIndices.size();
	m_version++;
}

void Strands::OnCreate()
{
	m_vao = std::make_shared<OpenGLUtils::GLVAO>();
	m_bound = Bound();
}

void Strands::Enable() const
{
	m_vao->Bind();
}

std::shared_ptr<OpenGLUtils::GLVAO> Strands::Vao() const
{
	return m_vao;
}

Bound Strands::GetBound()
{
	return m_bound;
}

size_t Strands::GetSegmentAmount() const
{
	return m_segmentIndicesSize;
}

size_t Strands::GetPointAmount() const
{
	return m_pointSize;
}

void Strands::SetSegments(const unsigned& mask, const std::vector<glm::uint>& segments, const std::vector<StrandPoint>& points) {
	if (points.empty() || segments.empty()) {
		return;
	}

	m_segments = segments;
	m_points = points;

	PrepareStrands(mask);
}

void Strands::SetStrands(const unsigned& mask, const std::vector<glm::uint>& strands, const std::vector<StrandPoint>& points)
{
	if (points.empty() || strands.empty()) {
		return;
	}

	m_segments.clear();
	// loop to one before end, as last strand value is the "past last valid vertex"
	// index
	for (auto strand = strands.begin(); strand != strands.end() - 1; ++strand) {
		const int start = *(strand);                      // first vertex in first segment
		const int end = *(strand + 1) - CurveDegree();  // second vertex of last segment
		for (int i = start; i < end; i++) {
			m_segments.emplace_back(i);
		}
	}
	m_points = points;
	PrepareStrands(mask);
}


void CubicInterpolation(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, glm::vec3& result, glm::vec3& tangent, const float t)
{
	float t2 = t * t;

	glm::vec3 a0 = -0.5f * v0 + 1.5f * v1 - 1.5f * v2 + 0.5f * v3;
	glm::vec3 a1 = v0 - 2.5f * v1 + 2.0f * v2 - 0.5f * v3;
	glm::vec3 a2 = -0.5f * v0 + 0.5f * v2;
	glm::vec3 a3 = v1;

	result = glm::vec3(a0 * t * t2 + a1 * t2 + a2 * t + a3);
	tangent = normalize(glm::vec3(3.0f * a0 * t2 + 2.0f * a1 * t + a2));
}

void Strands::RecalculateNormal()
{
	glm::vec3 tangent, temp;
	for(const auto& indices : m_segmentIndices)
	{
		CubicInterpolation(m_points[indices[0]].m_position, m_points[indices[1]].m_position, m_points[indices[2]].m_position, m_points[indices[3]].m_position, temp, tangent, 0.0f);
		m_points[indices[0]].m_normal = glm::vec3(tangent.y, tangent.z, tangent.x);

		CubicInterpolation(m_points[indices[0]].m_position, m_points[indices[1]].m_position, m_points[indices[2]].m_position, m_points[indices[3]].m_position, temp, tangent, 0.25f);
		m_points[indices[1]].m_normal = glm::cross(glm::cross(tangent, m_points[indices[0]].m_normal), tangent);

		CubicInterpolation(m_points[indices[0]].m_position, m_points[indices[1]].m_position, m_points[indices[2]].m_position, m_points[indices[3]].m_position, temp, tangent, 0.75f);
		m_points[indices[2]].m_normal = glm::cross(glm::cross(tangent, m_points[indices[1]].m_normal), tangent);

		CubicInterpolation(m_points[indices[0]].m_position, m_points[indices[1]].m_position, m_points[indices[2]].m_position, m_points[indices[3]].m_position, temp, tangent, 1.0f);
		m_points[indices[3]].m_normal = glm::cross(glm::cross(tangent, m_points[indices[2]].m_normal), tangent);
	}
}


