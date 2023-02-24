#pragma once
#include "Transform.hpp"
#include "Vertex.hpp"
#include "ParticleMatrices.hpp"
namespace UniEngine
{

	class UNIENGINE_API RenderGeometry
	{
	public:
		virtual void Draw() const = 0;
		virtual void DrawInstanced(const std::vector<glm::mat4>& matrices) const {}
		virtual void DrawInstanced(const std::vector<GlobalTransform>& matrices) const {}
		virtual void DrawInstanced(const std::shared_ptr<ParticleMatrices>& matrices) const {}
		virtual void DrawInstancedColored(const std::vector<glm::vec4>& colors, const std::vector<glm::mat4>& matrices) const {}
		virtual void DrawInstancedColored(const std::vector<glm::vec4>& colors, const std::vector<GlobalTransform>& matrices) const {}
	};
}