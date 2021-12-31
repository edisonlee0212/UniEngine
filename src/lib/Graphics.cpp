#include "EditorLayer.hpp"
#include "RenderLayer.hpp"
#include <Application.hpp>
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <LightProbe.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
#include <ReflectionProbe.hpp>
#include "Engine/Utilities/Graphics.hpp"
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;

#pragma region Helpers
unsigned int environmentalMapCubeVAO = 0;
unsigned int environmentalMapCubeVBO = 0;
void Graphics::RenderCube()
{
    // initialize (if necessary)
    if (environmentalMapCubeVAO == 0)
    {
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
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
void Graphics::RenderQuad()
{
    if (environmentalMapQuadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &environmentalMapQuadVAO);
        glGenBuffers(1, &environmentalMapQuadVBO);
        glBindVertexArray(environmentalMapQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, environmentalMapQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(environmentalMapQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
#pragma endregion

#pragma region RenderAPI

#pragma region External
void Graphics::DrawGizmoMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const glm::vec4 &color,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;

    auto &sceneCamera = editorLayer->m_sceneCamera;
    if (sceneCamera && sceneCamera->IsEnabled())
    {
        Camera::m_cameraInfoBlock.UpdateMatrices(
            sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
        Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
        sceneCamera->Bind();
        renderLayer->DrawGizmoMeshInstanced(
            true, mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
    }
}

void Graphics::DrawGizmoMeshInstancedColored(
    const std::shared_ptr<Mesh> &mesh,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;

    auto &sceneCamera = editorLayer->m_sceneCamera;
    if (sceneCamera && sceneCamera->IsEnabled())
    {
        Camera::m_cameraInfoBlock.UpdateMatrices(
            sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
        Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
        sceneCamera->Bind();
        renderLayer->DrawGizmoMeshInstancedColored(true, mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
    }
}

void Graphics::DrawGizmoMesh(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    renderLayer->DrawGizmoMesh(true, mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void Graphics::DrawGizmoMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    renderLayer->DrawGizmoMeshInstanced(
        true, mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void Graphics::DrawGizmoMeshInstancedColored(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    renderLayer->DrawGizmoMeshInstancedColored(true, mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
}

void Graphics::DrawGizmoRay(
    const glm::vec4 &color, const glm::vec3 &start, const glm::vec3 &end, const float &width)
{
    glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                       glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
    DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model);
}

void Graphics::DrawGizmoRays(
    const glm::vec4 &color, const std::vector<std::pair<glm::vec3, glm::vec3>> &connections, const float &width)
{
    if (connections.empty())
        return;
    std::vector<glm::mat4> models;
    models.resize(connections.size());
    for (int i = 0; i < connections.size(); i++)
    {
        auto start = connections[i].first;
        auto &end = connections[i].second;
        glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                           glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
        models[i] = model;
    }

    DrawGizmoMeshInstanced(DefaultResources::Primitives::Cylinder, color, models);
}

void Graphics::DrawGizmoRays(const glm::vec4 &color, const std::vector<Ray> &rays, const float &width)
{
    if (rays.empty())
        return;
    std::vector<glm::mat4> models;
    models.resize(rays.size());
    for (int i = 0; i < rays.size(); i++)
    {
        auto &ray = rays[i];
        glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        const glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                           glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
        models[i] = model;
    }
    DrawGizmoMeshInstanced(DefaultResources::Primitives::Cylinder, color, models);
}

void Graphics::DrawGizmoRay(const glm::vec4 &color, const Ray &ray, const float &width)
{
    glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                       glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
    DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model);
}

void Graphics::DrawGizmoRay(
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::vec3 &start,
    const glm::vec3 &end,
    const float &width)
{
    glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                       glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
    DrawGizmoMesh(
        DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, model);
}

void Graphics::DrawGizmoRays(
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
    const float &width)
{
    if (connections.empty())
        return;
    std::vector<glm::mat4> models;
    models.resize(connections.size());
    for (int i = 0; i < connections.size(); i++)
    {
        auto start = connections[i].first;
        auto &end = connections[i].second;
        glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                           glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
        models[i] = model;
    }
    DrawGizmoMeshInstanced(
        DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, models);
}

void Graphics::DrawGizmoRays(
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const std::vector<Ray> &rays,
    const float &width)
{
    if (rays.empty())
        return;
    std::vector<glm::mat4> models;
    models.resize(rays.size());
    for (int i = 0; i < rays.size(); i++)
    {
        auto &ray = rays[i];
        glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        const glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                           glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
        models[i] = model;
    }
    DrawGizmoMeshInstanced(
        DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, models);
}

void Graphics::DrawGizmoRay(
    const std::shared_ptr<Camera> &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const Ray &ray,
    const float &width)
{
    glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                       glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
    DrawGizmoMesh(
        DefaultResources::Primitives::Cylinder, cameraComponent, cameraPosition, cameraRotation, color, model);
}

void Graphics::DrawMesh(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const std::shared_ptr<Camera> &cameraComponent,
    const bool &receiveShadow,
    const bool &castShadow)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;

    RenderCommand renderCommand;
    renderCommand.m_commandType = RenderCommandType::FromAPI;
    renderCommand.m_meshType = RenderCommandMeshType::Default;
    renderCommand.m_mesh = mesh;
    renderCommand.m_receiveShadow = receiveShadow;
    renderCommand.m_castShadow = castShadow;
    renderCommand.m_globalTransform.m_value = model;
    renderLayer->m_forwardRenderInstances[cameraComponent].m_value[material].m_meshes[mesh->m_vao].push_back(
        renderCommand);
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (editorLayer)
    {
        auto &sceneCamera = editorLayer->m_sceneCamera;
        if (sceneCamera && sceneCamera->IsEnabled())
        {
            renderLayer->m_forwardRenderInstances[sceneCamera].m_value[material].m_meshes[mesh->m_vao].push_back(
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
    const bool &castShadow)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;

    RenderCommand renderCommand;
    renderCommand.m_commandType = RenderCommandType::FromAPI;
    renderCommand.m_meshType = RenderCommandMeshType::Default;
    renderCommand.m_mesh = mesh;
    renderCommand.m_matrices = matrices;
    renderCommand.m_receiveShadow = receiveShadow;
    renderCommand.m_castShadow = castShadow;
    renderCommand.m_globalTransform.m_value = model;
    renderLayer->m_forwardInstancedRenderInstances[cameraComponent].m_value[material].m_meshes[mesh->m_vao].push_back(
        renderCommand);
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (editorLayer)
    {
        auto &sceneCamera = editorLayer->m_sceneCamera;
        if (sceneCamera && sceneCamera->IsEnabled())
        {
            renderLayer->m_forwardInstancedRenderInstances[sceneCamera]
                .m_value[material]
                .m_meshes[mesh->m_vao]
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
    if (EditorManager::GetInstance().m_enabled)
    {
        auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
        if (&cameraComponent != &sceneCamera && sceneCamera.IsEnabled())
        {
            Camera::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera,
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
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

void Graphics::DrawGizmoMesh(
    const std::shared_ptr<Mesh> &mesh, const glm::vec4 &color, const glm::mat4 &model, const float &size)
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (!editorLayer)
        return;
    auto &sceneCamera = editorLayer->m_sceneCamera;
    if (sceneCamera && sceneCamera->IsEnabled())
    {
        Camera::m_cameraInfoBlock.UpdateMatrices(
            sceneCamera, editorLayer->m_sceneCameraPosition, editorLayer->m_sceneCameraRotation);
        Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
        sceneCamera->Bind();
        renderLayer->DrawGizmoMesh(true, mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
    }
}
#pragma endregion
#pragma endregion

#pragma endregion