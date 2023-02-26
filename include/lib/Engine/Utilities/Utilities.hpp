#pragma once
#include "Entity.hpp"
#include "Plot2D.hpp"
namespace UniEngine
{
	class UNIENGINE_API FileUtils
	{
		friend class DefaultResources;

	public:
		static std::string LoadFileAsString(const std::filesystem::path& path = "");
		static void OpenFolder(
			const std::string& dialogTitle,
			const std::function<void(const std::filesystem::path& path)>& func, bool projectDirCheck = true);

		static void OpenFile(
			const std::string& dialogTitle,
			const std::string& fileType,
			const std::vector<std::string>& extensions,
			const std::function<void(const std::filesystem::path& path)>& func, bool projectDirCheck = true);
		static void SaveFile(
			const std::string& dialogTitle,
			const std::string& fileType,
			const std::vector<std::string>& extensions,
			const std::function<void(const std::filesystem::path& path)>& func, bool projectDirCheck = true);
		static std::pair<bool, uint32_t> DirectoryTreeViewRecursive(
			const std::filesystem::path& path, uint32_t* count, int* selection_mask);
	};

	struct UNIENGINE_API Bound
	{
		glm::vec3 m_min;
		glm::vec3 m_max;
		Bound();
		[[nodiscard]] glm::vec3 Size() const;
		[[nodiscard]] glm::vec3 Center() const;
		[[nodiscard]] bool InBound(const glm::vec3& position) const;
		void ApplyTransform(const glm::mat4& transform);
		void PopulateCorners(std::vector<glm::vec3>& corners) const;
	};
	struct UNIENGINE_API Ray : IDataComponent
	{
		glm::vec3 m_start;
		glm::vec3 m_direction;
		float m_length;
		Ray() = default;
		Ray(glm::vec3 start, glm::vec3 end);
		Ray(glm::vec3 start, glm::vec3 direction, float length);
		[[nodiscard]] bool Intersect(const glm::vec3& position, float radius) const;
		[[nodiscard]] bool Intersect(const glm::mat4& transform, const Bound& bound) const;
		[[nodiscard]] glm::vec3 GetEnd() const;
	};
	struct UNIENGINE_API Bezier2D
	{
		bool m_fixed = true;
		glm::vec2 m_controlPoints[4];
		Bezier2D();
		[[nodiscard]] glm::vec2 GetPoint(const float& t) const;
		bool DrawGraph(const std::string& label);
	};
	

	

	class UNIENGINE_API SphereMeshGenerator {
	public:
		static void Icosahedron(std::vector<glm::vec3>& vertices, std::vector<glm::uvec3>& triangles);
	};

} // namespace UniEngine
namespace ImGui
{
	IMGUI_API bool Splitter(
		bool split_vertically,
		float thickness,
		float& size1,
		float& size2,
		float min_size1,
		float min_size2,
		float splitter_long_axis_size = -1.0f);

	IMGUI_API bool Combo(const std::string& label, const std::vector<std::string>& items, unsigned& currentSelection, ImGuiComboFlags flags = 0);
	IMGUI_API bool Combo(const std::string& label, const std::vector<std::string>& items, int& currentSelection, ImGuiComboFlags flags = 0);
}
