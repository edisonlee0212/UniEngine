#include "Graphics.hpp"
#include "EditorLayer.hpp"
#include "RenderLayer.hpp"
#include "Application.hpp"
#include "ProjectManager.hpp"
#include "Camera.hpp"
#include "Cubemap.hpp"
#include "DefaultResources.hpp"
#include "Editor.hpp"
#include "Inputs.hpp"
#include "LightProbe.hpp"
#include "Lights.hpp"
#include "MeshRenderer.hpp"
#include "PostProcessing.hpp"
#include "ReflectionProbe.hpp"
#include "SkinnedMeshRenderer.hpp"

using namespace UniEngine;

#pragma region Helpers
unsigned int environmentalMapCubeVAO = 0;
unsigned int environmentalMapCubeVBO = 0;

void Graphics::RenderCube() {
// initialize (if necessary)
		if (environmentalMapCubeVAO == 0) {
				float vertices[] = {
								// back face
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								0.0f,
								0.0f, // bottom-left
								1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								1.0f,
								1.0f, // top-right
								1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								1.0f,
								0.0f, // bottom-right
								1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								1.0f,
								1.0f, // top-right
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								0.0f,
								0.0f, // bottom-left
								-1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								-1.0f,
								0.0f,
								1.0f, // top-left
								// front face
								-1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f,
								0.0f, // bottom-left
								1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								1.0f,
								0.0f, // bottom-right
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								1.0f,
								1.0f, // top-right
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								1.0f,
								1.0f, // top-right
								-1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f,
								1.0f, // top-left
								-1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f,
								0.0f, // bottom-left
								// left face
								-1.0f,
								1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f, // top-right
								-1.0f,
								1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								1.0f,
								1.0f, // top-left
								-1.0f,
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								0.0f,
								1.0f, // bottom-left
								-1.0f,
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								0.0f,
								0.0f,
								1.0f, // bottom-left
								-1.0f,
								-1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								0.0f,
								0.0f, // bottom-right
								-1.0f,
								1.0f,
								1.0f,
								-1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f, // top-right
								// right face
								1.0f,
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f, // top-left
								1.0f,
								-1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								0.0f,
								1.0f, // bottom-right
								1.0f,
								1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								1.0f, // top-right
								1.0f,
								-1.0f,
								-1.0f,
								1.0f,
								0.0f,
								0.0f,
								0.0f,
								1.0f, // bottom-right
								1.0f,
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f,
								0.0f, // top-left
								1.0f,
								-1.0f,
								1.0f,
								1.0f,
								0.0f,
								0.0f,
								0.0f,
								0.0f, // bottom-left
								// bottom face
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								-1.0f,
								0.0f,
								0.0f,
								1.0f, // top-right
								1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								-1.0f,
								0.0f,
								1.0f,
								1.0f, // top-left
								1.0f,
								-1.0f,
								1.0f,
								0.0f,
								-1.0f,
								0.0f,
								1.0f,
								0.0f, // bottom-left
								1.0f,
								-1.0f,
								1.0f,
								0.0f,
								-1.0f,
								0.0f,
								1.0f,
								0.0f, // bottom-left
								-1.0f,
								-1.0f,
								1.0f,
								0.0f,
								-1.0f,
								0.0f,
								0.0f,
								0.0f, // bottom-right
								-1.0f,
								-1.0f,
								-1.0f,
								0.0f,
								-1.0f,
								0.0f,
								0.0f,
								1.0f, // top-right
								// top face
								-1.0f,
								1.0f,
								-1.0f,
								0.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f, // top-left
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								1.0f,
								0.0f,
								1.0f,
								0.0f, // bottom-right
								1.0f,
								1.0f,
								-1.0f,
								0.0f,
								1.0f,
								0.0f,
								1.0f,
								1.0f, // top-right
								1.0f,
								1.0f,
								1.0f,
								0.0f,
								1.0f,
								0.0f,
								1.0f,
								0.0f, // bottom-right
								-1.0f,
								1.0f,
								-1.0f,
								0.0f,
								1.0f,
								0.0f,
								0.0f,
								1.0f, // top-left
								-1.0f,
								1.0f,
								1.0f,
								0.0f,
								1.0f,
								0.0f,
								0.0f,
								0.0f // bottom-left
				};
				glGenVertexArrays(1, &environmentalMapCubeVAO);
				glGenBuffers(1, &environmentalMapCubeVBO);
				// fill buffer
				glBindBuffer(GL_ARRAY_BUFFER, environmentalMapCubeVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				// link vertex attributes
				glBindVertexArray(environmentalMapCubeVAO);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
		}
// render Cube
		glBindVertexArray(environmentalMapCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		OpenGLUtils::GLVAO::BindDefault();
}

unsigned int environmentalMapQuadVAO = 0;
unsigned int environmentalMapQuadVBO;

void Graphics::RenderQuad() {
		if (environmentalMapQuadVAO == 0) {
				float quadVertices[] = {
								// positions        // texture Coords
								-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
								1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				};
				// setup plane VAO
				glGenVertexArrays(1, &environmentalMapQuadVAO);
				glGenBuffers(1, &environmentalMapQuadVBO);
				glBindVertexArray(environmentalMapQuadVAO);
				glBindBuffer(GL_ARRAY_BUFFER, environmentalMapQuadVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
		}
		glBindVertexArray(environmentalMapQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
}

void Gizmos::DrawGizmoStrandsInternal(const GizmoSettings &gizmoSettings, const std::shared_ptr<Strands> &strands,
																			const glm::vec4 &color, const glm::mat4 &model, const glm::mat4 &scaleMatrix) {
		if (strands == nullptr)
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();

		switch (gizmoSettings.m_colorMode) {
				case GizmoSettings::ColorMode::Default: {
						DefaultResources::GizmoStrandsProgram->Bind();
						DefaultResources::GizmoStrandsProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoStrandsProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::VertexColor: {
						DefaultResources::GizmoStrandsVertexColoredProgram->Bind();
						DefaultResources::GizmoStrandsVertexColoredProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsVertexColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::NormalColor: {
						DefaultResources::GizmoStrandsNormalColoredProgram->Bind();
						DefaultResources::GizmoStrandsNormalColoredProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsNormalColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
		}
		strands->Draw();
}

void
Gizmos::DrawGizmoStrandsInstancedInternal(const GizmoSettings &gizmoSettings, const std::shared_ptr<Strands> &strands,
																					const glm::vec4 &color, const glm::mat4 &model,
																					const std::vector<glm::mat4> &matrices,
																					const glm::mat4 &scaleMatrix) {
		if (strands == nullptr || matrices.empty())
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();
		strands->Enable();
		switch (gizmoSettings.m_colorMode) {
				case GizmoSettings::ColorMode::Default: {
						DefaultResources::GizmoStrandsInstancedProgram->Bind();
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::VertexColor: {
						DefaultResources::GizmoStrandsInstancedProgram->Bind();
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::NormalColor: {
						DefaultResources::GizmoStrandsInstancedProgram->Bind();
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoStrandsInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
		}
		strands->DrawInstanced(matrices);
}

void Gizmos::DrawGizmoStrandsInstancedColoredInternal(const GizmoSettings &gizmoSettings,
																											const std::shared_ptr<Strands> &strands,
																											const std::vector<glm::vec4> &colors,
																											const std::vector<glm::mat4> &matrices,
																											const glm::mat4 &model, const glm::mat4 &scaleMatrix) {
		if (strands == nullptr || matrices.empty() || colors.empty() || matrices.size() != colors.size())
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();
		strands->Enable();
		const auto vao = strands->Vao();
		const OpenGLUtils::GLBuffer colorsBuffer(OpenGLUtils::GLBufferTarget::Array);
		colorsBuffer.SetData(static_cast<GLsizei>(matrices.size()) * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
		vao->EnableAttributeArray(11);
		vao->SetAttributePointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *) 0);
		vao->SetAttributeDivisor(11, 1);

		DefaultResources::GizmoStrandsInstancedColoredProgram->Bind();
		DefaultResources::GizmoStrandsInstancedColoredProgram->SetFloat4x4("model", model);
		DefaultResources::GizmoStrandsInstancedColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
		strands->DrawInstanced(matrices);
		OpenGLUtils::GLVAO::BindDefault();
}


void Gizmos::DrawGizmoMeshInstancedInternal(
				const GizmoSettings &gizmoSettings,
				const std::shared_ptr<Mesh> &mesh,
				const glm::vec4 &color,
				const glm::mat4 &model,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &scaleMatrix) {
		if (mesh == nullptr || matrices.empty())
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();
		mesh->Enable();
		switch (gizmoSettings.m_colorMode) {
				case GizmoSettings::ColorMode::Default: {
						DefaultResources::GizmoInstancedProgram->Bind();
						DefaultResources::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::VertexColor: {
						DefaultResources::GizmoInstancedProgram->Bind();
						DefaultResources::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::NormalColor: {
						DefaultResources::GizmoInstancedProgram->Bind();
						DefaultResources::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
		}
		mesh->DrawInstanced(matrices);
}

void Gizmos::DrawGizmoMeshInstancedColoredInternal(
				const GizmoSettings &gizmoSettings,
				const std::shared_ptr<Mesh> &mesh,
				const std::vector<glm::vec4> &colors,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &model,
				const glm::mat4 &scaleMatrix) {
		if (mesh == nullptr || matrices.empty() || colors.empty() || matrices.size() != colors.size())
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();

		DefaultResources::GizmoInstancedColoredProgram->Bind();
		DefaultResources::GizmoInstancedColoredProgram->SetFloat4x4("model", model);
		DefaultResources::GizmoInstancedColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
		mesh->DrawInstancedColored(colors, matrices);
		OpenGLUtils::GLVAO::BindDefault();
}

void Gizmos::DrawGizmoMeshInternal(
				const GizmoSettings &gizmoSettings,
				const std::shared_ptr<Mesh> &mesh,
				const glm::vec4 &color,
				const glm::mat4 &model,
				const glm::mat4 &scaleMatrix) {
		if (mesh == nullptr)
				return;
		OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, gizmoSettings.m_depthTest);
		gizmoSettings.m_drawSettings.ApplySettings();

		switch (gizmoSettings.m_colorMode) {
				case GizmoSettings::ColorMode::Default: {
						DefaultResources::GizmoProgram->Bind();
						DefaultResources::GizmoProgram->SetFloat4("surfaceColor", color);
						DefaultResources::GizmoProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::VertexColor: {
						DefaultResources::GizmoVertexColoredProgram->Bind();
						DefaultResources::GizmoVertexColoredProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoVertexColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
				case GizmoSettings::ColorMode::NormalColor: {
						DefaultResources::GizmoNormalColoredProgram->Bind();
						DefaultResources::GizmoNormalColoredProgram->SetFloat4x4("model", model);
						DefaultResources::GizmoNormalColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
				}
						break;
		}
		mesh->Draw();
}


#pragma endregion

#pragma region RenderAPI

#pragma region External

void Gizmos::DrawGizmoMeshInstanced(
				const std::shared_ptr<Mesh> &mesh,
				const glm::vec4 &color,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &model,
				const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;

		auto &sceneCamera = editorLayer->m_sceneCamera;
		if (sceneCamera && sceneCamera->IsEnabled()) {
				Camera::m_cameraInfoBlock.UploadMatrices(
								sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
				sceneCamera->Bind();
				DrawGizmoMeshInstancedInternal(gizmoSettings,
																			 mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
		}
}

void Gizmos::DrawGizmoMeshInstancedColored(
				const std::shared_ptr<Mesh> &mesh,
				const std::vector<glm::vec4> &colors,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &model,
				const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;

		auto &sceneCamera = editorLayer->m_sceneCamera;
		if (sceneCamera && sceneCamera->IsEnabled()) {
				Camera::m_cameraInfoBlock.UploadMatrices(
								sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
				sceneCamera->Bind();
				DrawGizmoMeshInstancedColoredInternal(
								gizmoSettings, mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
		}
}

void Gizmos::DrawGizmoMesh(
				const std::shared_ptr<Mesh> &mesh,
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const glm::mat4 &model,
				const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent, cameraPosition, cameraRotation);
		cameraComponent->Bind();
		DrawGizmoMeshInternal(gizmoSettings, mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}


void Gizmos::DrawGizmoMeshInstanced(
				const std::shared_ptr<Mesh> &mesh,
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &model,
				const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent, cameraPosition, cameraRotation);
		cameraComponent->Bind();
		DrawGizmoMeshInstancedInternal(
						gizmoSettings, mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void Gizmos::DrawGizmoMeshInstancedColored(
				const std::shared_ptr<Mesh> &mesh,
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const std::vector<glm::vec4> &colors,
				const std::vector<glm::mat4> &matrices,
				const glm::mat4 &model,
				const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent, cameraPosition, cameraRotation);
		cameraComponent->Bind();
		DrawGizmoMeshInstancedColoredInternal(
						gizmoSettings, mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
}

void Gizmos::DrawGizmoRay(const glm::vec4 &color, const glm::vec3 &start, const glm::vec3 &end, const float &width,
													const GizmoSettings &gizmoSettings) {
		glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		const glm::mat4 rotationMat = glm::mat4_cast(rotation);
		const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
											 glm::scale(glm::vec3(width, glm::distance(end, start), width));
		DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model, 1.0f, gizmoSettings);
}

void Gizmos::DrawGizmoRays(const glm::vec4 &color,
													 const std::vector<glm::vec3> &starts,
													 const std::vector<glm::vec3> &ends,
													 const float &width, const GizmoSettings &gizmoSettings) {
		if (starts.empty() || ends.empty() || starts.size() != ends.size())
				return;
		std::vector<glm::mat4> models;
		models.resize(starts.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						starts.size(),
						[&](unsigned i) {
							auto start = starts[i];
							auto end = ends[i];
							auto direction = glm::normalize(end - start);
							glm::quat rotation = glm::quatLookAt(direction, glm::vec3(direction.y, direction.z, direction.x));
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
																 glm::scale(glm::vec3(width, glm::distance(end, start), width));
							models[i] = model;
						},
						results);
		for (const auto &i: results)
				i.wait();

		DrawGizmoMeshInstanced(DefaultResources::Primitives::Cylinder, color, models, glm::mat4(1.0f), 1.0f, gizmoSettings);
}

void Gizmos::DrawGizmoRays(const std::vector<glm::vec4> &colors,
													 const std::vector<glm::vec3> &starts,
													 const std::vector<glm::vec3> &ends,
													 const float &width, const GizmoSettings &gizmoSettings) {
		if (starts.empty() || ends.empty() || starts.size() != ends.size())
				return;
		std::vector<glm::mat4> models;
		models.resize(starts.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						starts.size(),
						[&](unsigned i) {
							auto start = starts[i];
							auto end = ends[i];
							auto direction = glm::normalize(end - start);
							glm::quat rotation = glm::quatLookAt(direction, glm::vec3(direction.y, direction.z, direction.x));
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
																 glm::scale(glm::vec3(width, glm::distance(end, start), width));
							models[i] = model;
						},
						results);
		for (const auto &i: results)
				i.wait();

		DrawGizmoMeshInstancedColored(DefaultResources::Primitives::Cylinder, colors, models, glm::mat4(1.0f), 1.0f, gizmoSettings);
}

void Gizmos::DrawGizmoRays(
				const glm::vec4 &color, const std::vector<std::pair<glm::vec3, glm::vec3>> &startEnds, const float &width,
				const GizmoSettings &gizmoSettings) {
		if (startEnds.empty())
				return;
		std::vector<glm::mat4> models;
		models.resize(startEnds.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						startEnds.size(),
						[&](unsigned i) {
							auto start = startEnds[i].first;
							auto end = startEnds[i].second;
							auto direction = glm::normalize(end - start);
							glm::quat rotation = glm::quatLookAt(direction, glm::vec3(direction.y, direction.z, direction.x));
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
																 glm::scale(glm::vec3(width, glm::distance(end, start), width));
							models[i] = model;
						},
						results);
		for (const auto &i: results)
				i.wait();

		DrawGizmoMeshInstanced(DefaultResources::Primitives::Cylinder, color, models, glm::mat4(1.0f), 1.0f, gizmoSettings);
}


void Gizmos::DrawGizmoRays(const glm::vec4 &color, const std::vector<Ray> &rays, const float &width,
													 const GizmoSettings &gizmoSettings) {
		if (rays.empty())
				return;
		std::vector<glm::mat4> models;
		models.resize(rays.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						rays.size(),
						[&](unsigned i) {
							auto &ray = rays[i];
							glm::quat rotation = glm::quatLookAt(ray.m_direction,
																									 {ray.m_direction.y, ray.m_direction.z, ray.m_direction.x});
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							const glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
																 glm::scale(glm::vec3(width, ray.m_length, width));
							models[i] = model;
						},
						results);
		for (const auto &i: results)
				i.wait();
		DrawGizmoMeshInstanced(DefaultResources::Primitives::Cylinder, color, models, glm::mat4(1.0f), 1.0f, gizmoSettings);
}

void
Gizmos::DrawGizmoRay(const glm::vec4 &color, const Ray &ray, const float &width, const GizmoSettings &gizmoSettings) {
		glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		const glm::mat4 rotationMat = glm::mat4_cast(rotation);
		const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
											 glm::scale(glm::vec3(width, ray.m_length, width));
		DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model);
}

void Gizmos::DrawGizmoRay(
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const glm::vec3 &start,
				const glm::vec3 &end,
				const float &width, const GizmoSettings &gizmoSettings) {
		glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		const glm::mat4 rotationMat = glm::mat4_cast(rotation);
		const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
											 glm::scale(glm::vec3(width, glm::distance(end, start), width));
		DrawGizmoMesh(
						DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, model, 1.0f,
						gizmoSettings);
}

void Gizmos::DrawGizmoRays(
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
				const float &width, const GizmoSettings &gizmoSettings) {
		if (connections.empty())
				return;
		std::vector<glm::mat4> models;
		models.resize(connections.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						connections.size(), [&](unsigned i) {
							auto start = connections[i].first;
							auto end = connections[i].second;
							auto direction = glm::normalize(end - start);
							glm::quat rotation = glm::quatLookAt(direction, glm::vec3(direction.y, direction.z, direction.x));
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
																 glm::scale(glm::vec3(width, glm::distance(end, start), width));
							models[i] = model;
						}, results);
		for (const auto &i: results)
				i.wait();
		DrawGizmoMeshInstanced(
						DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, models,
						glm::mat4(1.0f), 1.0f, gizmoSettings);
}

void Gizmos::DrawGizmoRays(
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const std::vector<Ray> &rays,
				const float &width, const GizmoSettings &gizmoSettings) {
		if (rays.empty())
				return;
		std::vector<glm::mat4> models;
		models.resize(rays.size());
		std::vector<std::shared_future<void>> results;
		Jobs::ParallelFor(
						rays.size(), [&](unsigned i) {
							auto &ray = rays[i];
							glm::quat rotation = glm::quatLookAt(ray.m_direction,
																									 {ray.m_direction.y, ray.m_direction.z, ray.m_direction.x});
							rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
							const glm::mat4 rotationMat = glm::mat4_cast(rotation);
							const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
																 glm::scale(glm::vec3(width, ray.m_length, width));
							models[i] = model;
						}, results);
		for (const auto &i: results)
				i.wait();
		DrawGizmoMeshInstanced(
						DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, models,
						glm::mat4(1.0f), 1.0f, gizmoSettings);
}

void Gizmos::DrawGizmoRay(
				const std::shared_ptr<Camera> &cameraComponent,
				const glm::vec3 &cameraPosition,
				const glm::quat &cameraRotation,
				const glm::vec4 &color,
				const Ray &ray,
				const float &width, const GizmoSettings &gizmoSettings) {
		glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
		const glm::mat4 rotationMat = glm::mat4_cast(rotation);
		const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
											 glm::scale(glm::vec3(width, ray.m_length, width));
		DrawGizmoMesh(
						DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, model, 1.0f,
						gizmoSettings);
}

void Graphics::DrawMesh(
				const std::shared_ptr<Mesh> &mesh,
				const std::shared_ptr<Material> &material,
				const glm::mat4 &model,
				const std::shared_ptr<Camera> &cameraComponent,
				const bool &receiveShadow,
				const bool &castShadow) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;

		RenderCommand renderCommand;
		renderCommand.m_commandType = RenderCommandType::FromAPI;
		renderCommand.m_geometryType = RenderGeometryType::Mesh;
		renderCommand.m_renderGeometry = mesh;
		renderCommand.m_receiveShadow = receiveShadow;
		renderCommand.m_castShadow = castShadow;
		renderCommand.m_globalTransform.m_value = model;
		auto &group = renderLayer->m_forwardRenderInstances[cameraComponent->GetHandle()];
		group.m_camera = cameraComponent;
		auto &commands = group.m_renderCommandsGroups[material->GetHandle()];
		commands.m_material = material;
		commands.m_renderCommands[mesh->GetHandle()].push_back(
						renderCommand);
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (editorLayer) {
				auto &sceneCamera = editorLayer->m_sceneCamera;
				if (sceneCamera && sceneCamera->IsEnabled()) {
						auto &group = renderLayer->m_forwardRenderInstances[sceneCamera->GetHandle()];
						group.m_camera = sceneCamera;
						auto &commands = group.m_renderCommandsGroups[material->GetHandle()];
						commands.m_material = material;
						commands.m_renderCommands[mesh->GetHandle()].push_back(
										renderCommand);
				}
		}
}

void Graphics::DrawMeshInstanced(
				const std::shared_ptr<Mesh> &mesh,
				const std::shared_ptr<Material> &material,
				const glm::mat4 &model,
				const std::shared_ptr<ParticleMatrices> &matrices,
				const std::shared_ptr<Camera> &cameraComponent,
				const bool &receiveShadow,
				const bool &castShadow) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;

		RenderCommand renderCommand;
		renderCommand.m_commandType = RenderCommandType::FromAPI;
		renderCommand.m_geometryType = RenderGeometryType::Mesh;
		renderCommand.m_renderGeometry = mesh;
		renderCommand.m_matrices = matrices;
		renderCommand.m_receiveShadow = receiveShadow;
		renderCommand.m_castShadow = castShadow;
		renderCommand.m_globalTransform.m_value = model;

		auto &group = renderLayer->m_forwardInstancedRenderInstances[cameraComponent->GetHandle()];
		group.m_camera = cameraComponent;
		auto &commands = group.m_renderCommandsGroups[material->GetHandle()];
		commands.m_material = material;
		commands.m_renderCommands[mesh->GetHandle()].push_back(
						renderCommand);
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (editorLayer) {
				auto &sceneCamera = editorLayer->m_sceneCamera;
				if (sceneCamera && sceneCamera->IsEnabled()) {
						auto &group = renderLayer->m_forwardInstancedRenderInstances[sceneCamera->GetHandle()];
						group.m_camera = sceneCamera;
						auto &commands = group.m_renderCommandsGroups[material->GetHandle()];
						commands.m_material = material;
						commands.m_renderCommands[mesh->GetHandle()]
										.push_back(renderCommand);
				}
		}
}

/*
#pragma region DrawTexture
void Graphics::DrawTexture2D(
	const OpenGLUtils::GLTexture2D *texture,
	const float &depth,
	const glm::vec2 &center,
	const glm::vec2 &size,
	const RenderTarget *target)
{
	target->Bind();
	DrawTexture2D(texture, depth, center, size);
}

void Graphics::DrawTexture2D(
	const Texture2D *texture,
	const float &depth,
	const glm::vec2 &center,
	const glm::vec2 &size,
	const Camera &cameraComponent)
{
	if (Editor::GetInstance().m_enabled)
	{
		auto &sceneCamera = Editor::GetInstance().m_sceneCamera;
		if (&cameraComponent != &sceneCamera && sceneCamera.IsEnabled())
		{
			Camera::m_cameraInfoBlock.UploadMatrices(
				sceneCamera,
				Editor::GetInstance().m_sceneCameraPosition,
				Editor::GetInstance().m_sceneCameraRotation);
			Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
			sceneCamera.Bind();
			DrawTexture2D(texture->Texture().get(), depth, center, size);
		}
	}
	if (!cameraComponent.IsEnabled())
		return;
	const auto entity = cameraComponent.GetOwner();
	if (!entity.IsEnabled())
		return;
	cameraComponent.Bind();
	DrawTexture2D(texture->Texture().get(), depth, center, size);
}
void Graphics::DrawTexture2D(
	const OpenGLUtils::GLTexture2D *texture, const float &depth, const glm::vec2 &center, const glm::vec2 &size)
	{
	const auto program = DefaultResources::GLPrograms::ScreenProgram;
	program->Bind();
	DefaultResources::GLPrograms::ScreenVAO->Bind();
	texture->Bind(0);
	program->SetInt("screenTexture", 0);
	program->SetFloat("depth", depth);
	program->SetFloat2("center", center);
	program->SetFloat2("size", size);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	}
void Graphics::DrawTexture2D(
	const Texture2D *texture,
	const float &depth,
	const glm::vec2 &center,
	const glm::vec2 &size,
	const RenderTarget *target)
{
	target->Bind();
	DrawTexture2D(texture->Texture().get(), depth, center, size);
}
*/

#pragma endregion
#pragma region Gizmo

void Gizmos::DrawGizmoMesh(
				const std::shared_ptr<Mesh> &mesh, const glm::vec4 &color, const glm::mat4 &model, const float &size,
				const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		auto &sceneCamera = editorLayer->m_sceneCamera;
		if (sceneCamera && sceneCamera->IsEnabled()) {
				Camera::m_cameraInfoBlock.UploadMatrices(
								sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
				sceneCamera->Bind();
				DrawGizmoMeshInternal(gizmoSettings, mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
		}
}

void Gizmos::DrawGizmoStrands(const std::shared_ptr<Strands> &strands, const glm::vec4 &color, const glm::mat4 &model,
															const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		auto &sceneCamera = editorLayer->m_sceneCamera;
		if (sceneCamera && sceneCamera->IsEnabled()) {
				Camera::m_cameraInfoBlock.UploadMatrices(
								sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
				sceneCamera->Bind();
				DrawGizmoStrandsInternal(gizmoSettings, strands, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
		}
}

void Gizmos::DrawGizmoStrands(const std::shared_ptr<Strands> &strands, const std::shared_ptr<Camera> &cameraComponent,
															const glm::vec3 &cameraPosition, const glm::quat &cameraRotation, const glm::vec4 &color,
															const glm::mat4 &model, const float &size, const GizmoSettings &gizmoSettings) {
		auto renderLayer = Application::GetLayer<RenderLayer>();
		if (!renderLayer)
				return;
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (!editorLayer)
				return;
		if (cameraComponent && cameraComponent->IsEnabled()) {
				Camera::m_cameraInfoBlock.UploadMatrices(
								cameraComponent, cameraPosition, cameraRotation);
				cameraComponent->Bind();
				DrawGizmoStrandsInternal(gizmoSettings, strands, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
		}
}

#pragma endregion
#pragma endregion

#pragma endregion