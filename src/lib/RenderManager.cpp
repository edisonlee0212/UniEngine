#include <Application.hpp>
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <Cubemap.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <LightProbe.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
#include <ReflectionProbe.hpp>
#include <RenderManager.hpp>
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;
#pragma region RenderCommand Dispatch
void RenderManager::DispatchRenderCommands(
    const std::map<std::shared_ptr<Material>, RenderCommandGroup> &renderCommands,
    const std::function<void(const std::shared_ptr<Material> &, const RenderCommand &renderCommand)> &func,
    const bool &setMaterial,
    const bool &bindProgram)
{
    auto &renderManager = GetInstance();
    for (const auto &renderCollection : renderCommands)
    {
        const auto &material = renderCollection.first;
        if (setMaterial)
        {
            MaterialPropertySetter(material, true);
            GetInstance().m_materialSettings = MaterialSettingsBlock();
            ApplyMaterialSettings(material);
        }
        if (bindProgram)
        {
            material->m_program->Bind();
        }
        for (const auto &renderCommandGroup : renderCollection.second.m_meshes)
        {
            for (const auto &renderCommands : renderCommandGroup.second)
            {
                for (const auto &renderCommand : renderCommands.second)
                {
                    func(material, renderCommand);
                }
            }
        }
        for (const auto &renderCommandGroup : renderCollection.second.m_skinnedMeshes)
        {
            for (const auto &renderCommands : renderCommandGroup.second)
            {
                for (const auto &renderCommand : renderCommands.second)
                {
                    func(material, renderCommand);
                }
            }
        }
        if (setMaterial)
            ReleaseMaterialSettings(material);
    }
}

#pragma endregion
void RenderManager::RenderToCamera(Camera &cameraComponent)
{
    auto &renderManager = GetInstance();
    glEnable(GL_DEPTH_TEST);
    cameraComponent.m_gBuffer->Bind();
    unsigned int attachments[4] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    cameraComponent.m_gBuffer->GetFrameBuffer()->DrawBuffers(4, attachments);
    cameraComponent.m_gBuffer->Clear();
    DispatchRenderCommands(
        renderManager.m_deferredRenderInstances[&cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                auto &program = renderManager.m_gBufferPrepass;
                program->Bind();
                ApplyProgramSettings(program);
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DeferredPrepassInternal(renderCommand.m_mesh);
                break;
            }
            case RenderCommandMeshType::Skinned: {
                auto &program = renderManager.m_gBufferSkinnedPrepass;
                program->Bind();
                ApplyProgramSettings(program);
                renderCommand.m_boneMatrices->UploadBones(renderCommand.m_skinnedMesh);
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                DeferredPrepassInternal(renderCommand.m_skinnedMesh);
                break;
            }
            }
        },
        true,
        false);
    DispatchRenderCommands(
        renderManager.m_deferredInstancedRenderInstances[&cameraComponent],
        [&](const std::shared_ptr<Material> &, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                auto &program = renderManager.m_gBufferInstancedPrepass;
                program->Bind();
                ApplyProgramSettings(program);
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DeferredPrepassInstancedInternal(renderCommand.m_mesh, renderCommand.m_matrices->m_value);
                break;
            }
            }
        },
        true,
        false);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
#pragma region Copy Depth Buffer back to camera
    auto res = cameraComponent.GetResolution();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, cameraComponent.m_gBuffer->GetFrameBuffer()->Id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cameraComponent.GetFrameBuffer()->Id()); // write to default framebuffer
    glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#pragma endregion

    cameraComponent.m_frameBuffer->DrawBuffer(GL_COLOR_ATTACHMENT0);
#pragma region Apply GBuffer with lighting
    renderManager.m_gBufferLightingPass->Bind();
    cameraComponent.m_gBufferDepth->Bind(12);
    cameraComponent.m_gBufferNormal->Bind(13);
    cameraComponent.m_gBufferAlbedoEmission->Bind(14);
    cameraComponent.m_gBufferMetallicRoughnessAmbient->Bind(15);
    if (!OpenGLUtils::GetInstance().m_enableBindlessTexture)
    {
        renderManager.m_gBufferLightingPass->SetInt("UE_DIRECTIONAL_LIGHT_SM_LEGACY", 0);
        renderManager.m_gBufferLightingPass->SetInt("UE_POINT_LIGHT_SM_LEGACY", 1);
        renderManager.m_gBufferLightingPass->SetInt("UE_SPOT_LIGHT_SM_LEGACY", 2);
        renderManager.m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_MAP_LEGACY", 8);
        renderManager.m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_IRRADIANCE_LEGACY", 9);
        renderManager.m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_PREFILERED_LEGACY", 10);
        renderManager.m_gBufferLightingPass->SetInt("UE_ENVIRONMENTAL_BRDFLUT_LEGACY", 11);
    }
    renderManager.m_gBufferLightingPass->SetInt("gDepth", 12);
    renderManager.m_gBufferLightingPass->SetInt("gNormal", 13);
    renderManager.m_gBufferLightingPass->SetInt("gAlbedoEmission", 14);
    renderManager.m_gBufferLightingPass->SetInt("gMetallicRoughnessAmbient", 15);
    DefaultResources::GLPrograms::ScreenVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
#pragma endregion
    glEnable(GL_DEPTH_TEST);
#pragma region Forward rendering
    DispatchRenderCommands(
        renderManager.m_forwardRenderInstances[&cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                material->m_program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DrawMeshInternal(renderCommand.m_mesh);
                break;
            }
            case RenderCommandMeshType::Skinned: {
                renderCommand.m_boneMatrices->UploadBones(renderCommand.m_skinnedMesh);
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                DrawMeshInternal(renderCommand.m_skinnedMesh);
                break;
            }
            }
        },
        true,
        true);
    DispatchRenderCommands(
        renderManager.m_forwardInstancedRenderInstances[&cameraComponent],
        [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
            switch (renderCommand.m_meshType)
            {
            case RenderCommandMeshType::Default: {
                renderManager.m_materialSettings.m_receiveShadow = renderCommand.m_receiveShadow;
                renderManager.m_materialSettingsBuffer->SubData(
                    0, sizeof(MaterialSettingsBlock), &renderManager.m_materialSettings);
                material->m_program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                DrawMeshInstancedInternal(renderCommand.m_mesh, renderCommand.m_matrices->m_value);
                break;
            }
            }
        },
        true,
        true);
#pragma endregion
#pragma region Transparent
#pragma endregion
#pragma region Environment
    glDepthFunc(
        GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
    DefaultResources::GLPrograms::SkyboxProgram->Bind();
    DefaultResources::GLPrograms::SkyboxVAO->Bind();
    if (!OpenGLUtils::GetInstance().m_enableBindlessTexture)
    {
        DefaultResources::GLPrograms::SkyboxProgram->SetInt("UE_ENVIRONMENTAL_MAP_LEGACY", 8);
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS); // set depth function back to default
#pragma endregion
}
void RenderManager::PreUpdate()
{
    ProfilerManager::StartEvent("RenderManager");

    auto &renderManager = GetInstance();
    if(renderManager.m_mainCameraComponent) EditorManager::RenderToSceneCamera();

    ProfilerManager::StartEvent("Clear GBuffer");
    renderManager.m_deferredRenderInstances.clear();
    renderManager.m_deferredInstancedRenderInstances.clear();
    renderManager.m_forwardRenderInstances.clear();
    renderManager.m_forwardInstancedRenderInstances.clear();
    renderManager.m_transparentRenderInstances.clear();
    renderManager.m_instancedTransparentRenderInstances.clear();
    ProfilerManager::EndEvent("Clear GBuffer");
    ProfilerManager::EndEvent("RenderManager");
}
void RenderManager::LateUpdate()
{
    ProfilerManager::StartEvent("RenderManager");
    auto &renderManager = GetInstance();
#pragma region Collect RenderCommands
    ProfilerManager::StartEvent("RenderCommand Collection");
    Bound worldBound;
    const std::vector<Entity> *cameraEntities = EntityManager::UnsafeGetPrivateComponentOwnersList<Camera>();
    bool boundCalculated = false;
    if (cameraEntities != nullptr)
    {
        for (auto cameraEntity : *cameraEntities)
        {
            if (!cameraEntity.IsEnabled())
                continue;
            auto &cameraComponent = cameraEntity.GetPrivateComponent<Camera>();
            if (cameraComponent.IsEnabled())
            {
                const bool calculateBound = &cameraComponent == renderManager.m_mainCameraComponent;
                CollectRenderInstances(cameraComponent, cameraComponent.GetOwner().GetDataComponent<GlobalTransform>().GetPosition(), worldBound, calculateBound);
                if (calculateBound)
                    boundCalculated = true;
            }
        }
    }

    CollectRenderInstances(EditorManager::GetSceneCamera(), EditorManager::GetInstance().m_sceneCameraPosition,worldBound, !boundCalculated);
    ProfilerManager::EndEvent("RenderCommand Collection");
#pragma endregion
#pragma region Shadowmap prepass
    ProfilerManager::StartEvent("Shadowmap Prepass");
    EntityManager::GetCurrentScene()->SetBound(worldBound);
    if (renderManager.m_mainCameraComponent != nullptr)
    {
        if (const auto mainCameraEntity = renderManager.m_mainCameraComponent->GetOwner(); mainCameraEntity.IsEnabled())
        {
            RenderShadows(worldBound, *renderManager.m_mainCameraComponent, mainCameraEntity);
        }
    }
    ProfilerManager::EndEvent("Shadowmap Prepass");
#pragma endregion
#pragma region Render to cameras
    ProfilerManager::StartEvent("Main Rendering");
    renderManager.m_triangles = 0;
    renderManager.m_drawCall = 0;
    if (renderManager.m_mainCameraComponent != nullptr)
    {
        if (renderManager.m_mainCameraComponent->m_allowAutoResize)
            renderManager.m_mainCameraComponent->ResizeResolution(
                renderManager.m_mainCameraResolutionX, renderManager.m_mainCameraResolutionY);
    }

    if (cameraEntities != nullptr)
    {
        for (auto cameraEntity : *cameraEntities)
        {
            if (!cameraEntity.IsEnabled())
                continue;
            auto &cameraComponent = cameraEntity.GetPrivateComponent<Camera>();
            if (cameraComponent.IsEnabled())
            {
                auto ltw = cameraEntity.GetDataComponent<GlobalTransform>();
                Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, ltw.GetPosition(), ltw.GetRotation());
                Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
                ApplyShadowMapSettings(cameraComponent);
                ApplyEnvironmentalSettings(cameraComponent);
                RenderToCamera(cameraComponent);
            }
        }
    }
    ProfilerManager::EndEvent("Main Rendering");
#pragma endregion
#pragma region Post - processing
    ProfilerManager::StartEvent("Post Processing");
    const std::vector<Entity> *postProcessingEntities =
        EntityManager::UnsafeGetPrivateComponentOwnersList<PostProcessing>();
    if (postProcessingEntities != nullptr)
    {
        for (auto postProcessingEntity : *postProcessingEntities)
        {
            if (!postProcessingEntity.IsEnabled())
                continue;
            auto &postProcessing = postProcessingEntity.GetPrivateComponent<PostProcessing>();
            if (postProcessing.IsEnabled())
                postProcessing.Process();
        }
    }
    ProfilerManager::EndEvent("Post Processing");
#pragma endregion

    ProfilerManager::EndEvent("RenderManager");
}
glm::vec3 RenderManager::ClosestPointOnLine(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b)
{
    const float lineLength = distance(a, b);
    const glm::vec3 vector = point - a;
    const glm::vec3 lineDirection = (b - a) / lineLength;

    // Project Vector to LineDirection to get the distance of point from a
    const float distance = dot(vector, lineDirection);
    return a + lineDirection * distance;
}
void RenderManager::OnGui()
{
    auto &renderManager = GetInstance();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Render Manager", &renderManager.m_enableRenderMenu);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (renderManager.m_enableRenderMenu)
    {
        ImGui::Begin("Render Manager");
        ImGui::DragFloat("Gamma", &renderManager.m_lightSettings.m_gamma, 0.01f, 1.0f, 3.0f);
        if (ImGui::CollapsingHeader("Environment Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Environmental map:");
            ImGui::SameLine();
            EditorManager::DragAndDrop(renderManager.m_environmentalMap);
            ImGui::DragFloat(
                "Environmental light intensity", &renderManager.m_lightSettings.m_ambientLight, 0.01f, 0.0f, 2.0f);
        }
        bool enableShadow = renderManager.m_materialSettings.m_enableShadow;
        if (ImGui::Checkbox("Enable shadow", &enableShadow))
        {
            renderManager.m_materialSettings.m_enableShadow = enableShadow;
        }
        if (renderManager.m_materialSettings.m_enableShadow &&
            ImGui::CollapsingHeader("Shadow", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNode("Distance"))
            {
                ImGui::DragFloat("Max shadow distance", &renderManager.m_maxShadowDistance, 1.0f, 0.1f);
                ImGui::DragFloat(
                    "Split 1",
                    &renderManager.m_shadowCascadeSplit[0],
                    0.01f,
                    0.0f,
                    renderManager.m_shadowCascadeSplit[1]);
                ImGui::DragFloat(
                    "Split 2",
                    &renderManager.m_shadowCascadeSplit[1],
                    0.01f,
                    renderManager.m_shadowCascadeSplit[0],
                    renderManager.m_shadowCascadeSplit[2]);
                ImGui::DragFloat(
                    "Split 3",
                    &renderManager.m_shadowCascadeSplit[2],
                    0.01f,
                    renderManager.m_shadowCascadeSplit[1],
                    renderManager.m_shadowCascadeSplit[3]);
                ImGui::DragFloat(
                    "Split 4",
                    &renderManager.m_shadowCascadeSplit[3],
                    0.01f,
                    renderManager.m_shadowCascadeSplit[2],
                    1.0f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PCSS"))
            {
                ImGui::DragFloat("PCSS Factor", &renderManager.m_lightSettings.m_scaleFactor, 0.01f, 0.0f);
                ImGui::DragInt(
                    "Blocker search side amount", &renderManager.m_lightSettings.m_blockerSearchAmount, 1, 1, 8);
                ImGui::DragInt("PCF Sample Size", &renderManager.m_lightSettings.m_pcfSampleAmount, 1, 1, 64);
                ImGui::TreePop();
            }
            ImGui::DragFloat("Seam fix ratio", &renderManager.m_lightSettings.m_seamFixRatio, 0.001f, 0.0f, 0.1f);
            ImGui::Checkbox("Stable fit", &renderManager.m_stableFit);
        }
        ImGui::End();
    }
}
void RenderManager::Init()
{
    auto &manager = GetInstance();

    Camera::GenerateMatrices();
    manager.m_materialSettingsBuffer = std::make_unique<OpenGLUtils::GLUBO>();
    manager.m_materialSettingsBuffer->SetData(sizeof(MaterialSettingsBlock), nullptr, GL_STREAM_DRAW);
    manager.m_materialSettingsBuffer->SetBase(6);

    manager.m_environmentalMapSettingsBuffer = std::make_unique<OpenGLUtils::GLUBO>();
    manager.m_environmentalMapSettingsBuffer->SetData(sizeof(EnvironmentalMapSettingsBlock), nullptr, GL_STREAM_DRAW);
    manager.m_environmentalMapSettingsBuffer->SetBase(7);
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
    manager.m_kernelBlock = std::make_unique<OpenGLUtils::GLUBO>();
    manager.m_kernelBlock->SetBase(5);
    manager.m_kernelBlock->SetData(
        sizeof(glm::vec4) * uniformKernel.size() + sizeof(glm::vec4) * gaussianKernel.size(), NULL, GL_STATIC_DRAW);
    manager.m_kernelBlock->SubData(0, sizeof(glm::vec4) * uniformKernel.size(), uniformKernel.data());
    manager.m_kernelBlock->SubData(
        sizeof(glm::vec4) * uniformKernel.size(), sizeof(glm::vec4) * gaussianKernel.size(), gaussianKernel.data());

#pragma endregion
#pragma region Shadow
    manager.m_shadowCascadeInfoBlock.SetData(sizeof(LightSettingsBlock), nullptr, GL_DYNAMIC_DRAW);
    manager.m_shadowCascadeInfoBlock.SetBase(4);

#pragma region LightInfoBlocks
    size_t size = 16 + DefaultResources::ShaderIncludes::MaxDirectionalLightAmount * sizeof(DirectionalLightInfo);
    manager.m_directionalLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    manager.m_directionalLightBlock.SetBase(1);
    size = 16 + DefaultResources::ShaderIncludes::MaxPointLightAmount * sizeof(PointLightInfo);
    manager.m_pointLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    manager.m_pointLightBlock.SetBase(2);
    size = 16 + DefaultResources::ShaderIncludes::MaxSpotLightAmount * sizeof(SpotLightInfo);
    manager.m_spotLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    manager.m_spotLightBlock.SetBase(3);
#pragma endregion
#pragma region DirectionalLight
    manager.m_directionalLightShadowMap =
        std::make_unique<DirectionalLightShadowMap>(manager.m_directionalLightShadowMapResolution);

    std::string vertShaderCode =
        std::string("#version 450 core\n") +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/DirectionalLightShadowMap.vert");
    std::string fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(
            AssetManager::GetResourcePath() + "Shaders/Fragment/DirectionalLightShadowMap.frag");
    std::string geomShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(
            AssetManager::GetResourcePath() + "Shaders/Geometry/DirectionalLightShadowMap.geom");

    auto vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    auto fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    auto geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);
    manager.m_directionalLightProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_directionalLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/DirectionalLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_directionalLightInstancedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_directionalLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/DirectionalLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_directionalLightSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_directionalLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(
            AssetManager::GetResourcePath() + "Shaders/Vertex/DirectionalLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_directionalLightInstancedSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_directionalLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);
#pragma region PointLight
    manager.m_pointLightShadowMap = std::make_unique<PointLightShadowMap>(manager.m_pointLightShadowMapResolution);
    vertShaderCode =
        std::string("#version 450 core\n") +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/PointLightShadowMap.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Fragment/PointLightShadowMap.frag");
    geomShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Geometry/PointLightShadowMap.geom");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);

    manager.m_pointLightProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_pointLightProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/PointLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_pointLightInstancedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_pointLightInstancedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/PointLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_pointLightSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_pointLightSkinnedProgram->Link(vertShader, fragShader, geomShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/PointLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_pointLightInstancedSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_pointLightInstancedSkinnedProgram->Link(vertShader, fragShader, geomShader);
#pragma endregion
#pragma region SpotLight
    manager.m_spotLightShadowMap = std::make_unique<SpotLightShadowMap>(manager.m_spotLightShadowMapResolution);
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/SpotLightShadowMap.vert");
    fragShaderCode =
        std::string("#version 450 core\n") +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Fragment/SpotLightShadowMap.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    manager.m_spotLightProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_spotLightProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/SpotLightShadowMapInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_spotLightInstancedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_spotLightInstancedProgram->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/SpotLightShadowMapSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_spotLightSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_spotLightSkinnedProgram->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Vertex/SpotLightShadowMapInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_spotLightInstancedSkinnedProgram = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_spotLightInstancedSkinnedProgram->Link(vertShader, fragShader);
#pragma endregion
#pragma endregion

#pragma region GBuffer
    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + +"\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/TexturePassThrough.vert");
    fragShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
                     FileSystem::LoadFileAsString(
                         AssetManager::GetResourcePath() + "Shaders/Fragment/StandardDeferredLighting.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    manager.m_gBufferLightingPass = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_gBufferLightingPass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + +"\n" +
                     FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/Standard.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Fragment/StandardDeferred.frag");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    manager.m_gBufferPrepass = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_gBufferPrepass->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + +"\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/StandardSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_gBufferSkinnedPrepass = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_gBufferSkinnedPrepass->Link(vertShader, fragShader);

    vertShaderCode = std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + +"\n" +
                     FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/StandardInstanced.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_gBufferInstancedPrepass = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_gBufferInstancedPrepass->Link(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + +"\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/StandardInstancedSkinned.vert");
    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    manager.m_gBufferInstancedSkinnedPrepass = AssetManager::CreateAsset<OpenGLUtils::GLProgram>();
    manager.m_gBufferInstancedSkinnedPrepass->Link(vertShader, fragShader);
#pragma endregion
#pragma region SSAO
    vertShaderCode =
        std::string("#version 450 core\n") +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Vertex/TexturePassThrough.vert");
    fragShaderCode =
        std::string("#version 450 core\n") + *DefaultResources::ShaderIncludes::Uniform + "\n" +
        FileSystem::LoadFileAsString(AssetManager::GetResourcePath() + "Shaders/Fragment/SSAOGeometry.frag");

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

#pragma endregion

    manager.m_environmentalMap = DefaultResources::Environmental::DefaultEnvironmentalMap;
}
void RenderManager::CollectRenderInstances(Camera &camera, const glm::vec3& position, Bound &worldBound, const bool &calculateBound)
{
    auto &renderManager = GetInstance();
    auto &deferredRenderInstances = renderManager.m_deferredRenderInstances[&camera];
    auto &deferredInstancedRenderInstances = renderManager.m_deferredInstancedRenderInstances[&camera];
    auto &forwardRenderInstances = renderManager.m_forwardRenderInstances[&camera];
    auto &forwardInstancedRenderInstances = renderManager.m_forwardInstancedRenderInstances[&camera];
    auto &transparentRenderInstances = renderManager.m_transparentRenderInstances[&camera];
    auto &instancedTransparentRenderInstances = renderManager.m_instancedTransparentRenderInstances[&camera];

    auto &minBound = worldBound.m_min;
    auto &maxBound = worldBound.m_max;
    if (calculateBound)
    {
        minBound = glm::vec3(INT_MAX);
        maxBound = glm::vec3(INT_MIN);
    }
    const std::vector<Entity> *owners = EntityManager::UnsafeGetPrivateComponentOwnersList<MeshRenderer>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
            if (!mmc.IsEnabled() || mmc.m_material == nullptr || mmc.m_mesh == nullptr)
                continue;
            auto gt = owner.GetDataComponent<GlobalTransform>();
            auto ltw = gt.m_value;
            auto meshBound = mmc.m_mesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();
            if (calculateBound)
            {
                glm::vec3 size = meshBound.Size();
                minBound = glm::vec3(
                    (glm::min)(minBound.x, center.x - size.x),
                    (glm::min)(minBound.y, center.y - size.y),
                    (glm::min)(minBound.z, center.z - size.z));
                maxBound = glm::vec3(
                    (glm::max)(maxBound.x, center.x + size.x),
                    (glm::max)(maxBound.y, center.y + size.y),
                    (glm::max)(maxBound.z, center.z + size.z));
            }
            auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
            float distance = glm::distance(glm::vec3(meshCenter), position);
            RenderCommand renderInstance;
            renderInstance.m_owner = owner;
            renderInstance.m_globalTransform = gt;
            renderInstance.m_mesh = mmc.m_mesh;
            renderInstance.m_castShadow = mmc.m_castShadow;
            renderInstance.m_receiveShadow = mmc.m_receiveShadow;
            renderInstance.m_meshType = RenderCommandMeshType::Default;
            if (mmc.m_material->m_blendingMode != MaterialBlendingMode::Off)
            {
                transparentRenderInstances[mmc.m_material]
                    .m_meshes[distance][renderInstance.m_mesh->m_vao.get()]
                    .push_back(renderInstance);
            }
            else if (mmc.m_forwardRendering)
            {
                forwardRenderInstances[mmc.m_material].m_meshes[distance][renderInstance.m_mesh->m_vao.get()].push_back(
                    renderInstance);
            }
            else
            {
                deferredRenderInstances[mmc.m_material]
                    .m_meshes[distance][renderInstance.m_mesh->m_vao.get()]
                    .push_back(renderInstance);
            }
        }
    }
    owners = EntityManager::UnsafeGetPrivateComponentOwnersList<Particles>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &particles = owner.GetPrivateComponent<Particles>();
            if (!particles.IsEnabled() || particles.m_material == nullptr || particles.m_mesh == nullptr)
                continue;
            auto gt = owner.GetDataComponent<GlobalTransform>();
            auto ltw = gt.m_value;
            auto meshBound = particles.m_mesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();
            if (calculateBound)
            {
                glm::vec3 size = meshBound.Size();
                minBound = glm::vec3(
                    (glm::min)(minBound.x, center.x - size.x),
                    (glm::min)(minBound.y, center.y - size.y),
                    (glm::min)(minBound.z, center.z - size.z));

                maxBound = glm::vec3(
                    (glm::max)(maxBound.x, center.x + size.x),
                    (glm::max)(maxBound.y, center.y + size.y),
                    (glm::max)(maxBound.z, center.z + size.z));
            }
            auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
            float distance = glm::distance(glm::vec3(meshCenter), position);
            RenderCommand renderInstance;
            renderInstance.m_owner = owner;
            renderInstance.m_globalTransform = gt;
            renderInstance.m_mesh = particles.m_mesh;
            renderInstance.m_castShadow = particles.m_castShadow;
            renderInstance.m_receiveShadow = particles.m_receiveShadow;
            renderInstance.m_matrices = particles.m_matrices;
            renderInstance.m_meshType = RenderCommandMeshType::Default;
            if (particles.m_material->m_blendingMode != MaterialBlendingMode::Off)
            {
                instancedTransparentRenderInstances[particles.m_material]
                    .m_meshes[distance][renderInstance.m_mesh->m_vao.get()]
                    .push_back(renderInstance);
            }
            else if (particles.m_forwardRendering)
            {
                forwardInstancedRenderInstances[particles.m_material]
                    .m_meshes[distance][renderInstance.m_mesh->m_vao.get()]
                    .push_back(renderInstance);
            }
            else
            {
                deferredInstancedRenderInstances[particles.m_material]
                    .m_meshes[distance][renderInstance.m_mesh->m_vao.get()]
                    .push_back(renderInstance);
            }
        }
    }
    owners = EntityManager::UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &smmc = owner.GetPrivateComponent<SkinnedMeshRenderer>();
            if (!smmc.IsEnabled() || smmc.m_material == nullptr || smmc.m_skinnedMesh == nullptr)
                continue;
            GlobalTransform gt;
            if (smmc.m_animator.IsValid())
            {
                gt = smmc.m_animator.GetDataComponent<GlobalTransform>();
            }
            else
            {
                smmc.m_animator = Entity();
                gt = owner.GetDataComponent<GlobalTransform>();
            }
            auto ltw = gt.m_value;
            auto meshBound = smmc.m_skinnedMesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();
            if (calculateBound)
            {
                glm::vec3 size = meshBound.Size();
                minBound = glm::vec3(
                    (glm::min)(minBound.x, center.x - size.x),
                    (glm::min)(minBound.y, center.y - size.y),
                    (glm::min)(minBound.z, center.z - size.z));
                maxBound = glm::vec3(
                    (glm::max)(maxBound.x, center.x + size.x),
                    (glm::max)(maxBound.y, center.y + size.y),
                    (glm::max)(maxBound.z, center.z + size.z));
            }
            auto meshCenter = gt.m_value * glm::vec4(center, 1.0);
            float distance = glm::distance(glm::vec3(meshCenter), position);
            RenderCommand renderInstance;
            renderInstance.m_owner = owner;
            renderInstance.m_globalTransform = gt;
            renderInstance.m_skinnedMesh = smmc.m_skinnedMesh;
            renderInstance.m_castShadow = smmc.m_castShadow;
            renderInstance.m_receiveShadow = smmc.m_receiveShadow;
            renderInstance.m_meshType = RenderCommandMeshType::Skinned;
            renderInstance.m_boneMatrices = smmc.m_finalResults;
            if (smmc.m_material->m_blendingMode != MaterialBlendingMode::Off)
            {
                transparentRenderInstances[smmc.m_material]
                    .m_skinnedMeshes[distance][renderInstance.m_skinnedMesh->m_vao.get()]
                    .push_back(renderInstance);
            }
            else if (smmc.m_forwardRendering)
            {
                forwardRenderInstances[smmc.m_material]
                    .m_skinnedMeshes[distance][renderInstance.m_skinnedMesh->m_vao.get()]
                    .push_back(renderInstance);
            }
            else
            {
                deferredRenderInstances[smmc.m_material]
                    .m_skinnedMeshes[distance][renderInstance.m_skinnedMesh->m_vao.get()]
                    .push_back(renderInstance);
            }
        }
    }
}
#pragma region Helpers
inline float RenderManager::Lerp(const float &a, const float &b, const float &f)
{
    return a + f * (b - a);
}
unsigned int environmentalMapCubeVAO = 0;
unsigned int environmentalMapCubeVBO = 0;
void RenderManager::RenderCube()
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
void RenderManager::RenderQuad()
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
#pragma region Shadow
void RenderManager::SetSplitRatio(const float &r1, const float &r2, const float &r3, const float &r4)
{
    GetInstance().m_shadowCascadeSplit[0] = r1;
    GetInstance().m_shadowCascadeSplit[1] = r2;
    GetInstance().m_shadowCascadeSplit[2] = r3;
    GetInstance().m_shadowCascadeSplit[3] = r4;
}
void RenderManager::SetDirectionalLightShadowMapResolution(const size_t &value)
{
    GetInstance().m_directionalLightShadowMapResolution = value;
    if (GetInstance().m_directionalLightShadowMap != nullptr)
        GetInstance().m_directionalLightShadowMap->SetResolution(value);
}

void RenderManager::SetPointLightShadowMapResolution(const size_t &value)
{
    GetInstance().m_pointLightShadowMapResolution = value;
    if (GetInstance().m_pointLightShadowMap != nullptr)
        GetInstance().m_pointLightShadowMap->SetResolution(value);
}

void RenderManager::SetSpotLightShadowMapResolution(const size_t &value)
{
    GetInstance().m_spotLightShadowMapResolution = value;
    if (GetInstance().m_spotLightShadowMap != nullptr)
        GetInstance().m_spotLightShadowMap->SetResolution(value);
}
void RenderManager::ShadowMapPrePass(
    const int &enabledSize,
    std::shared_ptr<OpenGLUtils::GLProgram> &defaultProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &defaultInstancedProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &skinnedProgram,
    std::shared_ptr<OpenGLUtils::GLProgram> &instancedSkinnedProgram)
{
    auto &renderManager = GetInstance();
    for (auto &i : renderManager.m_deferredRenderInstances)
    {
        const auto &cameraComponent = i.first;
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto &program = defaultProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_mesh->Draw();
                    break;
                }
                case RenderCommandMeshType::Skinned: {
                    auto &program = skinnedProgram;
                    program->Bind();
                    renderCommand.m_boneMatrices->UploadBones(renderCommand.m_skinnedMesh);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_skinnedMesh->Draw();
                    break;
                }
                }
            },
            false,
            false);
    }
    for (auto &i : renderManager.m_deferredInstancedRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto &program = defaultInstancedProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_mesh->DrawInstanced(renderCommand.m_matrices->m_value);
                    break;
                }
                }
            },
            false,
            false);
    }
    for (auto &i : renderManager.m_forwardRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto &program = defaultProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_mesh->Draw();
                    break;
                }
                case RenderCommandMeshType::Skinned: {
                    auto &program = skinnedProgram;
                    program->Bind();
                    renderCommand.m_boneMatrices->UploadBones(renderCommand.m_skinnedMesh);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_skinnedMesh->Draw();
                    break;
                }
                }
            },
            false,
            false);
    }
    for (auto &i : renderManager.m_forwardInstancedRenderInstances)
    {
        DispatchRenderCommands(
            i.second,
            [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                switch (renderCommand.m_meshType)
                {
                case RenderCommandMeshType::Default: {
                    auto &program = defaultInstancedProgram;
                    program->Bind();
                    program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                    program->SetInt("index", enabledSize);
                    renderCommand.m_mesh->DrawInstanced(renderCommand.m_matrices->m_value);
                    break;
                }
                }
            },
            false,
            false);
    }
}
void RenderManager::RenderShadows(Bound &worldBound, Camera &cameraComponent, const Entity &mainCameraEntity)
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    auto &renderManager = GetInstance();
#pragma region Shadow
    auto &minBound = worldBound.m_min;
    auto &maxBound = worldBound.m_max;

    if (renderManager.m_mainCameraComponent->IsEnabled())
    {
        auto ltw = mainCameraEntity.GetDataComponent<GlobalTransform>();
        glm::vec3 mainCameraPos = ltw.GetPosition();
        glm::quat mainCameraRot = ltw.GetRotation();
        renderManager.m_shadowCascadeInfoBlock.SubData(0, sizeof(LightSettingsBlock), &renderManager.m_lightSettings);
        const std::vector<Entity> *directionalLightEntities =
            EntityManager::UnsafeGetPrivateComponentOwnersList<DirectionalLight>();
        size_t size = 0;
        if (directionalLightEntities && !directionalLightEntities->empty())
        {
            size = directionalLightEntities->size();
            int enabledSize = 0;
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = directionalLightEntities->at(i);
                if (!lightEntity.IsEnabled())
                    continue;
                const auto &dlc = lightEntity.GetPrivateComponent<DirectionalLight>();
                if (!dlc.IsEnabled())
                    continue;
                glm::quat rotation = lightEntity.GetDataComponent<GlobalTransform>().GetRotation();
                glm::vec3 lightDir = glm::normalize(rotation * glm::vec3(0, 0, 1));
                float planeDistance = 0;
                glm::vec3 center;
                renderManager.m_directionalLights[enabledSize].m_direction = glm::vec4(lightDir, 0.0f);
                renderManager.m_directionalLights[enabledSize].m_diffuse =
                    glm::vec4(dlc.m_diffuse * dlc.m_diffuseBrightness, dlc.m_castShadow);
                renderManager.m_directionalLights[enabledSize].m_specular = glm::vec4(0.0f);
                for (int split = 0; split < DefaultResources::ShaderIncludes::ShadowCascadeAmount; split++)
                {
                    float splitStart = 0;
                    float splitEnd = renderManager.m_maxShadowDistance;
                    if (split != 0)
                        splitStart = renderManager.m_maxShadowDistance * renderManager.m_shadowCascadeSplit[split - 1];
                    if (split != DefaultResources::ShaderIncludes::ShadowCascadeAmount - 1)
                        splitEnd = renderManager.m_maxShadowDistance * renderManager.m_shadowCascadeSplit[split];
                    renderManager.m_lightSettings.m_splitDistance[split] = splitEnd;
                    glm::mat4 lightProjection, lightView;
                    float max = 0;
                    glm::vec3 lightPos;
                    glm::vec3 cornerPoints[8];
                    Camera::CalculateFrustumPoints(
                        cameraComponent, splitStart, splitEnd, mainCameraPos, mainCameraRot, cornerPoints);
                    glm::vec3 cameraFrustumCenter =
                        (mainCameraRot * glm::vec3(0, 0, -1)) * ((splitEnd - splitStart) / 2.0f + splitStart) +
                        mainCameraPos;
                    if (renderManager.m_stableFit)
                    {
                        // Less detail but no shimmering when rotating the camera.
                        // max = glm::distance(cornerPoints[4], cameraFrustumCenter);
                        max = splitEnd;
                    }
                    else
                    {
                        // More detail but cause shimmering when rotating camera.
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[0], ClosestPointOnLine(cornerPoints[0], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[1], ClosestPointOnLine(cornerPoints[1], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[2], ClosestPointOnLine(cornerPoints[2], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[3], ClosestPointOnLine(cornerPoints[3], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[4], ClosestPointOnLine(cornerPoints[4], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[5], ClosestPointOnLine(cornerPoints[5], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[6], ClosestPointOnLine(cornerPoints[6], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                        max =
                            (glm::
                                 max)(max, glm::distance(cornerPoints[7], ClosestPointOnLine(cornerPoints[7], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
                    }

                    glm::vec3 p0 = ClosestPointOnLine(
                        glm::vec3(maxBound.x, maxBound.y, maxBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);
                    glm::vec3 p7 = ClosestPointOnLine(
                        glm::vec3(minBound.x, minBound.y, minBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);

                    float d0 = glm::distance(p0, p7);

                    glm::vec3 p1 = ClosestPointOnLine(
                        glm::vec3(maxBound.x, maxBound.y, minBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);
                    glm::vec3 p6 = ClosestPointOnLine(
                        glm::vec3(minBound.x, minBound.y, maxBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);

                    float d1 = glm::distance(p1, p6);

                    glm::vec3 p2 = ClosestPointOnLine(
                        glm::vec3(maxBound.x, minBound.y, maxBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);
                    glm::vec3 p5 = ClosestPointOnLine(
                        glm::vec3(minBound.x, maxBound.y, minBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);

                    float d2 = glm::distance(p2, p5);

                    glm::vec3 p3 = ClosestPointOnLine(
                        glm::vec3(maxBound.x, minBound.y, minBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);
                    glm::vec3 p4 = ClosestPointOnLine(
                        glm::vec3(minBound.x, maxBound.y, maxBound.z),
                        cameraFrustumCenter,
                        cameraFrustumCenter + lightDir);

                    float d3 = glm::distance(p3, p4);

                    center =
                        ClosestPointOnLine(worldBound.Center(), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                    planeDistance = (glm::max)((glm::max)(d0, d1), (glm::max)(d2, d3));
                    lightPos = center - lightDir * planeDistance;
                    lightView =
                        glm::lookAt(lightPos, lightPos + lightDir, glm::normalize(rotation * glm::vec3(0, 1, 0)));
                    lightProjection = glm::ortho(-max, max, -max, max, 0.0f, planeDistance * 2.0f);
                    switch (enabledSize)
                    {
                    case 0:
                        renderManager.m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                            0,
                            0,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2);
                        break;
                    case 1:
                        renderManager.m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            0,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2);
                        break;
                    case 2:
                        renderManager.m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                            0,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2);
                        break;
                    case 3:
                        renderManager.m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2,
                            renderManager.m_directionalLightShadowMapResolution / 2);
                        break;
                    }

#pragma region Fix Shimmering due to the movement of the camera

                    glm::mat4 shadowMatrix = lightProjection * lightView;
                    glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                    shadowOrigin = shadowMatrix * shadowOrigin;
                    GLfloat storedW = shadowOrigin.w;
                    shadowOrigin =
                        shadowOrigin * (float)renderManager.m_directionalLights[enabledSize].m_viewPort.z / 2.0f;
                    glm::vec4 roundedOrigin = glm::round(shadowOrigin);
                    glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
                    roundOffset =
                        roundOffset * 2.0f / (float)renderManager.m_directionalLights[enabledSize].m_viewPort.z;
                    roundOffset.z = 0.0f;
                    roundOffset.w = 0.0f;
                    glm::mat4 shadowProj = lightProjection;
                    shadowProj[3] += roundOffset;
                    lightProjection = shadowProj;
#pragma endregion
                    renderManager.m_directionalLights[enabledSize].m_lightSpaceMatrix[split] =
                        lightProjection * lightView;
                    renderManager.m_directionalLights[enabledSize].m_lightFrustumWidth[split] = max;
                    renderManager.m_directionalLights[enabledSize].m_lightFrustumDistance[split] = planeDistance;
                    if (split == DefaultResources::ShaderIncludes::ShadowCascadeAmount - 1)
                        renderManager.m_directionalLights[enabledSize].m_reservedParameters =
                            glm::vec4(dlc.m_lightSize, 0, dlc.m_bias, dlc.m_normalOffset);
                }
                enabledSize++;
            }
            renderManager.m_directionalLightBlock.SubData(0, 4, &enabledSize);
            if (enabledSize != 0)
            {
                renderManager.m_directionalLightBlock.SubData(
                    16, enabledSize * sizeof(DirectionalLightInfo), &renderManager.m_directionalLights[0]);
            }
            if (renderManager.m_materialSettings.m_enableShadow)
            {
                renderManager.m_directionalLightShadowMap->Bind();
                renderManager.m_directionalLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                glClear(GL_DEPTH_BUFFER_BIT);
                enabledSize = 0;
                renderManager.m_directionalLightProgram->Bind();
                for (int i = 0; i < size; i++)
                {
                    Entity lightEntity = directionalLightEntities->at(i);
                    if (!lightEntity.IsEnabled())
                        continue;
                    glViewport(
                        renderManager.m_directionalLights[enabledSize].m_viewPort.x,
                        renderManager.m_directionalLights[enabledSize].m_viewPort.y,
                        renderManager.m_directionalLights[enabledSize].m_viewPort.z,
                        renderManager.m_directionalLights[enabledSize].m_viewPort.w);
                    renderManager.m_directionalLightProgram->SetInt("index", enabledSize);
                    ShadowMapPrePass(
                        enabledSize,
                        renderManager.m_directionalLightProgram,
                        renderManager.m_directionalLightInstancedProgram,
                        renderManager.m_directionalLightSkinnedProgram,
                        renderManager.m_directionalLightInstancedSkinnedProgram);
                    enabledSize++;
                }
            }
        }
        else
        {
            renderManager.m_directionalLightBlock.SubData(0, 4, &size);
        }
        const std::vector<Entity> *pointLightEntities =
            EntityManager::UnsafeGetPrivateComponentOwnersList<PointLight>();
        size = 0;
        if (pointLightEntities && !pointLightEntities->empty())
        {
            size = pointLightEntities->size();
            size_t enabledSize = 0;
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = pointLightEntities->at(i);
                if (!lightEntity.IsEnabled())
                    continue;
                const auto &plc = lightEntity.GetPrivateComponent<PointLight>();
                if (!plc.IsEnabled())
                    continue;
                glm::vec3 position = lightEntity.GetDataComponent<GlobalTransform>().m_value[3];
                renderManager.m_pointLights[enabledSize].m_position = glm::vec4(position, 0);
                renderManager.m_pointLights[enabledSize].m_constantLinearQuadFarPlane.x = plc.m_constant;
                renderManager.m_pointLights[enabledSize].m_constantLinearQuadFarPlane.y = plc.m_linear;
                renderManager.m_pointLights[enabledSize].m_constantLinearQuadFarPlane.z = plc.m_quadratic;
                renderManager.m_pointLights[enabledSize].m_diffuse =
                    glm::vec4(plc.m_diffuse * plc.m_diffuseBrightness, plc.m_castShadow);
                renderManager.m_pointLights[enabledSize].m_specular = glm::vec4(0);
                renderManager.m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w = plc.GetFarPlane();

                glm::mat4 shadowProj = glm::perspective(
                    glm::radians(90.0f),
                    renderManager.m_pointLightShadowMap->GetResolutionRatio(),
                    1.0f,
                    renderManager.m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w);
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[0] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[1] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[2] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[3] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[4] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                renderManager.m_pointLights[enabledSize].m_lightSpaceMatrix[5] =
                    shadowProj *
                    glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                renderManager.m_pointLights[enabledSize].m_reservedParameters =
                    glm::vec4(plc.m_bias, plc.m_lightSize, 0, 0);

                switch (enabledSize)
                {
                case 0:
                    renderManager.m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                        0,
                        0,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2);
                    break;
                case 1:
                    renderManager.m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                        renderManager.m_pointLightShadowMapResolution / 2,
                        0,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2);
                    break;
                case 2:
                    renderManager.m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                        0,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2);
                    break;
                case 3:
                    renderManager.m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2,
                        renderManager.m_pointLightShadowMapResolution / 2);
                    break;
                }
                enabledSize++;
            }
            renderManager.m_pointLightBlock.SubData(0, 4, &enabledSize);
            if (enabledSize != 0)
                renderManager.m_pointLightBlock.SubData(
                    16, enabledSize * sizeof(PointLightInfo), &renderManager.m_pointLights[0]);
            if (renderManager.m_materialSettings.m_enableShadow)
            {
                renderManager.m_pointLightShadowMap->Bind();
                renderManager.m_pointLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                glClear(GL_DEPTH_BUFFER_BIT);
                enabledSize = 0;
                for (int i = 0; i < size; i++)
                {
                    Entity lightEntity = pointLightEntities->at(i);
                    if (!lightEntity.IsEnabled())
                        continue;
                    glViewport(
                        renderManager.m_pointLights[enabledSize].m_viewPort.x,
                        renderManager.m_pointLights[enabledSize].m_viewPort.y,
                        renderManager.m_pointLights[enabledSize].m_viewPort.z,
                        renderManager.m_pointLights[enabledSize].m_viewPort.w);
                    ShadowMapPrePass(
                        enabledSize,
                        renderManager.m_pointLightProgram,
                        renderManager.m_pointLightInstancedProgram,
                        renderManager.m_pointLightSkinnedProgram,
                        renderManager.m_pointLightInstancedSkinnedProgram);
                    enabledSize++;
                }
            }
        }
        else
        {
            renderManager.m_pointLightBlock.SubData(0, 4, &size);
        }
        const std::vector<Entity> *spotLightEntities = EntityManager::UnsafeGetPrivateComponentOwnersList<SpotLight>();
        size = 0;
        if (spotLightEntities && !spotLightEntities->empty())
        {
            size = spotLightEntities->size();
            size_t enabledSize = 0;
            for (int i = 0; i < size; i++)
            {
                Entity lightEntity = spotLightEntities->at(i);
                if (!lightEntity.IsEnabled())
                    continue;
                const auto &slc = lightEntity.GetPrivateComponent<SpotLight>();
                if (!slc.IsEnabled())
                    continue;
                auto ltw = lightEntity.GetDataComponent<GlobalTransform>();
                glm::vec3 position = ltw.m_value[3];
                glm::vec3 front = ltw.GetRotation() * glm::vec3(0, 0, -1);
                glm::vec3 up = ltw.GetRotation() * glm::vec3(0, 1, 0);
                renderManager.m_spotLights[enabledSize].m_position = glm::vec4(position, 0);
                renderManager.m_spotLights[enabledSize].m_direction = glm::vec4(front, 0);
                renderManager.m_spotLights[enabledSize].m_constantLinearQuadFarPlane.x = slc.m_constant;
                renderManager.m_spotLights[enabledSize].m_constantLinearQuadFarPlane.y = slc.m_linear;
                renderManager.m_spotLights[enabledSize].m_constantLinearQuadFarPlane.z = slc.m_quadratic;
                renderManager.m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w = slc.GetFarPlane();
                renderManager.m_spotLights[enabledSize].m_diffuse =
                    glm::vec4(slc.m_diffuse * slc.m_diffuseBrightness, slc.m_castShadow);
                renderManager.m_spotLights[enabledSize].m_specular = glm::vec4(0);

                glm::mat4 shadowProj = glm::perspective(
                    glm::radians(slc.m_outerDegrees * 2.0f),
                    renderManager.m_spotLightShadowMap->GetResolutionRatio(),
                    1.0f,
                    renderManager.m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w);
                renderManager.m_spotLights[enabledSize].m_lightSpaceMatrix =
                    shadowProj * glm::lookAt(position, position + front, up);
                renderManager.m_spotLights[enabledSize].m_cutOffOuterCutOffLightSizeBias = glm::vec4(
                    glm::cos(glm::radians(slc.m_innerDegrees)),
                    glm::cos(glm::radians(slc.m_outerDegrees)),
                    slc.m_lightSize,
                    slc.m_bias);

                switch (enabledSize)
                {
                case 0:
                    renderManager.m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                        0,
                        0,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2);
                    break;
                case 1:
                    renderManager.m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                        renderManager.m_spotLightShadowMapResolution / 2,
                        0,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2);
                    break;
                case 2:
                    renderManager.m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                        0,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2);
                    break;
                case 3:
                    renderManager.m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2,
                        renderManager.m_spotLightShadowMapResolution / 2);
                    break;
                }
                enabledSize++;
            }
            renderManager.m_spotLightBlock.SubData(0, 4, &enabledSize);
            if (enabledSize != 0)
                renderManager.m_spotLightBlock.SubData(
                    16, enabledSize * sizeof(SpotLightInfo), &renderManager.m_spotLights[0]);
            if (renderManager.m_materialSettings.m_enableShadow)
            {
                renderManager.m_spotLightShadowMap->Bind();
                renderManager.m_spotLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                glClear(GL_DEPTH_BUFFER_BIT);
                enabledSize = 0;
                for (int i = 0; i < size; i++)
                {
                    Entity lightEntity = spotLightEntities->at(i);
                    if (!lightEntity.IsEnabled())
                        continue;
                    glViewport(
                        renderManager.m_spotLights[enabledSize].m_viewPort.x,
                        renderManager.m_spotLights[enabledSize].m_viewPort.y,
                        renderManager.m_spotLights[enabledSize].m_viewPort.z,
                        renderManager.m_spotLights[enabledSize].m_viewPort.w);
                    ShadowMapPrePass(
                        enabledSize,
                        renderManager.m_spotLightProgram,
                        renderManager.m_spotLightInstancedProgram,
                        renderManager.m_spotLightSkinnedProgram,
                        renderManager.m_spotLightInstancedSkinnedProgram);
                    enabledSize++;
                }
#pragma endregion
            }
        }
        else
        {
            renderManager.m_spotLightBlock.SubData(0, 4, &size);
        }
    }
#pragma endregion
}
#pragma endregion
#pragma region RenderAPI
#pragma region Internal

void RenderManager::ApplyShadowMapSettings(const Camera &cameraComponent)
{
    auto &renderManager = GetInstance();
#pragma region Shadow map binding and default texture binding.
    const bool supportBindlessTexture = OpenGLUtils::GetInstance().m_enableBindlessTexture;
    if (!supportBindlessTexture)
    {
        renderManager.m_directionalLightShadowMap->DepthMapArray()->Bind(0);
        renderManager.m_pointLightShadowMap->DepthMapArray()->Bind(1);
        renderManager.m_spotLightShadowMap->DepthMap()->Bind(2);
    }
#pragma endregion
}

void RenderManager::ApplyEnvironmentalSettings(const Camera &cameraComponent)
{
    auto &manager = GetInstance();
    const bool supportBindlessTexture = OpenGLUtils::GetInstance().m_enableBindlessTexture;
    manager.m_environmentalMapSettings.m_backgroundColor =
        glm::vec4(cameraComponent.m_clearColor, cameraComponent.m_useClearColor);

    if (cameraComponent.m_skybox)
    {
        manager.m_environmentalMapSettings.m_skyboxGamma = cameraComponent.m_skybox->m_gamma;
    }
    else
    {
        manager.m_environmentalMapSettings.m_skyboxGamma =
            DefaultResources::Environmental::DefaultEnvironmentalMap->m_gamma;
    }

    const bool environmentalReady = manager.m_environmentalMap && manager.m_environmentalMap->m_ready;
    if (environmentalReady)
    {
        manager.m_environmentalMapSettings.m_environmentalLightingGamma = manager.m_environmentalMap->m_gamma;
    }
    else
    {
        manager.m_environmentalMapSettings.m_environmentalLightingGamma =
            DefaultResources::Environmental::DefaultEnvironmentalMap->m_gamma;
    }
    if (supportBindlessTexture)
    {
        if (cameraComponent.m_skybox)
        {
            manager.m_environmentalMapSettings.m_skybox = cameraComponent.m_skybox->Texture()->GetHandle();
        }
        else
        {
            manager.m_environmentalMapSettings.m_skybox =
                DefaultResources::Environmental::DefaultEnvironmentalMap->m_targetCubemap->Texture()->GetHandle();
        }
        manager.m_environmentalMapSettings.m_environmentalBrdfLut = manager.m_brdfLut->Texture()->GetHandle();
        if (environmentalReady)
        {
            manager.m_environmentalMapSettings.m_environmentalIrradiance =
                manager.m_environmentalMap->m_lightProbe->m_irradianceMap->Texture()->GetHandle();
            manager.m_environmentalMapSettings.m_environmentalPrefiltered =
                manager.m_environmentalMap->m_reflectionProbe->m_preFilteredMap->Texture()->GetHandle();
        }
        else
        {
            manager.m_environmentalMapSettings.m_environmentalIrradiance =
                DefaultResources::Environmental::DefaultEnvironmentalMap->m_lightProbe->m_irradianceMap->Texture()
                    ->GetHandle();
            manager.m_environmentalMapSettings.m_environmentalPrefiltered =
                DefaultResources::Environmental::DefaultEnvironmentalMap->m_reflectionProbe->m_preFilteredMap->Texture()
                    ->GetHandle();
        }
    }
    else
    {
        if (cameraComponent.m_skybox)
        {
            cameraComponent.m_skybox->Texture()->Bind(8);
        }
        else
        {
            DefaultResources::Environmental::DefaultEnvironmentalMap->m_targetCubemap->Texture()->Bind(8);
        }
        manager.m_brdfLut->Texture()->Bind(11);
        if (environmentalReady)
        {
            manager.m_environmentalMap->m_lightProbe->m_irradianceMap->Texture()->Bind(9);
            manager.m_environmentalMap->m_reflectionProbe->m_preFilteredMap->Texture()->Bind(10);
        }
        else
        {
            DefaultResources::Environmental::DefaultEnvironmentalMap->m_lightProbe->m_irradianceMap->Texture()->Bind(9);
            DefaultResources::Environmental::DefaultEnvironmentalMap->m_reflectionProbe->m_preFilteredMap->Texture()
                ->Bind(10);
        }
    }
    manager.m_environmentalMapSettingsBuffer->SubData(
        0, sizeof(EnvironmentalMapSettingsBlock), &manager.m_environmentalMapSettings);
}

void RenderManager::MaterialPropertySetter(const std::shared_ptr<Material> &material, const bool &disableBlending)
{
    switch (material->m_polygonMode)
    {
    case MaterialPolygonMode::Fill:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case MaterialPolygonMode::Line:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case MaterialPolygonMode::Point:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    }

    switch (material->m_cullingMode)
    {
    case MaterialCullingMode::Off:
        glDisable(GL_CULL_FACE);
        break;
    case MaterialCullingMode::Front:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case MaterialCullingMode::Back:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    }
    if (disableBlending)
        glDisable(GL_BLEND);
    else
    {
        switch (material->m_blendingMode)
        {
        case MaterialBlendingMode::Off:
            break;
        case MaterialBlendingMode::OneMinusSrcAlpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
    }
    glEnable(GL_DEPTH_TEST);
}

void RenderManager::ApplyMaterialSettings(const std::shared_ptr<Material> &material)
{
    auto &manager = GetInstance();
    const bool supportBindlessTexture = OpenGLUtils::GetInstance().m_enableBindlessTexture;
    bool hasAlbedo = false;
    auto search = material->m_textures.find(TextureType::Albedo);
    if (search != material->m_textures.end() && search->second)
    {
        hasAlbedo = true;
        if (supportBindlessTexture)
        {
            manager.m_materialSettings.m_albedoMap = search->second->Texture()->GetHandle();
        }
        else
        {
            search->second->Texture()->Bind(3);

            manager.m_materialSettings.m_albedoMap = 0;
        }
        manager.m_materialSettings.m_albedoEnabled = static_cast<int>(true);
    }
    search = material->m_textures.find(TextureType::Normal);
    if (search != material->m_textures.end() && search->second)
    {
        if (supportBindlessTexture)
        {
            manager.m_materialSettings.m_normalMap = search->second->Texture()->GetHandle();
        }
        else
        {
            search->second->Texture()->Bind(4);

            manager.m_materialSettings.m_normalMap = 0;
        }
        manager.m_materialSettings.m_normalEnabled = static_cast<int>(true);
    }
    search = material->m_textures.find(TextureType::Roughness);
    if (search != material->m_textures.end() && search->second)
    {
        if (supportBindlessTexture)
        {
            manager.m_materialSettings.m_roughnessMap = search->second->Texture()->GetHandle();
        }
        else
        {
            search->second->Texture()->Bind(6);
            manager.m_materialSettings.m_roughnessMap = 0;
        }
        manager.m_materialSettings.m_roughnessEnabled = static_cast<int>(true);
    }
    search = material->m_textures.find(TextureType::Metallic);
    if (search != material->m_textures.end() && search->second)
    {
        if (supportBindlessTexture)
        {
            manager.m_materialSettings.m_metallicMap = search->second->Texture()->GetHandle();
        }
        else
        {
            search->second->Texture()->Bind(5);
            manager.m_materialSettings.m_metallicMap = 0;
        }
        manager.m_materialSettings.m_metallicEnabled = static_cast<int>(true);
    }
    search = material->m_textures.find(TextureType::AO);
    if (search != material->m_textures.end() && search->second)
    {
        if (supportBindlessTexture)
        {
            manager.m_materialSettings.m_aoMap = search->second->Texture()->GetHandle();
        }
        else
        {
            search->second->Texture()->Bind(7);
            manager.m_materialSettings.m_aoMap = 0;
        }
        manager.m_materialSettings.m_aoEnabled = static_cast<int>(true);
    }

    if (!hasAlbedo)
    {
        DefaultResources::Textures::MissingTexture->Texture()->Bind(3);
    }
    manager.m_materialSettings.m_alphaDiscardEnabled = material->m_alphaDiscardEnabled;
    manager.m_materialSettings.m_alphaDiscardOffset = material->m_alphaDiscardOffset;
    manager.m_materialSettings.m_albedoColorVal = glm::vec4(material->m_albedoColor, 1.0f);
    manager.m_materialSettings.m_metallicVal = material->m_metallic;
    manager.m_materialSettings.m_roughnessVal = material->m_roughness;
    manager.m_materialSettings.m_aoVal = material->m_ambient;
    manager.m_materialSettings.m_emissionVal = material->m_emission;
    if (supportBindlessTexture)
    {
        manager.m_materialSettings.m_directionalShadowMap =
            manager.m_directionalLightShadowMap->DepthMapArray()->GetHandle();
        manager.m_materialSettings.m_pointShadowMap = manager.m_pointLightShadowMap->DepthMapArray()->GetHandle();
        manager.m_materialSettings.m_spotShadowMap = manager.m_spotLightShadowMap->DepthMap()->GetHandle();
    }
    else
    {
        manager.m_materialSettings.m_directionalShadowMap = 0;
        manager.m_materialSettings.m_pointShadowMap = 0;
        manager.m_materialSettings.m_spotShadowMap = 0;
    }

    manager.m_materialSettingsBuffer->SubData(0, sizeof(MaterialSettingsBlock), &manager.m_materialSettings);
}

void RenderManager::ApplyProgramSettings(const std::shared_ptr<OpenGLUtils::GLProgram> &program)
{
    auto &manager = GetInstance();
    const bool supportBindlessTexture = OpenGLUtils::GetInstance().m_enableBindlessTexture;
    program->SetInt("UE_ALBEDO_MAP_LEGACY", 3);
    program->SetInt("UE_NORMAL_MAP_LEGACY", 3);
    program->SetInt("UE_METALLIC_MAP_LEGACY", 3);
    program->SetInt("UE_ROUGHNESS_MAP_LEGACY", 3);
    program->SetInt("UE_AO_MAP_LEGACY", 3);
    if (!supportBindlessTexture)
    {
        program->SetInt("UE_DIRECTIONAL_LIGHT_SM_LEGACY", 0);
        program->SetInt("UE_POINT_LIGHT_SM_LEGACY", 1);
        program->SetInt("UE_SPOT_LIGHT_SM_LEGACY", 2);

        program->SetInt("UE_ALBEDO_MAP_LEGACY", 3);
        program->SetInt("UE_NORMAL_MAP_LEGACY", 4);
        program->SetInt("UE_ROUGHNESS_MAP_LEGACY", 6);
        program->SetInt("UE_METALLIC_MAP_LEGACY", 5);
        program->SetInt("UE_AO_MAP_LEGACY", 7);

        program->SetInt("UE_ENVIRONMENTAL_MAP_LEGACY", 8);
        program->SetInt("UE_ENVIRONMENTAL_IRRADIANCE_LEGACY", 9);
        program->SetInt("UE_ENVIRONMENTAL_PREFILERED_LEGACY", 10);
        program->SetInt("UE_ENVIRONMENTAL_BRDFLUT_LEGACY", 11);
    }
}

void RenderManager::ReleaseMaterialSettings(const std::shared_ptr<Material> &material)
{
    if (!OpenGLUtils::GetInstance().m_enableBindlessTexture)
        return;
    for (const auto &i : material->m_textures)
    {
        if (!i.second || !i.second->Texture())
            continue;
        i.second->Texture()->MakeNonResident();
    }
}

void RenderManager::PrepareBrdfLut()
{
    auto &manager = GetInstance();
    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    auto brdfLut = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RG16F, 512, 512, true);
    manager.m_brdfLut = std::make_unique<Texture2D>();
    manager.m_brdfLut->m_texture = std::move(brdfLut);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    manager.m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    manager.m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    manager.m_brdfLut->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    manager.m_brdfLut->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    size_t resolution = 512;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
    renderTarget->AttachTexture(manager.m_brdfLut->m_texture.get(), GL_COLOR_ATTACHMENT0);
    renderTarget->GetFrameBuffer()->ViewPort(resolution, resolution);
    DefaultResources::GLPrograms::BrdfProgram->Bind();
    renderTarget->Clear();
    RenderQuad();
    OpenGLUtils::GLFrameBuffer::BindDefault();
}

void RenderManager::DeferredPrepassInternal(const std::shared_ptr<Mesh> &mesh)
{
    if (mesh == nullptr)
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderManager::DeferredPrepassInstancedInternal(
    const std::shared_ptr<Mesh> &mesh, const std::vector<glm::mat4> &matrices)
{
    if (mesh == nullptr || matrices.empty())
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderManager::DeferredPrepassInternal(const std::shared_ptr<SkinnedMesh> &skinnedMesh)
{
    if (skinnedMesh == nullptr)
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += skinnedMesh->GetTriangleAmount();
    skinnedMesh->Draw();
}

void RenderManager::DeferredPrepassInstancedInternal(
    const std::shared_ptr<SkinnedMesh> &skinnedMesh, const std::vector<glm::mat4> &matrices)
{
    if (skinnedMesh == nullptr || matrices.empty())
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += skinnedMesh->GetTriangleAmount() * matrices.size();
    auto &program = GetInstance().m_gBufferInstancedPrepass;
    skinnedMesh->DrawInstanced(matrices);
}

void RenderManager::DrawMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const std::vector<glm::mat4> &matrices,
    const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr || matrices.empty())
        return;

    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    auto &program = material->m_program;
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
    GetInstance().m_materialSettings = MaterialSettingsBlock();
    GetInstance().m_materialSettings.m_receiveShadow = receiveShadow;
    ApplyMaterialSettings(material);
    ApplyProgramSettings(program);
    mesh->DrawInstanced(matrices);
    ReleaseMaterialSettings(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderManager::DrawMeshInstancedInternal(const std::shared_ptr<Mesh> &mesh, const std::vector<glm::mat4> &matrices)
{
    if (mesh == nullptr || matrices.empty())
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderManager::DrawMeshInstancedInternal(
    const std::shared_ptr<SkinnedMesh> &mesh, const std::vector<glm::mat4> &matrices)
{
    if (mesh == nullptr || matrices.empty())
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderManager::DrawMesh(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr)
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    auto &program = material->m_program;
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
    GetInstance().m_materialSettings = MaterialSettingsBlock();
    GetInstance().m_materialSettings.m_receiveShadow = receiveShadow;
    MaterialPropertySetter(material);
    ApplyMaterialSettings(material);
    ApplyProgramSettings(program);
    mesh->Draw();
    ReleaseMaterialSettings(material);
}

void RenderManager::DrawMeshInternal(const std::shared_ptr<Mesh> &mesh)
{
    if (mesh == nullptr)
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderManager::DrawMeshInternal(const std::shared_ptr<SkinnedMesh> &mesh)
{
    if (mesh == nullptr)
        return;
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}

void RenderManager::DrawTexture2D(
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

void RenderManager::DrawGizmoMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices.empty())
        return;
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    DefaultResources::GLPrograms::GizmoInstancedProgram->Bind();
    DefaultResources::GLPrograms::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
    DefaultResources::GLPrograms::GizmoInstancedProgram->SetFloat4x4("model", model);
    DefaultResources::GLPrograms::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const std::shared_ptr<Mesh> &mesh,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices.empty() || colors.empty() || matrices.size() != colors.size())
        return;
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    const auto vao = mesh->Vao();
    const OpenGLUtils::GLVBO colorsBuffer;
    colorsBuffer.SetData(static_cast<GLsizei>(matrices.size()) * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
    vao->EnableAttributeArray(11);
    vao->SetAttributePointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
    vao->SetAttributeDivisor(11, 1);

    DefaultResources::GLPrograms::GizmoInstancedColoredProgram->Bind();
    DefaultResources::GLPrograms::GizmoInstancedColoredProgram->SetFloat4x4("model", model);
    DefaultResources::GLPrograms::GizmoInstancedColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * matrices.size();
    mesh->DrawInstanced(matrices);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderManager::DrawGizmoMesh(
    const std::shared_ptr<Mesh> &mesh, const glm::vec4 &color, const glm::mat4 &model, const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr)
        return;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    DefaultResources::GLPrograms::GizmoProgram->Bind();
    DefaultResources::GLPrograms::GizmoProgram->SetFloat4("surfaceColor", color);
    DefaultResources::GLPrograms::GizmoProgram->SetFloat4x4("model", model);
    DefaultResources::GLPrograms::GizmoProgram->SetFloat4x4("scaleMatrix", scaleMatrix);

    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    mesh->Draw();
}
#pragma endregion

#pragma region External
void RenderManager::DrawGizmoMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const glm::vec4 &color,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.IsEnabled())
        {
            Camera::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera,
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
            sceneCamera.Bind();
            DrawGizmoMeshInstanced(mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
        }
    }
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const std::shared_ptr<Mesh> &mesh,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.IsEnabled())
        {
            Camera::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera,
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
            sceneCamera.Bind();
            DrawGizmoMeshInstancedColored(mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
        }
    }
}

void RenderManager::DrawGizmoMesh(
    const std::shared_ptr<Mesh> &mesh,
    const Camera &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const float &size)
{
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent.Bind();
    DrawGizmoMesh(mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void RenderManager::DrawGizmoMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const Camera &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent.Bind();
    DrawGizmoMeshInstanced(mesh, color, model, matrices, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const std::shared_ptr<Mesh> &mesh,
    const Camera &cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const std::vector<glm::vec4> &colors,
    const std::vector<glm::mat4> &matrices,
    const glm::mat4 &model,
    const float &size)
{
    Camera::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent.Bind();
    DrawGizmoMeshInstancedColored(mesh, colors, matrices, model, glm::scale(glm::vec3(size)));
}

void RenderManager::DrawGizmoRay(
    const glm::vec4 &color, const glm::vec3 &start, const glm::vec3 &end, const float &width)
{
    glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                       glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
    DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model);
}

void RenderManager::DrawGizmoRays(
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

void RenderManager::DrawGizmoRays(const glm::vec4 &color, const std::vector<Ray> &rays, const float &width)
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

void RenderManager::DrawGizmoRay(const glm::vec4 &color, const Ray &ray, const float &width)
{
    glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                       glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
    DrawGizmoMesh(DefaultResources::Primitives::Cylinder, color, model);
}

void RenderManager::DrawGizmoRay(
    const Camera &cameraComponent,
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

void RenderManager::DrawGizmoRays(
    const Camera &cameraComponent,
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

void RenderManager::DrawGizmoRays(
    const Camera &cameraComponent,
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

void RenderManager::DrawGizmoRay(
    const Camera &cameraComponent,
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

void RenderManager::DrawMesh(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    Camera &cameraComponent,
    const bool &receiveShadow,
    const bool &castShadow)
{
    RenderCommand renderCommand;
    renderCommand.m_commandType = RenderCommandType::FromAPI;
    renderCommand.m_meshType = RenderCommandMeshType::Default;
    renderCommand.m_mesh = mesh;
    renderCommand.m_receiveShadow = receiveShadow;
    renderCommand.m_castShadow = castShadow;
    renderCommand.m_globalTransform.m_value = model;
    const auto owner = cameraComponent.GetOwner();
    float distance = 0.0f;
    if (owner.IsValid())
    {
        auto meshCenter = mesh->GetBound().Center();
        meshCenter = glm::vec3(model * glm::vec4(meshCenter, 1.0));
        const auto cameraTransform = owner.GetDataComponent<GlobalTransform>();
        distance = glm::distance(glm::vec3(meshCenter), cameraTransform.GetPosition());
    }
    GetInstance()
        .m_forwardRenderInstances[&cameraComponent][material]
        .m_meshes[distance][renderCommand.m_mesh->m_vao.get()]
        .push_back(renderCommand);
    GetInstance()
        .m_forwardRenderInstances[&EditorManager::GetSceneCamera()][material]
        .m_meshes[distance][renderCommand.m_mesh->m_vao.get()]
        .push_back(renderCommand);
}

void RenderManager::DrawMeshInstanced(
    const std::shared_ptr<Mesh> &mesh,
    const std::shared_ptr<Material> &material,
    const glm::mat4 &model,
    const std::shared_ptr<ParticleMatrices> &matrices,
    Camera &cameraComponent,
    const bool &receiveShadow,
    const bool &castShadow)
{
    RenderCommand renderCommand;
    renderCommand.m_commandType = RenderCommandType::FromAPI;
    renderCommand.m_meshType = RenderCommandMeshType::Default;
    renderCommand.m_mesh = mesh;
    renderCommand.m_matrices = matrices;
    renderCommand.m_receiveShadow = receiveShadow;
    renderCommand.m_castShadow = castShadow;
    renderCommand.m_globalTransform.m_value = model;
    const auto owner = cameraComponent.GetOwner();
    float distance = 0.0f;
    if (owner.IsValid())
    {
        auto meshCenter = mesh->GetBound().Center();
        meshCenter = glm::vec3(model * glm::vec4(meshCenter, 1.0));
        const auto cameraTransform = owner.GetDataComponent<GlobalTransform>();
        distance = glm::distance(glm::vec3(meshCenter), cameraTransform.GetPosition());
    }
    GetInstance()
        .m_forwardInstancedRenderInstances[&cameraComponent][material]
        .m_meshes[distance][renderCommand.m_mesh->m_vao.get()]
        .push_back(renderCommand);
    GetInstance()
        .m_forwardInstancedRenderInstances[&EditorManager::GetSceneCamera()][material]
        .m_meshes[distance][renderCommand.m_mesh->m_vao.get()]
        .push_back(renderCommand);
}

#pragma region DrawTexture
void RenderManager::DrawTexture2D(
    const OpenGLUtils::GLTexture2D *texture,
    const float &depth,
    const glm::vec2 &center,
    const glm::vec2 &size,
    const RenderTarget *target)
{
    target->Bind();
    DrawTexture2D(texture, depth, center, size);
}

void RenderManager::DrawTexture2D(
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
void RenderManager::SetMainCamera(Camera *value)
{
    if (GetInstance().m_mainCameraComponent)
    {
        GetInstance().m_mainCameraComponent->m_isMainCamera = false;
    }
    GetInstance().m_mainCameraComponent = value;
    if (GetInstance().m_mainCameraComponent)
        GetInstance().m_mainCameraComponent->m_isMainCamera = true;
}

Camera *RenderManager::GetMainCamera()
{
    return GetInstance().m_mainCameraComponent;
}

void RenderManager::DrawTexture2D(
    const Texture2D *texture,
    const float &depth,
    const glm::vec2 &center,
    const glm::vec2 &size,
    const RenderTarget *target)
{
    target->Bind();
    DrawTexture2D(texture->Texture().get(), depth, center, size);
}
#pragma endregion
#pragma region Gizmo

void RenderManager::DrawGizmoMesh(
    const std::shared_ptr<Mesh> &mesh, const glm::vec4 &color, const glm::mat4 &model, const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.IsEnabled())
        {
            Camera::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera,
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            Camera::m_cameraInfoBlock.UploadMatrices(sceneCamera);
            sceneCamera.Bind();
            DrawGizmoMesh(mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
        }
    }
}
#pragma endregion
#pragma endregion

#pragma region Status

size_t RenderManager::Triangles()
{
    return GetInstance().m_triangles;
}

size_t RenderManager::DrawCall()
{
    return GetInstance().m_drawCall;
}

#pragma endregion
#pragma endregion