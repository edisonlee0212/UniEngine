//
// Created by lllll on 11/1/2021.
//

#include "RenderLayer.hpp"
#include "EditorLayer.hpp"
#include "ProfilerLayer.hpp"
#include "Graphics.hpp"
#include <Application.hpp>
#include <ProjectManager.hpp>
#include <Camera.hpp>
#include <Cubemap.hpp>
#include <DefaultResources.hpp>
#include "Editor.hpp"
#include "Engine/Core/Inputs.hpp"
#include <LightProbe.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
#include <ReflectionProbe.hpp>
#include "Engine/Rendering/Graphics.hpp"
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;

#pragma region RenderCommand Dispatch
void RenderLayer::DispatchRenderCommands(
    const RenderCommands &renderCommands,
    const std::function<void(const std::shared_ptr<Material> &, const RenderCommand &renderCommand)> &func,
    const bool &setMaterial)
{

    for (const auto &renderCollection : renderCommands.m_value)
    {
        const auto &material = renderCollection.first;
        if (material.expired())
            continue;
        auto mat = material.lock();
        if (setMaterial)
        {
            MaterialPropertySetter(mat, true);
            m_materialSettings = MaterialSettingsBlock();
            ApplyMaterialSettings(mat);
        }
        for (const auto &renderCommands : renderCollection.second.m_meshes)
        {
            for (const auto &renderCommand : renderCommands.second)
            {
                if (renderCommand.m_mesh.expired())
                    continue;
                func(mat, renderCommand);
            }
        }
        for (const auto &renderCommands : renderCollection.second.m_skinnedMeshes)
        {
            for (const auto &renderCommand : renderCommands.second)
            {
                if (renderCommand.m_skinnedMesh.expired() || renderCommand.m_boneMatrices.expired())
                    continue;
                func(mat, renderCommand);
            }
        }
        if (setMaterial)
            ReleaseMaterialSettings(mat);
    }
}

#pragma endregion
void RenderLayer::RenderToCamera(const std::shared_ptr<Camera> &cameraComponent, const GlobalTransform &cameraModel)
{
    /*
    if (cameraComponent->m_frameCount == 0)
    {
        cameraComponent->m_frameCount++;
        return;
    }
    cameraComponent->m_frameCount++;
    */
    auto scene = GetScene();
    auto sceneBound = scene->GetBound();
    RenderShadows(sceneBound, cameraComponent, cameraModel);
    ApplyShadowMapSettings();
    ApplyEnvironmentalSettings(cameraComponent);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    cameraComponent->m_gBuffer->Bind();
    cameraComponent->m_gBuffer->GetFrameBuffer()->DrawBuffers(
        {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
    cameraComponent->m_gBuffer->Clear();
    DispatchRenderCommands(
        m_deferredRenderInstances[cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                auto mesh = renderCommand.m_mesh.lock();
                auto &program = DefaultResources::m_gBufferPrepass;
                program->Bind();
                ApplyProgramSettings(program, material);
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DeferredPrepassInternal(mesh);
                break;
            }
            case RenderCommandMeshType::Skinned: {
                auto skinnedMesh = renderCommand.m_skinnedMesh.lock();
                auto &program = DefaultResources::m_gBufferSkinnedPrepass;
                program->Bind();
                ApplyProgramSettings(program, material);
                renderCommand.m_boneMatrices.lock()->UploadBones(skinnedMesh);
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DeferredPrepassInternal(skinnedMesh);
                break;
            }
            }
        },
        true);
    DispatchRenderCommands(
        m_deferredInstancedRenderInstances[cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                if (renderCommand.m_matrices.expired())
                    break;
                auto mesh = renderCommand.m_mesh.lock();
                auto &program = DefaultResources::m_gBufferInstancedPrepass;
                program->Bind();
                ApplyProgramSettings(program, material);
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DeferredPrepassInstancedInternal(mesh, renderCommand.m_matrices.lock());
                break;
            }
            }
        },
        true);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
#pragma region Copy Depth Buffer back to camera
    auto res = cameraComponent->GetResolution();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, cameraComponent->m_gBuffer->GetFrameBuffer()->Id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cameraComponent->GetFrameBuffer()->Id()); // write to default framebuffer
    glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#pragma endregion

    cameraComponent->m_frameBuffer->DrawBuffers({GL_COLOR_ATTACHMENT0});
#pragma region Apply GBuffer with lighting
    DefaultResources::m_gBufferLightingPass->Bind();
    cameraComponent->m_gBufferDepth->Bind(12);
    cameraComponent->m_gBufferNormal->Bind(13);
    cameraComponent->m_gBufferAlbedo->Bind(14);
    cameraComponent->m_gBufferMetallicRoughnessEmissionAmbient->Bind(15);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_DIRECTIONAL_LIGHT_SM", 0);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_POINT_LIGHT_SM", 1);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_SPOT_LIGHT_SM", 2);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_SKYBOX", 8);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_IRRADIANCE", 9);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_PREFILERED", 10);
    DefaultResources::m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_BRDFLUT", 11);

    DefaultResources::m_gBufferLightingPass->SetInt("gDepth", 12);
    DefaultResources::m_gBufferLightingPass->SetInt("gNormal", 13);
    DefaultResources::m_gBufferLightingPass->SetInt("gAlbedo", 14);
    DefaultResources::m_gBufferLightingPass->SetInt("gMetallicRoughnessEmissionAmbient", 15);
    DefaultResources::ScreenVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
#pragma endregion
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
#pragma region Forward rendering
    DispatchRenderCommands(
        m_forwardRenderInstances[cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                auto mesh = renderCommand.m_mesh.lock();
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                auto program = material->m_program.Get<OpenGLUtils::GLProgram>();
                if (!program)
                    break;
                program->Bind();
                ApplyProgramSettings(program, material);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DrawMeshInternal(mesh);
                break;
            }
            case RenderCommandMeshType::Skinned: {
                auto skinnedMesh = renderCommand.m_skinnedMesh.lock();
                renderCommand.m_boneMatrices.lock()->UploadBones(skinnedMesh);
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                auto program = material->m_program.Get<OpenGLUtils::GLProgram>();
                if (!program)
                    break;
                program->Bind();
                ApplyProgramSettings(program, material);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DrawMeshInternal(skinnedMesh);
                break;
            }
            }
        },
        true);
    DispatchRenderCommands(
        m_forwardInstancedRenderInstances[cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                if (renderCommand.m_matrices.expired())
                    break;
                auto mesh = renderCommand.m_mesh.lock();
                m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
                auto program = material->m_program.Get<OpenGLUtils::GLProgram>();
                if (!program)
                    break;
                program->Bind();
                ApplyProgramSettings(program, material);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DrawMeshInstancedInternal(mesh, renderCommand.m_matrices.lock());
                break;
            }
            }
        },
        true);
#pragma endregion
#pragma region Transparent
#pragma endregion
#pragma region Environment
    glDepthFunc(
        GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
    DefaultResources::SkyboxProgram->Bind();
    DefaultResources::SkyboxVAO->Bind();

    DefaultResources::SkyboxProgram->SetInt("UE_SKYBOX", 8);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS); // set depth function back to default
#pragma endregion

    cameraComponent->m_rendered = true;
    cameraComponent->m_requireRendering = false;
}
void RenderLayer::PreUpdate()
{
    ProfilerLayer::StartEvent("Graphics");

    ProfilerLayer::StartEvent("Clear GBuffer");
    m_deferredRenderInstances.clear();
    m_deferredInstancedRenderInstances.clear();
    m_forwardRenderInstances.clear();
    m_forwardInstancedRenderInstances.clear();
    m_transparentRenderInstances.clear();
    m_instancedTransparentRenderInstances.clear();
    ProfilerLayer::EndEvent("Clear GBuffer");
    ProfilerLayer::EndEvent("Graphics");
}
void RenderLayer::LateUpdate()
{
    ProfilerLayer::StartEvent("Graphics");
    auto scene = GetScene();
    if (!scene)
        return;

    std::shared_ptr<Camera> mainCamera = scene->m_mainCamera.Get<Camera>();
#pragma region Collect RenderCommands
    ProfilerLayer::StartEvent("RenderCommand Collection");
    Bound worldBound;
    CollectRenderInstances(worldBound);
    ProfilerLayer::EndEvent("RenderCommand Collection");
    scene->SetBound(worldBound);
#pragma endregion
#pragma region Render to cameras
    ProfilerLayer::StartEvent("Main Rendering");
    m_triangles = 0;
    m_drawCall = 0;
    if (mainCamera)
    {
        if (mainCamera->m_allowAutoResize)
            mainCamera->ResizeResolution(m_mainCameraResolutionX, m_mainCameraResolutionY);
    }
    const std::vector<Entity> *cameraEntities =
        scene->UnsafeGetPrivateComponentOwnersList<Camera>();
    if (cameraEntities != nullptr)
    {
        for (auto cameraEntity : *cameraEntities)
        {
            assert(scene->HasPrivateComponent<Camera>(cameraEntity));
            auto cameraComponent = scene->GetOrSetPrivateComponent<Camera>(cameraEntity).lock();
            cameraComponent->m_rendered = false;
            if (cameraComponent->m_requireRendering)
            {
                auto ltw = scene->GetDataComponent<GlobalTransform>(cameraEntity);
                Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, ltw.GetPosition(), ltw.GetRotation());
                Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
                RenderToCamera(cameraComponent, scene->GetDataComponent<GlobalTransform>(cameraEntity));
            }
        }
    }
    ProfilerLayer::EndEvent("Main Rendering");
#pragma endregion
#pragma region Post - processing
    ProfilerLayer::StartEvent("Post Processing");
    const std::vector<Entity> *postProcessingEntities =
        scene->UnsafeGetPrivateComponentOwnersList<PostProcessing>();
    if (postProcessingEntities != nullptr)
    {
        for (auto postProcessingEntity : *postProcessingEntities)
        {
            if (!scene->IsEntityEnabled(postProcessingEntity))
                continue;
            auto postProcessing = scene->GetOrSetPrivateComponent<PostProcessing>(postProcessingEntity).lock();
            if (postProcessing->IsEnabled())
                postProcessing->Process();
        }
    }
    ProfilerLayer::EndEvent("Post Processing");
#pragma endregion
    m_frameIndex++;
    ProfilerLayer::EndEvent("Graphics");
}
glm::vec3 RenderLayer::ClosestPointOnLine(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b)
{
    const float lineLength = distance(a, b);
    const glm::vec3 vector = point - a;
    const glm::vec3 lineDirection = (b - a) / lineLength;

    // Project Vector to LineDirection to get the distance of point from a
    const float distance = dot(vector, lineDirection);
    return a + lineDirection * distance;
}
void RenderLayer::OnInspect()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Render Settings", &m_enableRenderMenu);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_enableRenderMenu)
    {
        ImGui::Begin("Render Settings");
        ImGui::DragFloat("Gamma", &m_lightSettings.m_gamma, 0.01f, 1.0f, 3.0f);

        bool enableShadow = m_materialSettings.m_enableShadow;
        if (ImGui::Checkbox("Enable shadow", &enableShadow))
        {
            m_materialSettings.m_enableShadow = enableShadow;
        }
        if (m_materialSettings.m_enableShadow && ImGui::CollapsingHeader("Shadow", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNode("Distance"))
            {
                ImGui::DragFloat("Max shadow distance", &m_maxShadowDistance, 1.0f, 0.1f);
                ImGui::DragFloat("Split 1", &m_shadowCascadeSplit[0], 0.01f, 0.0f, m_shadowCascadeSplit[1]);
                ImGui::DragFloat(
                    "Split 2", &m_shadowCascadeSplit[1], 0.01f, m_shadowCascadeSplit[0], m_shadowCascadeSplit[2]);
                ImGui::DragFloat(
                    "Split 3", &m_shadowCascadeSplit[2], 0.01f, m_shadowCascadeSplit[1], m_shadowCascadeSplit[3]);
                ImGui::DragFloat("Split 4", &m_shadowCascadeSplit[3], 0.01f, m_shadowCascadeSplit[2], 1.0f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PCSS"))
            {
                ImGui::DragInt("Blocker search side amount", &m_lightSettings.m_blockerSearchAmount, 1, 1, 8);
                ImGui::DragInt("PCF Sample Size", &m_lightSettings.m_pcfSampleAmount, 1, 1, 64);
                ImGui::TreePop();
            }
            ImGui::DragFloat("Seam fix ratio", &m_lightSettings.m_seamFixRatio, 0.001f, 0.0f, 0.1f);
            ImGui::Checkbox("Stable fit", &m_stableFit);
        }
        ImGui::End();
    }
}
void RenderLayer::OnCreate()
{

    m_frameIndex = 0;
    Camera::GenerateMatrices();
    m_materialSettingsBuffer = std::make_unique<OpenGLUtils::GLUBO>();
    m_materialSettingsBuffer->SetData(sizeof(MaterialSettingsBlock), nullptr, GL_STREAM_DRAW);
    m_materialSettingsBuffer->SetBase(6);

    m_environmentalMapSettingsBuffer = std::make_unique<OpenGLUtils::GLUBO>();
    m_environmentalMapSettingsBuffer->SetData(sizeof(EnvironmentalMapSettingsBlock), nullptr, GL_STREAM_DRAW);
    m_environmentalMapSettingsBuffer->SetBase(7);
    SkinnedMesh::GenerateMatrices();

    PrepareBrdfLut();

    Mesh::m_matricesBuffer = std::make_unique<OpenGLUtils::GLVBO>();
    SkinnedMesh::m_matricesBuffer = std::make_unique<OpenGLUtils::GLVBO>();
#pragma region Kernel Setup
    std::vector<glm::vec4> uniformKernel;
    std::vector<glm::vec4> gaussianKernel;
    for (unsigned int i = 0; i < DefaultResources::ShaderIncludes::MaxKernelAmount; i++)
    {
        uniformKernel.emplace_back(glm::vec4(glm::ballRand(1.0f), 1.0f));
        gaussianKernel.emplace_back(
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f));
    }
    m_kernelBlock = std::make_unique<OpenGLUtils::GLUBO>();
    m_kernelBlock->SetBase(5);
    m_kernelBlock->SetData(
        sizeof(glm::vec4) * uniformKernel.size() + sizeof(glm::vec4) * gaussianKernel.size(), NULL, GL_STATIC_DRAW);
    m_kernelBlock->SubData(0, sizeof(glm::vec4) * uniformKernel.size(), uniformKernel.data());
    m_kernelBlock->SubData(
        sizeof(glm::vec4) * uniformKernel.size(), sizeof(glm::vec4) * gaussianKernel.size(), gaussianKernel.data());

#pragma endregion
#pragma region Shadow
    m_shadowCascadeInfoBlock.SetData(sizeof(LightSettingsBlock), nullptr, GL_DYNAMIC_DRAW);
    m_shadowCascadeInfoBlock.SetBase(4);

#pragma region LightInfoBlocks
    size_t size = 16 + DefaultResources::ShaderIncludes::MaxDirectionalLightAmount * sizeof(DirectionalLightInfo);
    m_directionalLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    m_directionalLightBlock.SetBase(1);
    size = 16 + DefaultResources::ShaderIncludes::MaxPointLightAmount * sizeof(PointLightInfo);
    m_pointLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    m_pointLightBlock.SetBase(2);
    size = 16 + DefaultResources::ShaderIncludes::MaxSpotLightAmount * sizeof(SpotLightInfo);
    m_spotLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    m_spotLightBlock.SetBase(3);
#pragma endregion
#pragma region DirectionalLight
    m_directionalLightShadowMap = std::make_unique<DirectionalLightShadowMap>(m_directionalLightShadowMapResolution);
#pragma region PointLight
    m_pointLightShadowMap = std::make_unique<PointLightShadowMap>(m_pointLightShadowMapResolution);
#pragma endregion
#pragma region SpotLight
    m_spotLightShadowMap = std::make_unique<SpotLightShadowMap>(m_spotLightShadowMapResolution);
#pragma endregion
#pragma endregion
}
void RenderLayer::CollectRenderInstances(Bound &worldBound)
{
    auto scene = GetScene();
    auto &editorManager = Editor::GetInstance();
    std::vector<std::pair<std::shared_ptr<Camera>, glm::vec3>> cameraPairs;
    auto editorLayer = Application::GetLayer<EditorLayer>();
    if (editorLayer)
    {
        auto &sceneCamera = editorLayer->m_sceneCamera;
        if (sceneCamera && sceneCamera->IsEnabled())
        {
            cameraPairs.emplace_back(sceneCamera, editorLayer->m_sceneCameraPosition);
        }
    }
    const std::vector<Entity> *cameraEntities =
        scene->UnsafeGetPrivateComponentOwnersList<Camera>();
    if (cameraEntities)
    {
        for (const auto &i : *cameraEntities)
        {
            if (!scene->IsEntityEnabled(i))
                continue;
            assert(scene->HasPrivateComponent<Camera>(i));
            auto camera = scene->GetOrSetPrivateComponent<Camera>(i).lock();
            if (!camera || !camera->IsEnabled())
                continue;
            cameraPairs.emplace_back(camera, scene->GetDataComponent<GlobalTransform>(i).GetPosition());
        }
    }
    auto &minBound = worldBound.m_min;
    auto &maxBound = worldBound.m_max;
    minBound = glm::vec3(INT_MAX);
    maxBound = glm::vec3(INT_MIN);

    const std::vector<Entity> *owners =
        scene->UnsafeGetPrivateComponentOwnersList<MeshRenderer>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!scene->IsEntityEnabled(owner))
                continue;
            auto mmc = scene->GetOrSetPrivateComponent<MeshRenderer>(owner).lock();
            auto material = mmc->m_material.Get<Material>();
            auto mesh = mmc->m_mesh.Get<Mesh>();
            if (!mmc->IsEnabled() || material == nullptr || mesh == nullptr)
                continue;
            auto gt = scene->GetDataComponent<GlobalTransform>(owner);
            auto ltw = gt.m_value;
            auto meshBound = mesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();

            glm::vec3 size = meshBound.Size();
            minBound = glm::vec3(
                (glm::min)(minBound.x, center.x - size.x),
                (glm::min)(minBound.y, center.y - size.y),
                (glm::min)(minBound.z, center.z - size.z));
            maxBound = glm::vec3(
                (glm::max)(maxBound.x, center.x + size.x),
                (glm::max)(maxBound.y, center.y + size.y),
                (glm::max)(maxBound.z, center.z + size.z));

            auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
            for (const auto &pair : cameraPairs)
            {
                auto &deferredRenderInstances = m_deferredRenderInstances[pair.first].m_value;
                auto &deferredInstancedRenderInstances = m_deferredInstancedRenderInstances[pair.first].m_value;
                auto &forwardRenderInstances = m_forwardRenderInstances[pair.first].m_value;
                auto &forwardInstancedRenderInstances = m_forwardInstancedRenderInstances[pair.first].m_value;
                auto &transparentRenderInstances = m_transparentRenderInstances[pair.first].m_value;
                auto &instancedTransparentRenderInstances = m_instancedTransparentRenderInstances[pair.first].m_value;

                float distance = glm::distance(glm::vec3(meshCenter), pair.second);
                RenderCommand renderInstance;
                renderInstance.m_owner = owner;
                renderInstance.m_globalTransform = gt;
                renderInstance.m_mesh = mesh;
                renderInstance.m_castShadow = mmc->m_castShadow;
                renderInstance.m_receiveShadow = mmc->m_receiveShadow;
                renderInstance.m_meshType = RenderCommandMeshType::Default;
                if (material->m_blendingMode != MaterialBlendingMode::Off)
                {
                    transparentRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
                else if (mmc->m_forwardRendering)
                {
                    forwardRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
                else
                {
                    deferredRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
            }
        }
    }
    owners = scene->UnsafeGetPrivateComponentOwnersList<Particles>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!scene->IsEntityEnabled(owner))
                continue;
            auto particles = scene->GetOrSetPrivateComponent<Particles>(owner).lock();
            auto material = particles->m_material.Get<Material>();
            auto mesh = particles->m_mesh.Get<Mesh>();
            if (!particles->IsEnabled() || material == nullptr || mesh == nullptr)
                continue;
            auto gt = scene->GetDataComponent<GlobalTransform>(owner);
            auto ltw = gt.m_value;
            auto meshBound = mesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();

            glm::vec3 size = meshBound.Size();
            minBound = glm::vec3(
                (glm::min)(minBound.x, center.x - size.x),
                (glm::min)(minBound.y, center.y - size.y),
                (glm::min)(minBound.z, center.z - size.z));

            maxBound = glm::vec3(
                (glm::max)(maxBound.x, center.x + size.x),
                (glm::max)(maxBound.y, center.y + size.y),
                (glm::max)(maxBound.z, center.z + size.z));
            for (const auto &pair : cameraPairs)
            {
                auto &deferredRenderInstances = m_deferredRenderInstances[pair.first].m_value;
                auto &deferredInstancedRenderInstances = m_deferredInstancedRenderInstances[pair.first].m_value;
                auto &forwardRenderInstances = m_forwardRenderInstances[pair.first].m_value;
                auto &forwardInstancedRenderInstances = m_forwardInstancedRenderInstances[pair.first].m_value;
                auto &transparentRenderInstances = m_transparentRenderInstances[pair.first].m_value;
                auto &instancedTransparentRenderInstances = m_instancedTransparentRenderInstances[pair.first].m_value;
                auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
                float distance = glm::distance(glm::vec3(meshCenter), pair.second);
                RenderCommand renderInstance;
                renderInstance.m_owner = owner;
                renderInstance.m_globalTransform = gt;
                renderInstance.m_mesh = mesh;
                renderInstance.m_castShadow = particles->m_castShadow;
                renderInstance.m_receiveShadow = particles->m_receiveShadow;
                renderInstance.m_matrices = particles->m_matrices;
                renderInstance.m_meshType = RenderCommandMeshType::Default;
                if (material->m_blendingMode != MaterialBlendingMode::Off)
                {
                    instancedTransparentRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
                else if (particles->m_forwardRendering)
                {
                    forwardInstancedRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
                else
                {
                    deferredInstancedRenderInstances[material].m_meshes[mesh->m_vao].push_back(renderInstance);
                }
            }
        }
    }
    owners = scene->UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!scene->IsEntityEnabled(owner))
                continue;
            auto smmc = scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(owner).lock();
            auto material = smmc->m_material.Get<Material>();
            auto skinnedMesh = smmc->m_skinnedMesh.Get<SkinnedMesh>();
            if (!smmc->IsEnabled() || material == nullptr || skinnedMesh == nullptr)
                continue;
            GlobalTransform gt;
            auto animator = smmc->m_animator.Get<Animator>();
            if (!animator)
            {
                continue;
            }
            if (!smmc->m_ragDoll)
            {
                gt = scene->GetDataComponent<GlobalTransform>(owner);
            }
            auto ltw = gt.m_value;
            auto meshBound = skinnedMesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();

            glm::vec3 size = meshBound.Size();
            minBound = glm::vec3(
                (glm::min)(minBound.x, center.x - size.x),
                (glm::min)(minBound.y, center.y - size.y),
                (glm::min)(minBound.z, center.z - size.z));
            maxBound = glm::vec3(
                (glm::max)(maxBound.x, center.x + size.x),
                (glm::max)(maxBound.y, center.y + size.y),
                (glm::max)(maxBound.z, center.z + size.z));
            for (const auto &pair : cameraPairs)
            {
                auto &deferredRenderInstances = m_deferredRenderInstances[pair.first].m_value;
                auto &deferredInstancedRenderInstances = m_deferredInstancedRenderInstances[pair.first].m_value;
                auto &forwardRenderInstances = m_forwardRenderInstances[pair.first].m_value;
                auto &forwardInstancedRenderInstances = m_forwardInstancedRenderInstances[pair.first].m_value;
                auto &transparentRenderInstances = m_transparentRenderInstances[pair.first].m_value;
                auto &instancedTransparentRenderInstances = m_instancedTransparentRenderInstances[pair.first].m_value;
                auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
                float distance = glm::distance(glm::vec3(meshCenter), pair.second);
                RenderCommand renderInstance;
                renderInstance.m_owner = owner;
                renderInstance.m_globalTransform = gt;
                renderInstance.m_skinnedMesh = skinnedMesh;
                renderInstance.m_castShadow = smmc->m_castShadow;
                renderInstance.m_receiveShadow = smmc->m_receiveShadow;
                renderInstance.m_meshType = RenderCommandMeshType::Skinned;
                renderInstance.m_boneMatrices = smmc->m_finalResults;
                if (material->m_blendingMode != MaterialBlendingMode::Off)
                {
                    transparentRenderInstances[material].m_skinnedMeshes[skinnedMesh->m_vao].push_back(renderInstance);
                }
                else if (smmc->m_forwardRendering)
                {
                    forwardRenderInstances[material].m_skinnedMeshes[skinnedMesh->m_vao].push_back(renderInstance);
                }
                else
                {
                    deferredRenderInstances[material].m_skinnedMeshes[skinnedMesh->m_vao].push_back(renderInstance);
                }
            }
        }
    }
}

inline float RenderLayer::Lerp(const float &a, const float &b, const float &f)
{
    return a + f * (b - a);
}
#pragma region Shadow
void RenderLayer::SetSplitRatio(const float &r1, const float &r2, const float &r3, const float &r4)
{
    m_shadowCascadeSplit[0] = r1;
    m_shadowCascadeSplit[1] = r2;
    m_shadowCascadeSplit[2] = r3;
    m_shadowCascadeSplit[3] = r4;
}
void RenderLayer::SetDirectionalLightShadowMapResolution(const size_t &value)
{
    m_directionalLightShadowMapResolution = value;
    if (m_directionalLightShadowMap != nullptr)
        m_directionalLightShadowMap->SetResolution(value);
}

void RenderLayer::SetPointLightShadowMapResolution(const size_t &value)
{
    m_pointLightShadowMapResolution = value;
    if (m_pointLightShadowMap != nullptr)
        m_pointLightShadowMap->SetResolution(value);
}

void RenderLayer::SetSpotLightShadowMapResolution(const size_t &value)
{
    m_spotLightShadowMapResolution = value;
    if (m_spotLightShadowMap != nullptr)
        m_spotLightShadowMap->SetResolution(value);
}
void RenderLayer::ShadowMapPrePass(
    const int &enabledSize,
    std::shared_ptr<OpenGLUtils::GLProgram> &defaultProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &defaultInstancedProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &skinnedProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &instancedSkinnedProgram)
{

    for (auto &i : m_deferredRenderInstances)
    {
        const auto &cameraComponent = i.first;
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                if (!renderCommand.m_castShadow)
                    return;
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto mesh = renderCommand.m_mesh.lock();
                    auto &program = defaultProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    mesh->Draw();
                    break;
                }
                case RenderCommandMeshType::Skinned: {
                    auto skinnedMesh = renderCommand.m_skinnedMesh.lock();
                    auto &program = skinnedProgram;
                    program->Bind();
                    renderCommand.m_boneMatrices.lock()->UploadBones(skinnedMesh);
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    skinnedMesh->Draw();
                    break;
                }
                }
            },
            false);
    }
    for (auto &i : m_deferredInstancedRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                if (!renderCommand.m_castShadow)
                    return;
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    if (renderCommand.m_matrices.expired())
                        break;
                    auto mesh = renderCommand.m_mesh.lock();
                    auto &program = defaultInstancedProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    mesh->DrawInstanced(renderCommand.m_matrices.lock());
                    break;
                }
                }
            },
            false);
    }
    for (auto &i : m_forwardRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                if (!renderCommand.m_castShadow)
                    return;
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto mesh = renderCommand.m_mesh.lock();
                    auto &program = defaultProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    mesh->Draw();
                    break;
                }
                case RenderCommandMeshType::Skinned: {
                    auto skinnedMesh = renderCommand.m_skinnedMesh.lock();
                    auto &program = skinnedProgram;
                    program->Bind();
                    renderCommand.m_boneMatrices.lock()->UploadBones(skinnedMesh);
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    skinnedMesh->Draw();
                    break;
                }
                }
            },
            false);
    }
    for (auto &i : m_forwardInstancedRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                if (!renderCommand.m_castShadow)
                    return;
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    if (renderCommand.m_matrices.expired())
                        break;
                    auto mesh = renderCommand.m_mesh.lock();
                    auto &program = defaultInstancedProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    mesh->DrawInstanced(renderCommand.m_matrices.lock());
                    break;
                }
                }
            },
            false);
    }
}
void RenderLayer::RenderShadows(
    Bound &worldBound, const std::shared_ptr<Camera> &cameraComponent, const GlobalTransform &cameraModel)
{
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    auto scene = GetScene();
#pragma region Shadow
    auto &minBound = worldBound.m_min;
    auto &maxBound = worldBound.m_max;

    auto ltw = cameraModel;
    glm::vec3 mainCameraPos = ltw.GetPosition();
    glm::quat mainCameraRot = ltw.GetRotation();
    m_shadowCascadeInfoBlock.SubData(0, sizeof(LightSettingsBlock), &m_lightSettings);
    const std::vector<Entity> *directionalLightEntities =
        scene->UnsafeGetPrivateComponentOwnersList<DirectionalLight>();
    size_t size = 0;
    if (directionalLightEntities && !directionalLightEntities->empty())
    {
        size = directionalLightEntities->size();
        int enabledSize = 0;
        for (int i = 0; i < size; i++)
        {
            Entity lightEntity = directionalLightEntities->at(i);
            if (!scene->IsEntityEnabled(lightEntity))
                continue;
            const auto dlc = scene->GetOrSetPrivateComponent<DirectionalLight>(lightEntity).lock();
            if (!dlc->IsEnabled())
                continue;
            glm::quat rotation = scene->GetDataComponent<GlobalTransform>(lightEntity).GetRotation();
            glm::vec3 lightDir = glm::normalize(rotation * glm::vec3(0, 0, 1));
            float planeDistance = 0;
            glm::vec3 center;
            m_directionalLights[enabledSize].m_direction = glm::vec4(lightDir, 0.0f);
            m_directionalLights[enabledSize].m_diffuse =
                glm::vec4(dlc->m_diffuse * dlc->m_diffuseBrightness, dlc->m_castShadow);
            m_directionalLights[enabledSize].m_specular = glm::vec4(0.0f);
            for (int split = 0; split < DefaultResources::ShaderIncludes::ShadowCascadeAmount; split++)
            {
                float splitStart = 0;
                float splitEnd = m_maxShadowDistance;
                if (split != 0)
                    splitStart = m_maxShadowDistance * m_shadowCascadeSplit[split - 1];
                if (split != DefaultResources::ShaderIncludes::ShadowCascadeAmount - 1)
                    splitEnd = m_maxShadowDistance * m_shadowCascadeSplit[split];
                m_lightSettings.m_splitDistance[split] = splitEnd;
                glm::mat4 lightProjection, lightView;
                float max = 0;
                glm::vec3 lightPos;
                glm::vec3 cornerPoints[8];
                Camera::CalculateFrustumPoints(
                    cameraComponent, splitStart, splitEnd, mainCameraPos, mainCameraRot, cornerPoints);
                glm::vec3 cameraFrustumCenter =
                    (mainCameraRot * glm::vec3(0, 0, -1)) * ((splitEnd - splitStart) / 2.0f + splitStart) +
                    mainCameraPos;
                if (m_stableFit)
                {
                    // Less detail but no shimmering when rotating the camera.
                    // max = glm::distance(cornerPoints[4], cameraFrustumCenter);
                    max = splitEnd;
                }
                else
                {
                    // More detail but cause shimmering when rotating camera.
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[0],
                            ClosestPointOnLine(cornerPoints[0], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[1],
                            ClosestPointOnLine(cornerPoints[1], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[2],
                            ClosestPointOnLine(cornerPoints[2], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[3],
                            ClosestPointOnLine(cornerPoints[3], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[4],
                            ClosestPointOnLine(cornerPoints[4], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[5],
                            ClosestPointOnLine(cornerPoints[5], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[6],
                            ClosestPointOnLine(cornerPoints[6], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    max = (glm::max)(
                        max,
                        glm::distance(
                            cornerPoints[7],
                            ClosestPointOnLine(cornerPoints[7], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                }

                glm::vec3 p0 = ClosestPointOnLine(
                    glm::vec3(maxBound.x, maxBound.y, maxBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                glm::vec3 p7 = ClosestPointOnLine(
                    glm::vec3(minBound.x, minBound.y, minBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);

                float d0 = glm::distance(p0, p7);

                glm::vec3 p1 = ClosestPointOnLine(
                    glm::vec3(maxBound.x, maxBound.y, minBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                glm::vec3 p6 = ClosestPointOnLine(
                    glm::vec3(minBound.x, minBound.y, maxBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);

                float d1 = glm::distance(p1, p6);

                glm::vec3 p2 = ClosestPointOnLine(
                    glm::vec3(maxBound.x, minBound.y, maxBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                glm::vec3 p5 = ClosestPointOnLine(
                    glm::vec3(minBound.x, maxBound.y, minBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);

                float d2 = glm::distance(p2, p5);

                glm::vec3 p3 = ClosestPointOnLine(
                    glm::vec3(maxBound.x, minBound.y, minBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                glm::vec3 p4 = ClosestPointOnLine(
                    glm::vec3(minBound.x, maxBound.y, maxBound.z), cameraFrustumCenter, cameraFrustumCenter + lightDir);

                float d3 = glm::distance(p3, p4);

                center = ClosestPointOnLine(worldBound.Center(), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                planeDistance = (glm::max)((glm::max)(d0, d1), (glm::max)(d2, d3));
                lightPos = center - lightDir * planeDistance;
                lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::normalize(rotation * glm::vec3(0, 1, 0)));
                lightProjection = glm::ortho(-max, max, -max, max, 0.0f, planeDistance * 2.0f);
                switch (enabledSize)
                {
                case 0:
                    m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                        0, 0, m_directionalLightShadowMapResolution / 2, m_directionalLightShadowMapResolution / 2);
                    break;
                case 1:
                    m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                        m_directionalLightShadowMapResolution / 2,
                        0,
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2);
                    break;
                case 2:
                    m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                        0,
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2);
                    break;
                case 3:
                    m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2,
                        m_directionalLightShadowMapResolution / 2);
                    break;
                }

#pragma region Fix Shimmering due to the movement of the camera

                glm::mat4 shadowMatrix = lightProjection * lightView;
                glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                shadowOrigin = shadowMatrix * shadowOrigin;
                GLfloat storedW = shadowOrigin.w;
                shadowOrigin = shadowOrigin * (float)m_directionalLights[enabledSize].m_viewPort.z / 2.0f;
                glm::vec4 roundedOrigin = glm::round(shadowOrigin);
                glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
                roundOffset = roundOffset * 2.0f / (float)m_directionalLights[enabledSize].m_viewPort.z;
                roundOffset.z = 0.0f;
                roundOffset.w = 0.0f;
                glm::mat4 shadowProj = lightProjection;
                shadowProj[3] += roundOffset;
                lightProjection = shadowProj;
#pragma endregion
                m_directionalLights[enabledSize].m_lightSpaceMatrix[split] = lightProjection * lightView;
                m_directionalLights[enabledSize].m_lightFrustumWidth[split] = max;
                m_directionalLights[enabledSize].m_lightFrustumDistance[split] = planeDistance;
                if (split == DefaultResources::ShaderIncludes::ShadowCascadeAmount - 1)
                    m_directionalLights[enabledSize].m_reservedParameters =
                        glm::vec4(dlc->m_lightSize, 0, dlc->m_bias, dlc->m_normalOffset);
            }
            enabledSize++;
        }
        m_directionalLightBlock.SubData(0, 4, &enabledSize);
        if (enabledSize != 0)
        {
            m_directionalLightBlock.SubData(16, enabledSize * sizeof(DirectionalLightInfo), &m_directionalLights[0]);
        }
        if (m_materialSettings.m_enableShadow)
        {
            m_directionalLightShadowMap->Bind();
            m_directionalLightShadowMap->GetFrameBuffer()->DrawBuffers({});
            glClear(GL_DEPTH_BUFFER_BIT);
            enabledSize = 0;
            DefaultResources::m_directionalLightProgram->Bind();
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = directionalLightEntities->at(i);
                if (!scene->IsEntityEnabled(lightEntity))
                    continue;
                OpenGLUtils::SetViewPort(
                    m_directionalLights[enabledSize].m_viewPort.x,
                    m_directionalLights[enabledSize].m_viewPort.y,
                    m_directionalLights[enabledSize].m_viewPort.z,
                    m_directionalLights[enabledSize].m_viewPort.w);
                DefaultResources::m_directionalLightProgram->SetInt("index", enabledSize);
                ShadowMapPrePass(
                    enabledSize,
                    DefaultResources::m_directionalLightProgram,
                    DefaultResources::m_directionalLightInstancedProgram,
                    DefaultResources::m_directionalLightSkinnedProgram,
                    DefaultResources::m_directionalLightInstancedSkinnedProgram);
                enabledSize++;
            }
        }
    }
    else
    {
        m_directionalLightBlock.SubData(0, 4, &size);
    }
    const std::vector<Entity> *pointLightEntities =
        scene->UnsafeGetPrivateComponentOwnersList<PointLight>();
    size = 0;
    if (pointLightEntities && !pointLightEntities->empty())
    {
        size = pointLightEntities->size();
        size_t enabledSize = 0;
        for (int i = 0; i < size; i++)
        {
            Entity lightEntity = pointLightEntities->at(i);
            if (!scene->IsEntityEnabled(lightEntity))
                continue;
            const auto plc = scene->GetOrSetPrivateComponent<PointLight>(lightEntity).lock();
            if (!plc->IsEnabled())
                continue;
            glm::vec3 position = scene->GetDataComponent<GlobalTransform>(lightEntity).m_value[3];
            m_pointLights[enabledSize].m_position = glm::vec4(position, 0);
            m_pointLights[enabledSize].m_constantLinearQuadFarPlane.x = plc->m_constant;
            m_pointLights[enabledSize].m_constantLinearQuadFarPlane.y = plc->m_linear;
            m_pointLights[enabledSize].m_constantLinearQuadFarPlane.z = plc->m_quadratic;
            m_pointLights[enabledSize].m_diffuse =
                glm::vec4(plc->m_diffuse * plc->m_diffuseBrightness, plc->m_castShadow);
            m_pointLights[enabledSize].m_specular = glm::vec4(0);
            m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w = plc->GetFarPlane();

            glm::mat4 shadowProj = glm::perspective(
                glm::radians(90.0f),
                m_pointLightShadowMap->GetResolutionRatio(),
                1.0f,
                m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w);
            m_pointLights[enabledSize].m_lightSpaceMatrix[0] =
                shadowProj *
                glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            m_pointLights[enabledSize].m_lightSpaceMatrix[1] =
                shadowProj *
                glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            m_pointLights[enabledSize].m_lightSpaceMatrix[2] =
                shadowProj * glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            m_pointLights[enabledSize].m_lightSpaceMatrix[3] =
                shadowProj *
                glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
            m_pointLights[enabledSize].m_lightSpaceMatrix[4] =
                shadowProj *
                glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            m_pointLights[enabledSize].m_lightSpaceMatrix[5] =
                shadowProj *
                glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            m_pointLights[enabledSize].m_reservedParameters = glm::vec4(plc->m_bias, plc->m_lightSize, 0, 0);

            switch (enabledSize)
            {
            case 0:
                m_pointLights[enabledSize].m_viewPort =
                    glm::ivec4(0, 0, m_pointLightShadowMapResolution / 2, m_pointLightShadowMapResolution / 2);
                break;
            case 1:
                m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                    m_pointLightShadowMapResolution / 2,
                    0,
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2);
                break;
            case 2:
                m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                    0,
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2);
                break;
            case 3:
                m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2,
                    m_pointLightShadowMapResolution / 2);
                break;
            }
            enabledSize++;
        }
        m_pointLightBlock.SubData(0, 4, &enabledSize);
        if (enabledSize != 0)
            m_pointLightBlock.SubData(16, enabledSize * sizeof(PointLightInfo), &m_pointLights[0]);
        if (m_materialSettings.m_enableShadow)
        {
            m_pointLightShadowMap->Bind();
            m_pointLightShadowMap->GetFrameBuffer()->DrawBuffers({});
            glClear(GL_DEPTH_BUFFER_BIT);
            enabledSize = 0;
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = pointLightEntities->at(i);
                if (!scene->IsEntityEnabled(lightEntity))
                    continue;
                OpenGLUtils::SetViewPort(
                    m_pointLights[enabledSize].m_viewPort.x,
                    m_pointLights[enabledSize].m_viewPort.y,
                    m_pointLights[enabledSize].m_viewPort.z,
                    m_pointLights[enabledSize].m_viewPort.w);
                ShadowMapPrePass(
                    enabledSize,
                    DefaultResources::m_pointLightProgram,
                    DefaultResources::m_pointLightInstancedProgram,
                    DefaultResources::m_pointLightSkinnedProgram,
                    DefaultResources::m_pointLightInstancedSkinnedProgram);
                enabledSize++;
            }
        }
    }
    else
    {
        m_pointLightBlock.SubData(0, 4, &size);
    }
    const std::vector<Entity> *spotLightEntities =
        scene->UnsafeGetPrivateComponentOwnersList<SpotLight>();
    size = 0;
    if (spotLightEntities && !spotLightEntities->empty())
    {
        size = spotLightEntities->size();
        size_t enabledSize = 0;
        for (int i = 0; i < size; i++)
        {
            Entity lightEntity = spotLightEntities->at(i);
            if (!scene->IsEntityEnabled(lightEntity))
                continue;
            const auto slc = scene->GetOrSetPrivateComponent<SpotLight>(lightEntity).lock();
            if (!slc->IsEnabled())
                continue;
            auto ltw = scene->GetDataComponent<GlobalTransform>(lightEntity);
            glm::vec3 position = ltw.m_value[3];
            glm::vec3 front = ltw.GetRotation() * glm::vec3(0, 0, -1);
            glm::vec3 up = ltw.GetRotation() * glm::vec3(0, 1, 0);
            m_spotLights[enabledSize].m_position = glm::vec4(position, 0);
            m_spotLights[enabledSize].m_direction = glm::vec4(front, 0);
            m_spotLights[enabledSize].m_constantLinearQuadFarPlane.x = slc->m_constant;
            m_spotLights[enabledSize].m_constantLinearQuadFarPlane.y = slc->m_linear;
            m_spotLights[enabledSize].m_constantLinearQuadFarPlane.z = slc->m_quadratic;
            m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w = slc->GetFarPlane();
            m_spotLights[enabledSize].m_diffuse =
                glm::vec4(slc->m_diffuse * slc->m_diffuseBrightness, slc->m_castShadow);
            m_spotLights[enabledSize].m_specular = glm::vec4(0);

            glm::mat4 shadowProj = glm::perspective(
                glm::radians(slc->m_outerDegrees * 2.0f),
                m_spotLightShadowMap->GetResolutionRatio(),
                1.0f,
                m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w);
            m_spotLights[enabledSize].m_lightSpaceMatrix = shadowProj * glm::lookAt(position, position + front, up);
            m_spotLights[enabledSize].m_cutOffOuterCutOffLightSizeBias = glm::vec4(
                glm::cos(glm::radians(slc->m_innerDegrees)),
                glm::cos(glm::radians(slc->m_outerDegrees)),
                slc->m_lightSize,
                slc->m_bias);

            switch (enabledSize)
            {
            case 0:
                m_spotLights[enabledSize].m_viewPort =
                    glm::ivec4(0, 0, m_spotLightShadowMapResolution / 2, m_spotLightShadowMapResolution / 2);
                break;
            case 1:
                m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                    m_spotLightShadowMapResolution / 2,
                    0,
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2);
                break;
            case 2:
                m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                    0,
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2);
                break;
            case 3:
                m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2,
                    m_spotLightShadowMapResolution / 2);
                break;
            }
            enabledSize++;
        }
        m_spotLightBlock.SubData(0, 4, &enabledSize);
        if (enabledSize != 0)
            m_spotLightBlock.SubData(16, enabledSize * sizeof(SpotLightInfo), &m_spotLights[0]);
        if (m_materialSettings.m_enableShadow)
        {
            m_spotLightShadowMap->Bind();
            m_spotLightShadowMap->GetFrameBuffer()->DrawBuffers({});
            glClear(GL_DEPTH_BUFFER_BIT);
            enabledSize = 0;
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = spotLightEntities->at(i);
                if (!scene->IsEntityEnabled(lightEntity))
                    continue;
                OpenGLUtils::SetViewPort(
                    m_spotLights[enabledSize].m_viewPort.x,
                    m_spotLights[enabledSize].m_viewPort.y,
                    m_spotLights[enabledSize].m_viewPort.z,
                    m_spotLights[enabledSize].m_viewPort.w);
                ShadowMapPrePass(
                    enabledSize,
                    DefaultResources::m_spotLightProgram,
                    DefaultResources::m_spotLightInstancedProgram,
                    DefaultResources::m_spotLightSkinnedProgram,
                    DefaultResources::m_spotLightInstancedSkinnedProgram);
                enabledSize++;
            }
#pragma endregion
        }
    }
    else
    {
        m_spotLightBlock.SubData(0, 4, &size);
    }
#pragma endregion
}
#pragma endregion

#pragma region Internal

void RenderLayer::ApplyShadowMapSettings()
{

#pragma region Shadow map binding and default texture binding.
    m_directionalLightShadowMap->DepthMapArray()->Bind(0);
    m_pointLightShadowMap->DepthMapArray()->Bind(1);
    m_spotLightShadowMap->DepthMap()->Bind(2);
#pragma endregion
}

void RenderLayer::ApplyEnvironmentalSettings(const std::shared_ptr<Camera> &cameraComponent)
{
    auto scene = GetScene();
    auto cameraSkybox = cameraComponent->m_skybox.Get<Cubemap>();
    if (!cameraSkybox || !cameraSkybox->Texture())
        cameraSkybox = DefaultResources::Environmental::DefaultEnvironmentalMap->m_targetCubemap.Get<Cubemap>();

    auto environmentalMap = scene->m_environmentSettings.m_environmentalMap.Get<EnvironmentalMap>();
    switch (scene->m_environmentSettings.m_environmentType)
    {
    case EnvironmentType::EnvironmentalMap: {
        if (!environmentalMap || !environmentalMap->m_ready)
        {
            environmentalMap = DefaultResources::Environmental::DefaultEnvironmentalMap;
            m_environmentalMapSettings.m_backgroundColor.w = 1.0f;
        }
        else
        {
            m_environmentalMapSettings.m_backgroundColor.w = 0.0f;
        }
    }
    break;
    case EnvironmentType::Color: {
        environmentalMap = DefaultResources::Environmental::DefaultEnvironmentalMap;
        m_environmentalMapSettings.m_backgroundColor = glm::vec4(scene->m_environmentSettings.m_backgroundColor, 1.0f);
    }
    break;
    }
    m_environmentalMapSettings.m_environmentalMapGamma = scene->m_environmentSettings.m_environmentGamma;
    m_environmentalMapSettings.m_environmentalLightingIntensity = scene->m_environmentSettings.m_ambientLightIntensity;
    cameraSkybox->Texture()->Bind(8);
    DefaultResources::m_brdfLut->UnsafeGetGLTexture()->Bind(11);
    environmentalMap->m_lightProbe.Get<LightProbe>()->m_irradianceMap->Texture()->Bind(9);
    environmentalMap->m_reflectionProbe.Get<ReflectionProbe>()->m_preFilteredMap->Texture()->Bind(10);

    m_environmentalMapSettingsBuffer->SubData(0, sizeof(EnvironmentalMapSettingsBlock), &m_environmentalMapSettings);
}

void RenderLayer::MaterialPropertySetter(const std::shared_ptr<Material> &material, const bool &disableBlending)
{
    switch (material->m_polygonMode)
    {
    case MaterialPolygonMode::Fill:
        OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
        break;
    case MaterialPolygonMode::Line:
        OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Line);
        break;
    case MaterialPolygonMode::Point:
        OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Point);
        break;
    }

    switch (material->m_cullingMode)
    {
    case MaterialCullingMode::Off:
        OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
        break;
    case MaterialCullingMode::Front:
        OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
        OpenGLUtils::SetCullFace(OpenGLCullFace::Front);
        break;
    case MaterialCullingMode::Back:
        OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
        OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
        break;
    }
    if (disableBlending)
        OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    else
    {
        switch (material->m_blendingMode)
        {
        case MaterialBlendingMode::Off:
            break;
        case MaterialBlendingMode::OneMinusSrcAlpha:
            OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
    }
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
}

void RenderLayer::ApplyMaterialSettings(const std::shared_ptr<Material> &material)
{
    bool hasAlbedo = false;

    auto albedoTexture = material->m_albedoTexture.Get<Texture2D>();
    if (albedoTexture && albedoTexture->UnsafeGetGLTexture())
    {
        hasAlbedo = true;
        albedoTexture->UnsafeGetGLTexture()->Bind(3);
        m_materialSettings.m_albedoEnabled = static_cast<int>(true);
    }
    auto normalTexture = material->m_normalTexture.Get<Texture2D>();
    if (normalTexture && normalTexture->UnsafeGetGLTexture())
    {
        normalTexture->UnsafeGetGLTexture()->Bind(4);
        m_materialSettings.m_normalEnabled = static_cast<int>(true);
    }
    auto metallicTexture = material->m_metallicTexture.Get<Texture2D>();
    if (metallicTexture && metallicTexture->UnsafeGetGLTexture())
    {
        metallicTexture->UnsafeGetGLTexture()->Bind(5);
        m_materialSettings.m_metallicEnabled = static_cast<int>(true);
    }
    auto roughnessTexture = material->m_roughnessTexture.Get<Texture2D>();
    if (roughnessTexture && roughnessTexture->UnsafeGetGLTexture())
    {
        roughnessTexture->UnsafeGetGLTexture()->Bind(6);
        m_materialSettings.m_roughnessEnabled = static_cast<int>(true);
    }
    auto aoTexture = material->m_aoTexture.Get<Texture2D>();
    if (aoTexture && aoTexture->UnsafeGetGLTexture())
    {
        aoTexture->UnsafeGetGLTexture()->Bind(7);
        m_materialSettings.m_aoEnabled = static_cast<int>(true);
    }

    if (!hasAlbedo)
    {
        DefaultResources::Textures::MissingTexture->UnsafeGetGLTexture()->Bind(3);
    }
    m_materialSettings.m_castShadow = true;
    m_materialSettings.m_subsurfaceColorRadius = glm::vec4(material->m_subsurfaceColor, material->m_subsurfaceRadius);
    m_materialSettings.m_albedoColorVal = glm::vec4(material->m_albedoColor, material->m_blendingMode != MaterialBlendingMode::Off ? material->m_transparency : 1.0f);
    m_materialSettings.m_metallicVal = material->m_metallic;
    m_materialSettings.m_roughnessVal = material->m_roughness;
    m_materialSettings.m_aoVal = material->m_ambient;
    m_materialSettings.m_emissionVal = material->m_emission;

    m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &m_materialSettings);
}

void RenderLayer::ApplyProgramSettings(
    const std::shared_ptr<OpenGLUtils::GLProgram> &program, const std::shared_ptr<Material> &material)
{
    program->SetInt("UE_DIRECTIONAL_LIGHT_SM", 0);
    program->SetInt("UE_POINT_LIGHT_SM", 1);
    program->SetInt("UE_SPOT_LIGHT_SM", 2);
    program->SetInt("UE_ALBEDO_MAP", 3);
    auto normalTexture = material->m_normalTexture.Get<Texture2D>();
    if (normalTexture && normalTexture->UnsafeGetGLTexture())
    {
        program->SetInt("UE_NORMAL_MAP", 4);
    }
    else
    {
        program->SetInt("UE_NORMAL_MAP", 3);
    }
    auto metallicTexture = material->m_metallicTexture.Get<Texture2D>();
    if (metallicTexture && metallicTexture->UnsafeGetGLTexture())
    {
        program->SetInt("UE_METALLIC_MAP", 5);
    }
    else
    {
        program->SetInt("UE_METALLIC_MAP", 3);
    }
    auto roughnessTexture = material->m_roughnessTexture.Get<Texture2D>();
    if (roughnessTexture && roughnessTexture->UnsafeGetGLTexture())
    {
        program->SetInt("UE_ROUGHNESS_MAP", 6);
    }
    else
    {
        program->SetInt("UE_ROUGHNESS_MAP", 3);
    }
    auto aoTexture = material->m_aoTexture.Get<Texture2D>();
    if (aoTexture && aoTexture->UnsafeGetGLTexture())
    {
        program->SetInt("UE_AO_MAP", 7);
    }
    else
    {
        program->SetInt("UE_AO_MAP", 3);
    }

    program->SetInt("UE_SKYBOX", 8);
    program->SetInt("UE_ENVIRONMENTAL_IRRADIANCE", 9);
    program->SetInt("UE_ENVIRONMENTAL_PREFILERED", 10);
    program->SetInt("UE_ENVIRONMENTAL_BRDFLUT", 11);
}

void RenderLayer::ReleaseMaterialSettings(const std::shared_ptr<Material> &material)
{
}

void RenderLayer::PrepareBrdfLut()
{
    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    auto brdfLut = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RG16F, 512, 512, true);
    DefaultResources::m_brdfLut = std::make_unique<Texture2D>();
    DefaultResources::m_brdfLut->m_texture = std::move(brdfLut);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    DefaultResources::m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    DefaultResources::m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    DefaultResources::m_brdfLut->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    DefaultResources::m_brdfLut->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    size_t resolution = 512;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
    renderTarget->AttachTexture(DefaultResources::m_brdfLut->m_texture.get(), GL_COLOR_ATTACHMENT0);
    OpenGLUtils::SetViewPort(resolution, resolution);
    DefaultResources::BrdfProgram->Bind();
    renderTarget->Clear();
    Graphics::RenderQuad();
    OpenGLUtils::GLFrameBuffer::BindDefault();
}

void RenderLayer::DeferredPrepassInternal(const std::shared_ptr<Mesh> &mesh)
{
    if (mesh == nullptr)
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderLayer::DeferredPrepassInstancedInternal(
    const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<ParticleMatrices> &matrices)
{
    if (mesh == nullptr || matrices->m_value.empty())
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices->m_value.size();
    mesh->DrawInstanced(matrices);
}

void RenderLayer::DeferredPrepassInternal(const std::shared_ptr<SkinnedMesh> &skinnedMesh)
{
    if (skinnedMesh == nullptr)
        return;
    m_drawCall++;
    m_triangles += skinnedMesh->GetTriangleAmount();
    skinnedMesh->Draw();
}

void RenderLayer::DeferredPrepassInstancedInternal(
    const std::shared_ptr<SkinnedMesh> &skinnedMesh, const std::shared_ptr<ParticleMatrices> &matrices)
{
    if (skinnedMesh == nullptr || matrices->m_value.empty())
        return;
    m_drawCall++;
    m_triangles += skinnedMesh->GetTriangleAmount() * matrices->m_value.size();
    auto &program = DefaultResources::m_gBufferInstancedPrepass;
    skinnedMesh->DrawInstanced(matrices);
}

void RenderLayer::DrawMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const std::vector<glm::mat4> &matrices,
    const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr || matrices.empty())
        return;

    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices.size();
    auto program = material->m_program.Get<OpenGLUtils::GLProgram>();
    if (program == nullptr)
        program = DefaultResources::GLPrograms::StandardInstancedProgram;
    program->Bind();
    program->SetFloat4x4("model", model);
    for (auto j : material->m_floatPropertyList)
    {
        program->SetFloat(j.m_name, j.m_value);
    }
    for (auto j : material->m_float4X4PropertyList)
    {
        program->SetFloat4x4(j.m_name, j.m_value);
    }

    MaterialPropertySetter(material);
    m_materialSettings = MaterialSettingsBlock();
    m_materialSettings.m_receiveShadow = receiveShadow;
    ApplyMaterialSettings(material);
    ApplyProgramSettings(program, material);
    mesh->DrawInstanced(matrices);
    ReleaseMaterialSettings(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderLayer::DrawMeshInstancedInternal(
    const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<ParticleMatrices> &matrices)
{
    if (mesh == nullptr || matrices->m_value.empty())
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices->m_value.size();
    mesh->DrawInstanced(matrices);
}

void RenderLayer::DrawMeshInstancedInternal(
    const std::shared_ptr<SkinnedMesh> &mesh, const std::vector<glm::mat4> &matrices)
{
    if (mesh == nullptr || matrices.empty())
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderLayer::DrawMesh(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr)
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    auto program = material->m_program.Get<OpenGLUtils::GLProgram>();
    if (program == nullptr)
        program = DefaultResources::GLPrograms::StandardProgram;
    program->Bind();
    program->SetFloat4x4("model", model);
    for (auto j : material->m_floatPropertyList)
    {
        program->SetFloat(j.m_name, j.m_value);
    }
    for (auto j : material->m_float4X4PropertyList)
    {
        program->SetFloat4x4(j.m_name, j.m_value);
    }
    m_materialSettings = MaterialSettingsBlock();
    m_materialSettings.m_receiveShadow = receiveShadow;
    MaterialPropertySetter(material);
    ApplyMaterialSettings(material);
    ApplyProgramSettings(program, material);
    mesh->Draw();
    ReleaseMaterialSettings(material);
}

void RenderLayer::DrawMeshInternal(const std::shared_ptr<Mesh> &mesh)
{
    if (mesh == nullptr)
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderLayer::DrawMeshInternal(const std::shared_ptr<SkinnedMesh> &mesh)
{
    if (mesh == nullptr)
        return;
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderLayer::DrawGizmoMeshInstanced(
    bool depthTest,
    const std::shared_ptr<Mesh> &mesh,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices.empty())
        return;
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
    OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
    if (!depthTest)
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    else
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    DefaultResources::GizmoInstancedProgram->Bind();
    DefaultResources::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
    DefaultResources::GizmoInstancedProgram->SetFloat4x4("model", model);
    DefaultResources::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderLayer::DrawGizmoMeshInstancedColored(
    bool depthTest,
    const std::shared_ptr<Mesh> &mesh,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices.empty() || colors.empty() || matrices.size() != colors.size())
        return;
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
    OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
    if (!depthTest)
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    else
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    const auto vao = mesh->Vao();
    const OpenGLUtils::GLVBO colorsBuffer;
    colorsBuffer.SetData(static_cast<GLsizei>(matrices.size()) * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
    vao->EnableAttributeArray(11);
    vao->SetAttributePointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
    vao->SetAttributeDivisor(11, 1);

    DefaultResources::GizmoInstancedColoredProgram->Bind();
    DefaultResources::GizmoInstancedColoredProgram->SetFloat4x4("model", model);
    DefaultResources::GizmoInstancedColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderLayer::DrawGizmoMesh(
    bool depthTest,
    const std::shared_ptr<Mesh> &mesh,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr)
        return;
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
    OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
    if (!depthTest)
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    else
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DefaultResources::GizmoProgram->Bind();
    DefaultResources::GizmoProgram->SetFloat4("surfaceColor", color);
    DefaultResources::GizmoProgram->SetFloat4x4("model", model);
    DefaultResources::GizmoProgram->SetFloat4x4("scaleMatrix", scaleMatrix);

    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderLayer::DrawGizmoMeshVertexColored(
    bool depthTest,
    const std::shared_ptr<Mesh> &mesh,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr)
        return;
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
    OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
    if (!depthTest)
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    else
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DefaultResources::GizmoVertexColoredProgram->Bind();
    DefaultResources::GizmoVertexColoredProgram->SetFloat4x4("model", model);
    DefaultResources::GizmoVertexColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);

    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderLayer::DrawGizmoMeshNormalColored(
    bool depthTest,
    const std::shared_ptr<Mesh> &mesh,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr)
        return;
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, true);
    OpenGLUtils::SetCullFace(OpenGLCullFace::Back);
    if (!depthTest)
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    else
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DefaultResources::GizmoNormalColoredProgram->Bind();
    DefaultResources::GizmoNormalColoredProgram->SetFloat4x4("model", model);
    DefaultResources::GizmoNormalColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);

    m_drawCall++;
    m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}
#pragma endregion
#pragma region Status

size_t RenderLayer::Triangles()
{
    return m_triangles;
}

size_t RenderLayer::DrawCall()
{
    return m_drawCall;
}
RenderLayer::RenderLayer()
{
}
#pragma endregion