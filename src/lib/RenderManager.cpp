#include <Application.hpp>
#include <CameraComponent.hpp>
#include <Cubemap.hpp>
#include <EditorManager.hpp>
#include <FileIO.hpp>
#include <InputManager.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <PostProcessing.hpp>
#include <RenderManager.hpp>
using namespace UniEngine;

void RenderManager::RenderToCameraDeferred(
    const std::unique_ptr<CameraComponent> &cameraComponent,
    const GlobalTransform &cameraTransform,
    glm::vec3 &minBound,
    glm::vec3 &maxBound,
    bool calculateBounds)
{
    cameraComponent->m_gBuffer->Bind();
    unsigned int attachments[4] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    cameraComponent->m_gBuffer->GetFrameBuffer()->DrawBuffers(4, attachments);
    cameraComponent->m_gBuffer->Clear();
    const std::vector<Entity> *owners = EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
    if (owners)
    {
        auto &program = GetInstance().m_gBufferPrepass;
        program->Bind();
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
            if (!mmc->IsEnabled() || mmc->m_material == nullptr || mmc->m_mesh == nullptr || mmc->m_forwardRendering)
                continue;
            if (mmc->m_material->m_blendingMode != MaterialBlendingMode::Off)
                continue;
            if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                  static_cast<size_t>(CameraLayer::MainCamera)))
                continue;
            auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
            if (calculateBounds)
            {
                auto meshBound = mmc->m_mesh->GetBound();
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
            }
            GetInstance().m_materialSettings.m_receiveShadow = mmc->m_receiveShadow;
            DeferredPrepass(mmc->m_mesh.get(), mmc->m_material.get(), ltw);
        }
    }

    owners = EntityManager::GetPrivateComponentOwnersList<Particles>();
    if (owners)
    {
        auto &program = GetInstance().m_gBufferInstancedPrepass;
        program->Bind();
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &particles = owner.GetPrivateComponent<Particles>();
            if (!particles->IsEnabled() || particles->m_material == nullptr || particles->m_mesh == nullptr ||
                particles->m_forwardRendering)
                continue;
            if (particles->m_material->m_blendingMode != MaterialBlendingMode::Off)
                continue;
            if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                  static_cast<size_t>(CameraLayer::MainCamera)))
                continue;
            auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
            if (calculateBounds)
            {
                auto meshBound = particles->m_mesh->GetBound();
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
            }
            GetInstance().m_materialSettings.m_receiveShadow = particles->m_receiveShadow;
            DeferredPrepassInstanced(
                particles->m_mesh.get(),
                particles->m_material.get(),
                ltw,
                particles->m_matrices.data(),
                particles->m_matrices.size());
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    Default::GLPrograms::ScreenVAO->Bind();

    cameraComponent->Bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GetInstance().m_gBufferLightingPass->Bind();

    cameraComponent->m_gPositionBuffer->Bind(3);
    cameraComponent->m_gNormalBuffer->Bind(4);
    cameraComponent->m_gColorSpecularBuffer->Bind(5);
    cameraComponent->m_gMetallicRoughnessAo->Bind(6);
    GetInstance().m_gBufferLightingPass->SetInt("gPositionShadow", 3);
    GetInstance().m_gBufferLightingPass->SetInt("gNormalShininess", 4);
    GetInstance().m_gBufferLightingPass->SetInt("gAlbedoSpecular", 5);
    GetInstance().m_gBufferLightingPass->SetInt("gMetallicRoughnessAO", 6);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    auto res = cameraComponent->GetResolution();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, cameraComponent->m_gBuffer->GetFrameBuffer()->Id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cameraComponent->GetFrameBuffer()->Id()); // write to default framebuffer
    glBlitFramebuffer(0, 0, res.x, res.y, 0, 0, res.x, res.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    RenderTarget::BindDefault();
}

void RenderManager::RenderBackGround(const std::unique_ptr<CameraComponent> &cameraComponent)
{
    cameraComponent->Bind();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(
        GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
    if (cameraComponent->m_drawSkyBox && cameraComponent->m_skyBox.get())
    {
        Default::GLPrograms::SkyboxProgram->Bind();
        Default::GLPrograms::SkyboxVAO->Bind();
        cameraComponent->m_skyBox->Texture()->Bind(3);
        Default::GLPrograms::SkyboxProgram->SetInt("skybox", 3);
    }
    else
    {
        Default::GLPrograms::BackGroundProgram->Bind();
        Default::GLPrograms::SkyboxVAO->Bind();
        Default::GLPrograms::BackGroundProgram->SetFloat3("clearColor", cameraComponent->m_clearColor);
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);
    OpenGLUtils::GLVAO::BindDefault();
    glDepthFunc(GL_LESS); // set depth function back to default
}

void RenderManager::RenderToCameraForward(
    const std::unique_ptr<CameraComponent> &cameraComponent,
    const GlobalTransform &cameraTransform,
    glm::vec3 &minBound,
    glm::vec3 &maxBound,
    bool calculateBounds)
{
    bool debug = cameraComponent.get() == EditorManager::GetInstance().m_sceneCamera.get();
    cameraComponent->Bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    std::map<float, std::pair<std::pair<Entity, MeshRenderer *>, glm::mat4>> transparentEntities;
    std::map<float, std::pair<std::pair<Entity, Particles *>, glm::mat4>> transparentInstancedEntities;
    const std::vector<Entity> *owners = EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
            if (!mmc->IsEnabled() || mmc->m_material == nullptr || mmc->m_mesh == nullptr ||
                !mmc->m_forwardRendering && mmc->m_material->m_blendingMode == MaterialBlendingMode::Off)
                continue;
            if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                  static_cast<size_t>(CameraLayer::MainCamera)))
                continue;
            auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
            auto meshBound = mmc->m_mesh->GetBound();
            meshBound.ApplyTransform(ltw);
            glm::vec3 center = meshBound.Center();
            glm::vec3 size = meshBound.Size();
            if (calculateBounds)
            {

                minBound = glm::vec3(
                    (glm::min)(minBound.x, center.x - size.x),
                    (glm::min)(minBound.y, center.y - size.y),
                    (glm::min)(minBound.z, center.z - size.z));

                maxBound = glm::vec3(
                    (glm::max)(maxBound.x, center.x + size.x),
                    (glm::max)(maxBound.y, center.y + size.y),
                    (glm::max)(maxBound.z, center.z + size.z));
            }
            if (mmc->m_material->m_blendingMode == MaterialBlendingMode::Off)
            {
                transparentEntities.insert(
                    {glm::distance(cameraTransform.GetPosition(), center),
                     std::make_pair(std::make_pair(owner, mmc.get()), ltw)});
                continue;
            }
            DrawMesh(mmc->m_mesh.get(), mmc->m_material.get(), ltw, mmc->m_receiveShadow);
        }
    }
    owners = EntityManager::GetPrivateComponentOwnersList<Particles>();
    if (owners)
    {
        for (auto owner : *owners)
        {
            if (!owner.IsEnabled())
                continue;
            auto &immc = owner.GetPrivateComponent<Particles>();
            if (!immc->IsEnabled() || immc->m_material == nullptr || immc->m_mesh == nullptr ||
                !immc->m_forwardRendering && immc->m_material->m_blendingMode == MaterialBlendingMode::Off)
                continue;
            if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                  static_cast<size_t>(CameraLayer::MainCamera)))
                continue;
            auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
            if (calculateBounds)
            {
                auto meshBound = immc->m_mesh->GetBound();
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
            }
            if (immc->m_material->m_blendingMode == MaterialBlendingMode::Off)
            {
                transparentInstancedEntities.insert(
                    {glm::distance(cameraTransform.GetPosition(), glm::vec3(ltw[3])),
                     std::make_pair(std::make_pair(owner, immc.get()), ltw)});
                continue;
            }
            DrawMeshInstanced(
                immc->m_mesh.get(),
                immc->m_material.get(),
                ltw,
                immc->m_matrices.data(),
                immc->m_matrices.size(),
                immc->m_receiveShadow);
        }
    }

    // Draw all transparent objects here:
    for (auto pair = transparentEntities.rbegin(); pair != transparentEntities.rend(); ++pair)
    {
        const auto *mmc = pair->second.first.second;
        DrawMesh(mmc->m_mesh.get(), mmc->m_material.get(), pair->second.second, mmc->m_receiveShadow);
    }
    for (auto pair = transparentInstancedEntities.rbegin(); pair != transparentInstancedEntities.rend(); ++pair)
    {
        const auto *immc = pair->second.first.second;
        glm::mat4 ltw = pair->second.second;
        Mesh *mesh = immc->m_mesh.get();
        Material *material = immc->m_material.get();
        const glm::mat4 *matrices = immc->m_matrices.data();
        DrawMeshInstanced(mesh, material, ltw, matrices, immc->m_matrices.size(), immc->m_receiveShadow);
    }
}

void RenderManager::Init()
{
    GetInstance().m_materialSettingsBuffer = std::make_unique<OpenGLUtils::GLUBO>();
    GetInstance().m_materialSettingsBuffer->SetData(sizeof(MaterialSettingsBlock), nullptr, GL_STREAM_DRAW);
    GetInstance().m_materialSettingsBuffer->SetBase(6);
#pragma region Kernel Setup
    std::vector<glm::vec4> uniformKernel;
    std::vector<glm::vec4> gaussianKernel;
    for (unsigned int i = 0; i < Default::ShaderIncludes::MaxKernelAmount; i++)
    {
        uniformKernel.emplace_back(glm::vec4(glm::ballRand(1.0f), 1.0f));
        gaussianKernel.emplace_back(
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f),
            glm::gaussRand(0.0f, 1.0f));
    }
    GetInstance().m_kernelBlock = std::make_unique<OpenGLUtils::GLUBO>();
    GetInstance().m_kernelBlock->SetBase(5);
    GetInstance().m_kernelBlock->SetData(
        sizeof(glm::vec4) * uniformKernel.size() + sizeof(glm::vec4) * gaussianKernel.size(), NULL, GL_STATIC_DRAW);
    GetInstance().m_kernelBlock->SubData(0, sizeof(glm::vec4) * uniformKernel.size(), uniformKernel.data());
    GetInstance().m_kernelBlock->SubData(
        sizeof(glm::vec4) * uniformKernel.size(), sizeof(glm::vec4) * gaussianKernel.size(), gaussianKernel.data());

#pragma endregion
#pragma region Shadow
    GetInstance().m_shadowCascadeInfoBlock.SetData(sizeof(LightSettingsBlock), nullptr, GL_DYNAMIC_DRAW);
    GetInstance().m_shadowCascadeInfoBlock.SetBase(4);

#pragma region LightInfoBlocks
    size_t size = 16 + Default::ShaderIncludes::MaxDirectionalLightAmount * sizeof(DirectionalLightInfo);
    GetInstance().m_directionalLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    GetInstance().m_directionalLightBlock.SetBase(1);
    size = 16 + Default::ShaderIncludes::MaxPointLightAmount * sizeof(PointLightInfo);
    GetInstance().m_pointLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    GetInstance().m_pointLightBlock.SetBase(2);
    size = 16 + Default::ShaderIncludes::MaxSpotLightAmount * sizeof(SpotLightInfo);
    GetInstance().m_spotLightBlock.SetData((GLsizei)size, nullptr, (GLsizei)GL_DYNAMIC_DRAW);
    GetInstance().m_spotLightBlock.SetBase(3);
#pragma endregion
#pragma region DirectionalLight
    GetInstance().m_directionalLightShadowMap =
        std::make_unique<DirectionalLightShadowMap>(GetInstance().m_shadowMapResolution);

    std::string vertShaderCode =
        std::string("#version 460 core\n") +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/DirectionalLightShadowMap.vert"));
    std::string fragShaderCode =
        std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/DirectionalLightShadowMap.frag"));
    std::string geomShaderCode =
        std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Geometry/DirectionalLightShadowMap.geom"));

    auto vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    auto fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    auto geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);

    GetInstance().m_directionalLightProgram =
        std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 460 core\n") +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/DirectionalLightShadowMapInstanced.vert"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);

    GetInstance().m_directionalLightInstancedProgram =
        std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader, geomShader);

#pragma region PointLight
    GetInstance().m_pointLightShadowMap = std::make_unique<PointLightShadowMap>(GetInstance().m_shadowMapResolution);
    vertShaderCode = std::string("#version 460 core\n") +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/PointLightShadowMap.vert"));
    fragShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/PointLightShadowMap.frag"));
    geomShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Geometry/PointLightShadowMap.geom"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);
    geomShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Geometry);
    geomShader->Compile(geomShaderCode);

    GetInstance().m_pointLightProgram = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader, geomShader);

    vertShaderCode =
        std::string("#version 460 core\n") +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/PointLightShadowMapInstanced.vert"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);

    GetInstance().m_pointLightInstancedProgram =
        std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader, geomShader);
#pragma endregion
#pragma region SpotLight
    GetInstance().m_spotLightShadowMap = std::make_unique<SpotLightShadowMap>(GetInstance().m_shadowMapResolution);
    vertShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/SpotLightShadowMap.vert"));
    fragShaderCode = std::string("#version 460 core\n") +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/SpotLightShadowMap.frag"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    GetInstance().m_spotLightProgram = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader);

    vertShaderCode =
        std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/SpotLightShadowMapInstanced.vert"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);

    GetInstance().m_spotLightInstancedProgram = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader);
#pragma endregion
#pragma endregion

#pragma region GBuffer
    vertShaderCode = std::string("#version 460 core\n") +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"));
    fragShaderCode =
        std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
        FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/StandardDeferredLighting.frag"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    GetInstance().m_gBufferLightingPass = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader);

    vertShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/Standard.vert"));

    fragShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/StandardDeferred.frag"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

    GetInstance().m_gBufferPrepass = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader);

    vertShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + +"\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/StandardInstanced.vert"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);

    GetInstance().m_gBufferInstancedPrepass = std::make_unique<OpenGLUtils::GLProgram>(vertShader, fragShader);

#pragma endregion
#pragma region SSAO
    vertShaderCode = std::string("#version 460 core\n") +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"));
    fragShaderCode = std::string("#version 460 core\n") + *Default::ShaderIncludes::Uniform + "\n" +
                     FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/SSAOGeometry.frag"));

    vertShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Vertex);
    vertShader->Compile(vertShaderCode);
    fragShader = std::make_shared<OpenGLUtils::GLShader>(OpenGLUtils::ShaderType::Fragment);
    fragShader->Compile(fragShaderCode);

#pragma endregion
}

void UniEngine::RenderManager::PreUpdate()
{
    GetInstance().m_triangles = 0;
    GetInstance().m_drawCall = 0;
    if (GetInstance().m_mainCameraComponent != nullptr)
    {
        if (GetInstance().m_mainCameraComponent->m_allowAutoResize)
            GetInstance().m_mainCameraComponent->ResizeResolution(
                GetInstance().m_mainCameraResolutionX, GetInstance().m_mainCameraResolutionY);
    }
    const std::vector<Entity> *cameraEntities = EntityManager::GetPrivateComponentOwnersList<CameraComponent>();
    if (cameraEntities != nullptr)
    {
        for (auto cameraEntity : *cameraEntities)
        {
            if (!cameraEntity.IsEnabled())
                continue;
            auto &cameraComponent = cameraEntity.GetPrivateComponent<CameraComponent>();
            if (cameraComponent->IsEnabled())
                cameraComponent->Clear();
        }
    }
    auto worldBound = Application::GetCurrentWorld()->GetBound();
    glm::vec3 maxBound = worldBound.m_max;
    glm::vec3 minBound = worldBound.m_min;
    if (GetInstance().m_mainCameraComponent != nullptr)
    {
        auto mainCameraEntity = GetInstance().m_mainCameraComponent->GetOwner();
        if (mainCameraEntity.IsEnabled())
        {
            auto &mainCamera = GetInstance().m_mainCameraComponent;
#pragma region Shadow
            if (GetInstance().m_mainCameraComponent->IsEnabled())
            {
                auto ltw = mainCameraEntity.GetComponentData<GlobalTransform>();
                glm::vec3 mainCameraPos = ltw.GetPosition();
                glm::quat mainCameraRot = ltw.GetRotation();
                GetInstance().m_shadowCascadeInfoBlock.SubData(
                    0, sizeof(LightSettingsBlock), &GetInstance().m_lightSettings);
                const std::vector<Entity> *directionalLightEntities =
                    EntityManager::GetPrivateComponentOwnersList<DirectionalLight>();
                size_t size = 0;
                if (directionalLightEntities && !directionalLightEntities->empty())
                {
                    size = directionalLightEntities->size();
                    size_t enabledSize = 0;
                    for (int i = 0; i < size; i++)
                    {
                        Entity lightEntity = directionalLightEntities->at(i);
                        if (!lightEntity.IsEnabled())
                            continue;
                        const auto &dlc = lightEntity.GetPrivateComponent<DirectionalLight>();
                        if (!dlc->IsEnabled())
                            continue;
                        glm::quat rotation = lightEntity.GetComponentData<GlobalTransform>().GetRotation();
                        glm::vec3 lightDir = glm::normalize(rotation * glm::vec3(0, 0, 1));
                        float planeDistance = 0;
                        glm::vec3 center;
                        GetInstance().m_directionalLights[enabledSize].m_direction = glm::vec4(lightDir, 0.0f);
                        GetInstance().m_directionalLights[enabledSize].m_diffuse =
                            glm::vec4(dlc->m_diffuse * dlc->m_diffuseBrightness, dlc->m_castShadow);
                        GetInstance().m_directionalLights[enabledSize].m_specular = glm::vec4(0.0f);
                        for (int split = 0; split < Default::ShaderIncludes::ShadowCascadeAmount; split++)
                        {
                            float splitStart = 0;
                            float splitEnd = GetInstance().m_maxShadowDistance;
                            if (split != 0)
                                splitStart =
                                    GetInstance().m_maxShadowDistance * GetInstance().m_shadowCascadeSplit[split - 1];
                            if (split != Default::ShaderIncludes::ShadowCascadeAmount - 1)
                                splitEnd =
                                    GetInstance().m_maxShadowDistance * GetInstance().m_shadowCascadeSplit[split];
                            GetInstance().m_lightSettings.m_splitDistance[split] = splitEnd;
                            glm::mat4 lightProjection, lightView;
                            float max = 0;
                            glm::vec3 lightPos;
                            glm::vec3 cornerPoints[8];
                            mainCamera->CalculateFrustumPoints(
                                splitStart, splitEnd, mainCameraPos, mainCameraRot, cornerPoints);
                            glm::vec3 cameraFrustumCenter =
                                (mainCameraRot * glm::vec3(0, 0, -1)) * ((splitEnd - splitStart) / 2.0f + splitStart) +
                                mainCameraPos;
                            if (GetInstance().m_stableFit)
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

                            center = ClosestPointOnLine(
                                worldBound.Center(), cameraFrustumCenter, cameraFrustumCenter + lightDir);
                            planeDistance = (glm::max)((glm::max)(d0, d1), (glm::max)(d2, d3));
                            lightPos = center - lightDir * planeDistance;
                            lightView = glm::lookAt(
                                lightPos, lightPos + lightDir, glm::normalize(rotation * glm::vec3(0, 1, 0)));
                            lightProjection = glm::ortho(-max, max, -max, max, 0.0f, planeDistance * 2.0f);
                            switch (enabledSize)
                            {
                            case 0:
                                GetInstance().m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                                    0,
                                    0,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2);
                                break;
                            case 1:
                                GetInstance().m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                                    GetInstance().m_shadowMapResolution / 2,
                                    0,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2);
                                break;
                            case 2:
                                GetInstance().m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                                    0,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2);
                                break;
                            case 3:
                                GetInstance().m_directionalLights[enabledSize].m_viewPort = glm::ivec4(
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2,
                                    GetInstance().m_shadowMapResolution / 2);
                                break;
                            }

#pragma region Fix Shimmering due to the movement of the camera

                            glm::mat4 shadowMatrix = lightProjection * lightView;
                            glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                            shadowOrigin = shadowMatrix * shadowOrigin;
                            GLfloat storedW = shadowOrigin.w;
                            shadowOrigin = shadowOrigin *
                                           (float)GetInstance().m_directionalLights[enabledSize].m_viewPort.z / 2.0f;
                            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
                            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
                            roundOffset =
                                roundOffset * 2.0f / (float)GetInstance().m_directionalLights[enabledSize].m_viewPort.z;
                            roundOffset.z = 0.0f;
                            roundOffset.w = 0.0f;
                            glm::mat4 shadowProj = lightProjection;
                            shadowProj[3] += roundOffset;
                            lightProjection = shadowProj;
#pragma endregion
                            GetInstance().m_directionalLights[enabledSize].m_lightSpaceMatrix[split] =
                                lightProjection * lightView;
                            GetInstance().m_directionalLights[enabledSize].m_lightFrustumWidth[split] = max;
                            GetInstance().m_directionalLights[enabledSize].m_lightFrustumDistance[split] =
                                planeDistance;
                            if (split == Default::ShaderIncludes::ShadowCascadeAmount - 1)
                                GetInstance().m_directionalLights[enabledSize].m_reservedParameters =
                                    glm::vec4(dlc->m_lightSize, 0, dlc->m_bias, dlc->m_normalOffset);
                        }
                        enabledSize++;
                    }
                    GetInstance().m_directionalLightBlock.SubData(0, 4, &enabledSize);
                    if (enabledSize != 0)
                    {
                        GetInstance().m_directionalLightBlock.SubData(
                            16, enabledSize * sizeof(DirectionalLightInfo), &GetInstance().m_directionalLights[0]);
                    }
                    if (GetInstance().m_materialSettings.m_enableShadow)
                    {
                        GetInstance().m_directionalLightShadowMap->Bind();
                        GetInstance().m_directionalLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                        glClear(GL_DEPTH_BUFFER_BIT);
                        enabledSize = 0;
                        GetInstance().m_directionalLightProgram->Bind();
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = directionalLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            /*
                            glClearTexSubImage(_DirectionalLightShadowMap->DepthMapArray()->ID(),
                                0, _DirectionalLights[enabledSize].viewPort.x,
                            _DirectionalLights[enabledSize].viewPort.y, 0,
                            (GLsizei)_DirectionalLights[enabledSize].viewPort.z,
                            (GLsizei)_DirectionalLights[enabledSize].viewPort.w, (GLsizei)4, GL_DEPTH_COMPONENT,
                            GL_FLOAT, nullptr);
                            */
                            glViewport(
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.x,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.y,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.z,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.w);
                            GetInstance().m_directionalLightProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
                                    if (!mmc->IsEnabled() || !mmc->m_castShadow || mmc->m_material == nullptr ||
                                        mmc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(mmc.get()->m_material.get(), true);
                                    auto mesh = mmc->m_mesh;
                                    auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
                                    GetInstance().m_directionalLightProgram->SetFloat4x4("model", ltw);
                                    mesh->Enable();
                                    mesh->Vao()->DisableAttributeArray(12);
                                    mesh->Vao()->DisableAttributeArray(13);
                                    mesh->Vao()->DisableAttributeArray(14);
                                    mesh->Vao()->DisableAttributeArray(15);
                                    glDrawElements(
                                        GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
                                }
                            }
                            enabledSize++;
                        }
                        enabledSize = 0;
                        GetInstance().m_directionalLightInstancedProgram->Bind();
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = directionalLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            glViewport(
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.x,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.y,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.z,
                                GetInstance().m_directionalLights[enabledSize].m_viewPort.w);
                            GetInstance().m_directionalLightInstancedProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<Particles>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &immc = owner.GetPrivateComponent<Particles>();
                                    if (!immc->IsEnabled() || !immc->m_castShadow || immc->m_material == nullptr ||
                                        immc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(immc.get()->m_material.get(), true);
                                    size_t count = immc->m_matrices.size();
                                    std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer =
                                        std::make_unique<OpenGLUtils::GLVBO>();
                                    matricesBuffer->SetData(
                                        (GLsizei)count * sizeof(glm::mat4), immc->m_matrices.data(), GL_STATIC_DRAW);
                                    auto mesh = immc->m_mesh;
                                    GetInstance().m_directionalLightInstancedProgram->SetFloat4x4(
                                        "model", EntityManager::GetComponentData<GlobalTransform>(owner).m_value);
                                    mesh->Enable();
                                    mesh->Vao()->EnableAttributeArray(12);
                                    mesh->Vao()->SetAttributePointer(
                                        12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
                                    mesh->Vao()->EnableAttributeArray(13);
                                    mesh->Vao()->SetAttributePointer(
                                        13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(14);
                                    mesh->Vao()->SetAttributePointer(
                                        14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(15);
                                    mesh->Vao()->SetAttributePointer(
                                        15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
                                    mesh->Vao()->SetAttributeDivisor(12, 1);
                                    mesh->Vao()->SetAttributeDivisor(13, 1);
                                    mesh->Vao()->SetAttributeDivisor(14, 1);
                                    mesh->Vao()->SetAttributeDivisor(15, 1);
                                    glDrawElementsInstanced(
                                        GL_TRIANGLES,
                                        (GLsizei)mesh->GetTriangleAmount() * 3,
                                        GL_UNSIGNED_INT,
                                        0,
                                        (GLsizei)count);
                                    OpenGLUtils::GLVAO::BindDefault();
                                }
                            }
                            enabledSize++;
                        }
                    }
                }
                else
                {
                    GetInstance().m_directionalLightBlock.SubData(0, 4, &size);
                }
                const std::vector<Entity> *pointLightEntities =
                    EntityManager::GetPrivateComponentOwnersList<PointLight>();
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
                        if (!plc->IsEnabled())
                            continue;
                        glm::vec3 position = EntityManager::GetComponentData<GlobalTransform>(lightEntity).m_value[3];
                        GetInstance().m_pointLights[enabledSize].m_position = glm::vec4(position, 0);
                        GetInstance().m_pointLights[enabledSize].m_constantLinearQuadFarPlane.x = plc->m_constant;
                        GetInstance().m_pointLights[enabledSize].m_constantLinearQuadFarPlane.y = plc->m_linear;
                        GetInstance().m_pointLights[enabledSize].m_constantLinearQuadFarPlane.z = plc->m_quadratic;
                        GetInstance().m_pointLights[enabledSize].m_diffuse =
                            glm::vec4(plc->m_diffuse * plc->m_diffuseBrightness, plc->m_castShadow);
                        GetInstance().m_pointLights[enabledSize].m_specular = glm::vec4(0);
                        GetInstance().m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w = plc->m_farPlane;

                        glm::mat4 shadowProj = glm::perspective(
                            glm::radians(90.0f),
                            GetInstance().m_pointLightShadowMap->GetResolutionRatio(),
                            1.0f,
                            GetInstance().m_pointLights[enabledSize].m_constantLinearQuadFarPlane.w);
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[0] =
                            shadowProj *
                            glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[1] =
                            shadowProj *
                            glm::lookAt(
                                position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[2] =
                            shadowProj *
                            glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[3] =
                            shadowProj *
                            glm::lookAt(
                                position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[4] =
                            shadowProj *
                            glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                        GetInstance().m_pointLights[enabledSize].m_lightSpaceMatrix[5] =
                            shadowProj *
                            glm::lookAt(
                                position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                        GetInstance().m_pointLights[enabledSize].m_reservedParameters =
                            glm::vec4(plc->m_bias, plc->m_lightSize, 0, 0);

                        switch (enabledSize)
                        {
                        case 0:
                            GetInstance().m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                                0, 0, GetInstance().m_shadowMapResolution / 2, GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 1:
                            GetInstance().m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                                GetInstance().m_shadowMapResolution / 2,
                                0,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 2:
                            GetInstance().m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                                0,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 3:
                            GetInstance().m_pointLights[enabledSize].m_viewPort = glm::ivec4(
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        }
                        enabledSize++;
                    }
                    GetInstance().m_pointLightBlock.SubData(0, 4, &enabledSize);
                    if (enabledSize != 0)
                        GetInstance().m_pointLightBlock.SubData(
                            16, enabledSize * sizeof(PointLightInfo), &GetInstance().m_pointLights[0]);
                    if (GetInstance().m_materialSettings.m_enableShadow)
                    {
#pragma region PointLight Shadowmap Pass
                        GetInstance().m_pointLightShadowMap->Bind();
                        GetInstance().m_pointLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                        glClear(GL_DEPTH_BUFFER_BIT);
                        GetInstance().m_pointLightProgram->Bind();
                        enabledSize = 0;
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = pointLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            glViewport(
                                GetInstance().m_pointLights[enabledSize].m_viewPort.x,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.y,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.z,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.w);
                            GetInstance().m_pointLightProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
                                    if (!mmc->IsEnabled() || !mmc->m_castShadow || mmc->m_material == nullptr ||
                                        mmc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(mmc.get()->m_material.get(), true);
                                    auto mesh = mmc->m_mesh;
                                    GetInstance().m_pointLightProgram->SetFloat4x4(
                                        "model", EntityManager::GetComponentData<GlobalTransform>(owner).m_value);
                                    mesh->Enable();
                                    mesh->Vao()->DisableAttributeArray(12);
                                    mesh->Vao()->DisableAttributeArray(13);
                                    mesh->Vao()->DisableAttributeArray(14);
                                    mesh->Vao()->DisableAttributeArray(15);
                                    glDrawElements(
                                        GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
                                }
                            }
                            enabledSize++;
                        }
                        enabledSize = 0;
                        GetInstance().m_pointLightInstancedProgram->Bind();
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = pointLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            glViewport(
                                GetInstance().m_pointLights[enabledSize].m_viewPort.x,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.y,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.z,
                                GetInstance().m_pointLights[enabledSize].m_viewPort.w);
                            GetInstance().m_pointLightInstancedProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<Particles>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &immc = owner.GetPrivateComponent<Particles>();
                                    if (!immc->IsEnabled() || !immc->m_castShadow || immc->m_material == nullptr ||
                                        immc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(immc.get()->m_material.get(), true);
                                    size_t count = immc->m_matrices.size();
                                    std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer =
                                        std::make_unique<OpenGLUtils::GLVBO>();
                                    matricesBuffer->SetData(
                                        (GLsizei)count * sizeof(glm::mat4), immc->m_matrices.data(), GL_STATIC_DRAW);
                                    auto mesh = immc->m_mesh;
                                    GetInstance().m_pointLightInstancedProgram->SetFloat4x4(
                                        "model", EntityManager::GetComponentData<GlobalTransform>(owner).m_value);
                                    mesh->Enable();
                                    mesh->Vao()->EnableAttributeArray(12);
                                    mesh->Vao()->SetAttributePointer(
                                        12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
                                    mesh->Vao()->EnableAttributeArray(13);
                                    mesh->Vao()->SetAttributePointer(
                                        13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(14);
                                    mesh->Vao()->SetAttributePointer(
                                        14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(15);
                                    mesh->Vao()->SetAttributePointer(
                                        15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
                                    mesh->Vao()->SetAttributeDivisor(12, 1);
                                    mesh->Vao()->SetAttributeDivisor(13, 1);
                                    mesh->Vao()->SetAttributeDivisor(14, 1);
                                    mesh->Vao()->SetAttributeDivisor(15, 1);
                                    glDrawElementsInstanced(
                                        GL_TRIANGLES,
                                        (GLsizei)mesh->GetTriangleAmount() * 3,
                                        GL_UNSIGNED_INT,
                                        0,
                                        (GLsizei)count);
                                    OpenGLUtils::GLVAO::BindDefault();
                                }
                            }
                            enabledSize++;
                        }
#pragma endregion
                    }
                }
                else
                {
                    GetInstance().m_pointLightBlock.SubData(0, 4, &size);
                }
                const std::vector<Entity> *spotLightEntities =
                    EntityManager::GetPrivateComponentOwnersList<SpotLight>();
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
                        if (!slc->IsEnabled())
                            continue;
                        auto ltw = EntityManager::GetComponentData<GlobalTransform>(lightEntity);
                        glm::vec3 position = ltw.m_value[3];
                        glm::vec3 front = ltw.GetRotation() * glm::vec3(0, 0, -1);
                        glm::vec3 up = ltw.GetRotation() * glm::vec3(0, 1, 0);
                        GetInstance().m_spotLights[enabledSize].m_position = glm::vec4(position, 0);
                        GetInstance().m_spotLights[enabledSize].m_direction = glm::vec4(front, 0);
                        GetInstance().m_spotLights[enabledSize].m_constantLinearQuadFarPlane.x = slc->m_constant;
                        GetInstance().m_spotLights[enabledSize].m_constantLinearQuadFarPlane.y = slc->m_linear;
                        GetInstance().m_spotLights[enabledSize].m_constantLinearQuadFarPlane.z = slc->m_quadratic;
                        GetInstance().m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w = slc->m_farPlane;
                        GetInstance().m_spotLights[enabledSize].m_diffuse =
                            glm::vec4(slc->m_diffuse * slc->m_diffuseBrightness, slc->m_castShadow);
                        GetInstance().m_spotLights[enabledSize].m_specular = glm::vec4(0);

                        glm::mat4 shadowProj = glm::perspective(
                            glm::radians(slc->m_outerDegrees * 2.0f),
                            GetInstance().m_spotLightShadowMap->GetResolutionRatio(),
                            1.0f,
                            GetInstance().m_spotLights[enabledSize].m_constantLinearQuadFarPlane.w);
                        GetInstance().m_spotLights[enabledSize].m_lightSpaceMatrix =
                            shadowProj * glm::lookAt(position, position + front, up);
                        GetInstance().m_spotLights[enabledSize].m_cutOffOuterCutOffLightSizeBias = glm::vec4(
                            glm::cos(glm::radians(slc->m_innerDegrees)),
                            glm::cos(glm::radians(slc->m_outerDegrees)),
                            slc->m_lightSize,
                            slc->m_bias);

                        switch (enabledSize)
                        {
                        case 0:
                            GetInstance().m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                                0, 0, GetInstance().m_shadowMapResolution / 2, GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 1:
                            GetInstance().m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                                GetInstance().m_shadowMapResolution / 2,
                                0,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 2:
                            GetInstance().m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                                0,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        case 3:
                            GetInstance().m_spotLights[enabledSize].m_viewPort = glm::ivec4(
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2,
                                GetInstance().m_shadowMapResolution / 2);
                            break;
                        }
                        enabledSize++;
                    }
                    GetInstance().m_spotLightBlock.SubData(0, 4, &enabledSize);
                    if (enabledSize != 0)
                        GetInstance().m_spotLightBlock.SubData(
                            16, enabledSize * sizeof(SpotLightInfo), &GetInstance().m_spotLights[0]);
                    if (GetInstance().m_materialSettings.m_enableShadow)
                    {
#pragma region SpotLight Shadowmap Pass
                        GetInstance().m_spotLightShadowMap->Bind();
                        GetInstance().m_spotLightShadowMap->GetFrameBuffer()->DrawBuffer(GL_NONE);
                        glClear(GL_DEPTH_BUFFER_BIT);
                        GetInstance().m_spotLightProgram->Bind();
                        enabledSize = 0;
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = spotLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            glViewport(
                                GetInstance().m_spotLights[enabledSize].m_viewPort.x,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.y,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.z,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.w);
                            GetInstance().m_spotLightProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
                                    if (!mmc->IsEnabled() || !mmc->m_castShadow || mmc->m_material == nullptr ||
                                        mmc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(mmc.get()->m_material.get(), true);
                                    auto mesh = mmc->m_mesh;
                                    GetInstance().m_spotLightProgram->SetFloat4x4(
                                        "model", EntityManager::GetComponentData<GlobalTransform>(owner).m_value);
                                    mesh->Enable();
                                    mesh->Vao()->DisableAttributeArray(12);
                                    mesh->Vao()->DisableAttributeArray(13);
                                    mesh->Vao()->DisableAttributeArray(14);
                                    mesh->Vao()->DisableAttributeArray(15);
                                    glDrawElements(
                                        GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
                                }
                            }
                            enabledSize++;
                        }
                        enabledSize = 0;
                        GetInstance().m_spotLightInstancedProgram->Bind();
                        for (int i = 0; i < size; i++)
                        {
                            Entity lightEntity = spotLightEntities->at(i);
                            if (!lightEntity.IsEnabled())
                                continue;
                            glViewport(
                                GetInstance().m_spotLights[enabledSize].m_viewPort.x,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.y,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.z,
                                GetInstance().m_spotLights[enabledSize].m_viewPort.w);
                            GetInstance().m_spotLightInstancedProgram->SetInt("index", enabledSize);
                            const std::vector<Entity> *owners =
                                EntityManager::GetPrivateComponentOwnersList<Particles>();
                            if (owners)
                            {
                                for (auto owner : *owners)
                                {
                                    if (!owner.IsEnabled())
                                        continue;
                                    auto &immc = owner.GetPrivateComponent<Particles>();
                                    if (!immc->IsEnabled() || !immc->m_castShadow || immc->m_material == nullptr ||
                                        immc->m_mesh == nullptr)
                                        continue;
                                    MaterialPropertySetter(immc.get()->m_material.get(), true);
                                    size_t count = immc->m_matrices.size();
                                    std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer =
                                        std::make_unique<OpenGLUtils::GLVBO>();
                                    matricesBuffer->SetData(
                                        (GLsizei)count * sizeof(glm::mat4), immc->m_matrices.data(), GL_STATIC_DRAW);
                                    auto mesh = immc->m_mesh;
                                    GetInstance().m_spotLightInstancedProgram->SetFloat4x4(
                                        "model", EntityManager::GetComponentData<GlobalTransform>(owner).m_value);
                                    mesh->Enable();
                                    mesh->Vao()->EnableAttributeArray(12);
                                    mesh->Vao()->SetAttributePointer(
                                        12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
                                    mesh->Vao()->EnableAttributeArray(13);
                                    mesh->Vao()->SetAttributePointer(
                                        13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(14);
                                    mesh->Vao()->SetAttributePointer(
                                        14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
                                    mesh->Vao()->EnableAttributeArray(15);
                                    mesh->Vao()->SetAttributePointer(
                                        15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
                                    mesh->Vao()->SetAttributeDivisor(12, 1);
                                    mesh->Vao()->SetAttributeDivisor(13, 1);
                                    mesh->Vao()->SetAttributeDivisor(14, 1);
                                    mesh->Vao()->SetAttributeDivisor(15, 1);
                                    glDrawElementsInstanced(
                                        GL_TRIANGLES,
                                        (GLsizei)mesh->GetTriangleAmount() * 3,
                                        GL_UNSIGNED_INT,
                                        0,
                                        (GLsizei)count);
                                    OpenGLUtils::GLVAO::BindDefault();
                                }
                            }
                            enabledSize++;
                        }
#pragma endregion
                    }
                }
                else
                {
                    GetInstance().m_spotLightBlock.SubData(0, 4, &size);
                }
            }
#pragma endregion
        }
    }
#pragma region Render to other cameras
    if (cameraEntities != nullptr)
    {
        for (auto cameraEntity : *cameraEntities)
        {
            if (!cameraEntity.IsEnabled())
                continue;
            auto &cameraComponent = cameraEntity.GetPrivateComponent<CameraComponent>();
            if (GetInstance().m_mainCameraComponent && cameraComponent.get() == GetInstance().m_mainCameraComponent)
                continue;
            if (cameraComponent->IsEnabled())
            {
                auto ltw = cameraEntity.GetComponentData<GlobalTransform>();
                CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                    cameraComponent.get(), ltw.GetPosition(), ltw.GetRotation());
                CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent.get());
                const auto cameraTransform = cameraEntity.GetComponentData<GlobalTransform>();
                RenderToCameraDeferred(cameraComponent, cameraTransform, minBound, maxBound, false);
                RenderBackGround(cameraComponent);
                RenderToCameraForward(cameraComponent, cameraTransform, minBound, maxBound, false);
            }
        }
    }
#pragma endregion
#pragma region Render to scene camera
    if (EditorManager::GetInstance().m_enabled && EditorManager::GetInstance().m_sceneCamera->IsEnabled())
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(
            EditorManager::GetInstance().m_sceneCamera.get(),
            EditorManager::GetInstance().m_sceneCameraPosition,
            EditorManager::GetInstance().m_sceneCameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(EditorManager::GetInstance().m_sceneCamera.get());
        GlobalTransform cameraTransform;
        cameraTransform.m_value = glm::translate(EditorManager::GetInstance().m_sceneCameraPosition) *
                                  glm::mat4_cast(EditorManager::GetInstance().m_sceneCameraRotation);

#pragma region For entity selection
        EditorManager::GetInstance().m_sceneCameraEntityRecorder->Bind();
        const std::vector<Entity> *mmcowners = EntityManager::GetPrivateComponentOwnersList<MeshRenderer>();
        const std::vector<Entity> *immcowners = EntityManager::GetPrivateComponentOwnersList<Particles>();
        if (mmcowners)
        {

            EditorManager::GetInstance().m_sceneCameraEntityRecorderProgram->Bind();
            for (auto owner : *mmcowners)
            {
                if (!owner.IsEnabled())
                    continue;
                auto &mmc = owner.GetPrivateComponent<MeshRenderer>();
                if (!mmc->IsEnabled() || mmc->m_material == nullptr || mmc->m_mesh == nullptr)
                    continue;
                if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                    !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                      static_cast<size_t>(CameraLayer::MainCamera)))
                    continue;
                auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
                auto *mesh = mmc->m_mesh.get();
                mesh->Enable();
                mesh->Vao()->DisableAttributeArray(12);
                mesh->Vao()->DisableAttributeArray(13);
                mesh->Vao()->DisableAttributeArray(14);
                mesh->Vao()->DisableAttributeArray(15);
                EditorManager::GetInstance().m_sceneCameraEntityRecorderProgram->SetInt("EntityIndex", owner.m_index);
                EditorManager::GetInstance().m_sceneCameraEntityRecorderProgram->SetFloat4x4("model", ltw);
                glDrawElements(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
            }
            OpenGLUtils::GLVAO::BindDefault();
        }
        if (immcowners)
        {
            EditorManager::GetInstance().m_sceneCameraEntityInstancedRecorderProgram->Bind();
            for (auto owner : *immcowners)
            {
                if (!owner.IsEnabled())
                    continue;
                auto &immc = owner.GetPrivateComponent<Particles>();
                if (!immc->IsEnabled() || immc->m_material == nullptr || immc->m_mesh == nullptr)
                    continue;
                if (EntityManager::HasComponentData<CameraLayerMask>(owner) &&
                    !(EntityManager::GetComponentData<CameraLayerMask>(owner).m_value &
                      static_cast<size_t>(CameraLayer::MainCamera)))
                    continue;
                auto count = immc->m_matrices.size();
                std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer = std::make_unique<OpenGLUtils::GLVBO>();
                matricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), immc->m_matrices.data(), GL_STATIC_DRAW);
                auto ltw = EntityManager::GetComponentData<GlobalTransform>(owner).m_value;
                auto *mesh = immc->m_mesh.get();
                mesh->Enable();
                mesh->Vao()->EnableAttributeArray(12);
                mesh->Vao()->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
                mesh->Vao()->EnableAttributeArray(13);
                mesh->Vao()->SetAttributePointer(
                    13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
                mesh->Vao()->EnableAttributeArray(14);
                mesh->Vao()->SetAttributePointer(
                    14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
                mesh->Vao()->EnableAttributeArray(15);
                mesh->Vao()->SetAttributePointer(
                    15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
                mesh->Vao()->SetAttributeDivisor(12, 1);
                mesh->Vao()->SetAttributeDivisor(13, 1);
                mesh->Vao()->SetAttributeDivisor(14, 1);
                mesh->Vao()->SetAttributeDivisor(15, 1);
                EditorManager::GetInstance().m_sceneCameraEntityInstancedRecorderProgram->SetInt(
                    "EntityIndex", owner.m_index);
                EditorManager::GetInstance().m_sceneCameraEntityInstancedRecorderProgram->SetFloat4x4("model", ltw);
                glDrawElementsInstanced(
                    GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0, (GLsizei)count);
            }
        }
#pragma endregion
        RenderToCameraDeferred(EditorManager::GetInstance().m_sceneCamera, cameraTransform, minBound, maxBound, false);
        RenderBackGround(EditorManager::GetInstance().m_sceneCamera);
        RenderToCameraForward(EditorManager::GetInstance().m_sceneCamera, cameraTransform, minBound, maxBound, false);
    }
#pragma endregion

#pragma region Render to main Camera and calculate bounds.
    if (GetInstance().m_mainCameraComponent != nullptr)
    {
        auto mainCameraEntity = GetInstance().m_mainCameraComponent->GetOwner();
        if (mainCameraEntity.IsEnabled())
        {
            auto &mainCamera = GetInstance().m_mainCameraComponent;
            if (GetInstance().m_mainCameraComponent->IsEnabled())
            {
                auto minBound = glm::vec3((int)INT_MAX);
                auto maxBound = glm::vec3((int)INT_MIN);
                auto ltw = mainCameraEntity.GetComponentData<GlobalTransform>();
                CameraComponent::m_cameraInfoBlock.UpdateMatrices(mainCamera, ltw.GetPosition(), ltw.GetRotation());
                CameraComponent::m_cameraInfoBlock.UploadMatrices(mainCamera);
                GlobalTransform cameraTransform = mainCameraEntity.GetComponentData<GlobalTransform>();
                auto &mainCameraComponent = mainCameraEntity.GetPrivateComponent<CameraComponent>();
                RenderToCameraDeferred(mainCameraComponent, cameraTransform, minBound, maxBound, true);
                RenderBackGround(mainCameraComponent);
                RenderToCameraForward(mainCameraComponent, cameraTransform, minBound, maxBound, true);
                worldBound.m_max = maxBound;
                worldBound.m_min = minBound;
                Application::GetCurrentWorld()->SetBound(worldBound);
            }
        }
    }
#pragma endregion
}

inline float RenderManager::Lerp(const float &a, const float &b, const float &f)
{
    return a + f * (b - a);
}
#pragma region Settings

#pragma endregion
#pragma region Shadow

void UniEngine::RenderManager::SetSplitRatio(const float &r1, const float &r2, const float &r3, const float &r4)
{
    GetInstance().m_shadowCascadeSplit[0] = r1;
    GetInstance().m_shadowCascadeSplit[1] = r2;
    GetInstance().m_shadowCascadeSplit[2] = r3;
    GetInstance().m_shadowCascadeSplit[3] = r4;
}

void UniEngine::RenderManager::SetShadowMapResolution(const size_t &value)
{
    GetInstance().m_shadowMapResolution = value;
    if (GetInstance().m_directionalLightShadowMap != nullptr)
        GetInstance().m_directionalLightShadowMap->SetResolution(value);
}

glm::vec3 UniEngine::RenderManager::ClosestPointOnLine(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b)
{
    const float lineLength = distance(a, b);
    const glm::vec3 vector = point - a;
    const glm::vec3 lineDirection = (b - a) / lineLength;

    // Project Vector to LineDirection to get the distance of point from a
    const float distance = dot(vector, lineDirection);
    return a + lineDirection * distance;
}

void RenderManager::LateUpdate()
{
    const std::vector<Entity> *postProcessingEntities = EntityManager::GetPrivateComponentOwnersList<PostProcessing>();
    if (postProcessingEntities != nullptr)
    {
        for (auto postProcessingEntity : *postProcessingEntities)
        {
            if (!postProcessingEntity.IsEnabled())
                continue;
            auto &postProcessing = postProcessingEntity.GetPrivateComponent<PostProcessing>();
            if (postProcessing->IsEnabled())
                postProcessing->Process();
        }
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::BeginMenu("Rendering"))
            {
                ImGui::Checkbox("Lighting Manager", &GetInstance().m_enableLightMenu);
                ImGui::Checkbox("Render Manager", &GetInstance().m_enableRenderMenu);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (GetInstance().m_enableLightMenu)
    {
        ImGui::Begin("Light Manager");
        if (ImGui::TreeNode("Environment Lighting"))
        {
            ImGui::DragFloat("Brightness", &GetInstance().m_lightSettings.m_ambientLight, 0.01f, 0.0f, 2.0f);
            ImGui::TreePop();
        }
        bool enableShadow = GetInstance().m_materialSettings.m_enableShadow;
        if (ImGui::Checkbox("Enable shadow", &enableShadow))
        {
            GetInstance().m_materialSettings.m_enableShadow = enableShadow;
        }
        if (GetInstance().m_materialSettings.m_enableShadow && ImGui::TreeNode("Shadow"))
        {
            if (ImGui::TreeNode("Distance"))
            {
                ImGui::DragFloat("Max shadow distance", &GetInstance().m_maxShadowDistance, 1.0f, 0.1f);
                ImGui::DragFloat(
                    "Split 1",
                    &GetInstance().m_shadowCascadeSplit[0],
                    0.01f,
                    0.0f,
                    GetInstance().m_shadowCascadeSplit[1]);
                ImGui::DragFloat(
                    "Split 2",
                    &GetInstance().m_shadowCascadeSplit[1],
                    0.01f,
                    GetInstance().m_shadowCascadeSplit[0],
                    GetInstance().m_shadowCascadeSplit[2]);
                ImGui::DragFloat(
                    "Split 3",
                    &GetInstance().m_shadowCascadeSplit[2],
                    0.01f,
                    GetInstance().m_shadowCascadeSplit[1],
                    GetInstance().m_shadowCascadeSplit[3]);
                ImGui::DragFloat(
                    "Split 4",
                    &GetInstance().m_shadowCascadeSplit[3],
                    0.01f,
                    GetInstance().m_shadowCascadeSplit[2],
                    1.0f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("PCSS"))
            {
                ImGui::DragFloat("PCSS Factor", &GetInstance().m_lightSettings.m_scaleFactor, 0.01f, 0.0f);
                ImGui::DragInt(
                    "Blocker search side amount", &GetInstance().m_lightSettings.m_blockerSearchAmount, 1, 1, 8);
                ImGui::DragInt("PCF Sample Size", &GetInstance().m_lightSettings.m_pcfSampleAmount, 1, 1, 64);
                ImGui::TreePop();
            }
            ImGui::DragFloat("Seam fix ratio", &GetInstance().m_lightSettings.m_seamFixRatio, 0.001f, 0.0f, 0.1f);
            ImGui::Checkbox("Stable fit", &GetInstance().m_stableFit);
            ImGui::TreePop();
        }
        ImGui::End();
    }
    if (GetInstance().m_enableRenderMenu)
    {
        ImGui::Begin("Render Manager");
        ImGui::Checkbox("Display info", &GetInstance().m_enableInfoWindow);
        ImGui::End();
    }

    ImVec2 viewPortSize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGui::Begin("Camera");
    {
        static int corner = 1;
        // Using a Child allow to fill all the space of the window.
        // It also allows customization
        if (ImGui::BeginChild("CameraRenderer"))
        {
            viewPortSize = ImGui::GetWindowSize();
            // Get the size of the child (i.e. the whole draw size of the windows).
            ImVec2 overlayPos = ImGui::GetWindowPos();
            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            bool cameraActive = false;
            if (GetInstance().m_mainCameraComponent != nullptr)
            {
                auto entity = GetInstance().m_mainCameraComponent->GetOwner();
                if (entity.IsEnabled() && GetInstance().m_mainCameraComponent->IsEnabled())
                {
                    auto id = GetInstance().m_mainCameraComponent->GetTexture()->Texture()->Id();
                    ImGui::Image((ImTextureID)id, viewPortSize, ImVec2(0, 1), ImVec2(1, 0));
                    cameraActive = true;
                }
            }
            if (!cameraActive)
            {
                ImGui::Text("No active main camera!");
            }

            ImVec2 window_pos = ImVec2(
                (corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
                (corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y));
            if (GetInstance().m_enableInfoWindow)
            {
                ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

                ImGui::BeginChild("Render Info", ImVec2(200, 100), false, window_flags);
                ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
                std::string trisstr = "";
                if (GetInstance().m_triangles < 999)
                    trisstr += std::to_string(GetInstance().m_triangles);
                else if (GetInstance().m_triangles < 999999)
                    trisstr += std::to_string((int)(GetInstance().m_triangles / 1000)) + "K";
                else
                    trisstr += std::to_string((int)(GetInstance().m_triangles / 1000000)) + "M";
                trisstr += " tris";
                ImGui::Text(trisstr.c_str());
                ImGui::Text("%d drawcall", GetInstance().m_drawCall);
                ImGui::Separator();
                if (ImGui::IsMousePosValid())
                {
                    glm::vec2 pos;
                    InputManager::GetMousePositionInternal(ImGui::GetCurrentWindowRead(), pos);
                    ImGui::Text("Mouse Position: (%.1f,%.1f)", pos.x, pos.y);
                }
                else
                {
                    ImGui::Text("Mouse Position: <invalid>");
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    GetInstance().m_mainCameraResolutionX = viewPortSize.x;
    GetInstance().m_mainCameraResolutionY = viewPortSize.y;
}

#pragma endregion
#pragma region RenderAPI
#pragma region Internal

void RenderManager::MaterialPropertySetter(const Material *material, const bool &disableBlending)
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

void RenderManager::ApplyMaterialSettings(const Material *material, const OpenGLUtils::GLProgram *program)
{
    GetInstance().m_materialSettings.m_alphaDiscardEnabled = material->m_alphaDiscardEnabled;
    GetInstance().m_materialSettings.m_alphaDiscardOffset = material->m_alphaDiscardOffset;
    GetInstance().m_materialSettings.m_displacementScale = 0.0f;
    GetInstance().m_materialSettings.m_albedoColorVal = glm::vec4(material->m_albedoColor, 1.0f);
    GetInstance().m_materialSettings.m_shininessVal = material->m_shininess;
    GetInstance().m_materialSettings.m_metallicVal = material->m_metallic;
    GetInstance().m_materialSettings.m_roughnessVal = material->m_roughness;
    GetInstance().m_materialSettings.m_aoVal = material->m_ambientOcclusion;
    GetInstance().m_materialSettings.m_directionalShadowMap =
        GetInstance().m_directionalLightShadowMap->DepthMapArray()->GetHandle();
    GetInstance().m_materialSettings.m_pointShadowMap =
        GetInstance().m_pointLightShadowMap->DepthMapArray()->GetHandle();
    GetInstance().m_materialSettings.m_spotShadowMap = GetInstance().m_spotLightShadowMap->DepthMap()->GetHandle();
    GetInstance().m_materialSettingsBuffer->SubData(
        0, sizeof(MaterialSettingsBlock), &GetInstance().m_materialSettings);
}

void RenderManager::BindTextureHandles(const Material *material)
{
    for (const auto &i : material->m_textures)
    {
        if (!i.second || !i.second->Texture())
            continue;
        switch (i.second->m_type)
        {
        case TextureType::Albedo:
            GetInstance().m_materialSettings.m_albedoMap = i.second->Texture()->GetHandle();
            GetInstance().m_materialSettings.m_albedoEnabled = static_cast<int>(true);
            break;
        case TextureType::Normal:
            GetInstance().m_materialSettings.m_normalMap = i.second->Texture()->GetHandle();
            GetInstance().m_materialSettings.m_normalEnabled = static_cast<int>(true);
            break;
        case TextureType::Metallic:
            GetInstance().m_materialSettings.m_metallicMap = i.second->Texture()->GetHandle();
            GetInstance().m_materialSettings.m_metallicEnabled = static_cast<int>(true);
            break;
        case TextureType::Roughness:
            GetInstance().m_materialSettings.m_roughnessMap = i.second->Texture()->GetHandle();
            GetInstance().m_materialSettings.m_roughnessEnabled = static_cast<int>(true);
            break;
        case TextureType::AO:
            GetInstance().m_materialSettings.m_aoMap = i.second->Texture()->GetHandle();
            GetInstance().m_materialSettings.m_aoEnabled = static_cast<int>(true);
            break;
        }
    }
}

void RenderManager::ReleaseTextureHandles(const Material *material)
{
    return;
    for (const auto &i : material->m_textures)
    {
        if (!i.second || !i.second->Texture())
            continue;
        i.second->Texture()->MakeNonResident();
    }
}

void RenderManager::DeferredPrepass(const Mesh *mesh, const Material *material, const glm::mat4 &model)
{
    if (mesh == nullptr || material == nullptr)
        return;
    mesh->Enable();
    mesh->Vao()->DisableAttributeArray(12);
    mesh->Vao()->DisableAttributeArray(13);
    mesh->Vao()->DisableAttributeArray(14);
    mesh->Vao()->DisableAttributeArray(15);

    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    auto &program = GetInstance().m_gBufferPrepass;
    program->SetFloat4x4("model", model);
    for (auto j : material->m_floatPropertyList)
    {
        program->SetFloat(j.m_name, j.m_value);
    }
    for (auto j : material->m_float4X4PropertyList)
    {
        program->SetFloat4x4(j.m_name, j.m_value);
    }
    MaterialPropertySetter(material, true);
    GetInstance().m_materialSettings = MaterialSettingsBlock();
    BindTextureHandles(material);
    ApplyMaterialSettings(material, program.get());
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
    ReleaseTextureHandles(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderManager::DeferredPrepassInstanced(
    const Mesh *mesh, const Material *material, const glm::mat4 &model, glm::mat4 *matrices, const size_t &count)
{
    if (mesh == nullptr || material == nullptr || matrices == nullptr || count == 0)
        return;
    std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer = std::make_unique<OpenGLUtils::GLVBO>();
    matricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices, GL_STATIC_DRAW);
    mesh->Enable();
    mesh->Vao()->EnableAttributeArray(12);
    mesh->Vao()->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    mesh->Vao()->EnableAttributeArray(13);
    mesh->Vao()->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    mesh->Vao()->EnableAttributeArray(14);
    mesh->Vao()->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    mesh->Vao()->EnableAttributeArray(15);
    mesh->Vao()->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    mesh->Vao()->SetAttributeDivisor(12, 1);
    mesh->Vao()->SetAttributeDivisor(13, 1);
    mesh->Vao()->SetAttributeDivisor(14, 1);
    mesh->Vao()->SetAttributeDivisor(15, 1);

    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * count;
    auto &program = GetInstance().m_gBufferInstancedPrepass;
    program->SetFloat4x4("model", model);
    for (auto j : material->m_floatPropertyList)
    {
        program->SetFloat(j.m_name, j.m_value);
    }
    for (auto j : material->m_float4X4PropertyList)
    {
        program->SetFloat4x4(j.m_name, j.m_value);
    }
    MaterialPropertySetter(material, true);
    GetInstance().m_materialSettings = MaterialSettingsBlock();
    BindTextureHandles(material);
    ApplyMaterialSettings(material, program.get());
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0, (GLsizei)count);
    ReleaseTextureHandles(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void UniEngine::RenderManager::DrawMeshInstanced(
    const Mesh *mesh,
    const Material *material,
    const glm::mat4 &model,
    const glm::mat4 *matrices,
    const size_t &count,
    const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr || matrices == nullptr || count == 0)
        return;
    std::unique_ptr<OpenGLUtils::GLVBO> matricesBuffer = std::make_unique<OpenGLUtils::GLVBO>();
    matricesBuffer->SetData((GLsizei)count * sizeof(glm::mat4), matrices, GL_STATIC_DRAW);
    mesh->Enable();
    mesh->Vao()->EnableAttributeArray(12);
    mesh->Vao()->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    mesh->Vao()->EnableAttributeArray(13);
    mesh->Vao()->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    mesh->Vao()->EnableAttributeArray(14);
    mesh->Vao()->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    mesh->Vao()->EnableAttributeArray(15);
    mesh->Vao()->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    mesh->Vao()->SetAttributeDivisor(12, 1);
    mesh->Vao()->SetAttributeDivisor(13, 1);
    mesh->Vao()->SetAttributeDivisor(14, 1);
    mesh->Vao()->SetAttributeDivisor(15, 1);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * count;
    auto program = material->m_program.get();
    if (program == nullptr)
        program = Default::GLPrograms::StandardInstancedProgram.get();
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
    BindTextureHandles(material);
    ApplyMaterialSettings(material, program);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0, (GLsizei)count);
    ReleaseTextureHandles(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void UniEngine::RenderManager::DrawMesh(
    const Mesh *mesh, const Material *material, const glm::mat4 &model, const bool &receiveShadow)
{
    if (mesh == nullptr || material == nullptr)
        return;
    mesh->Enable();
    mesh->Vao()->DisableAttributeArray(12);
    mesh->Vao()->DisableAttributeArray(13);
    mesh->Vao()->DisableAttributeArray(14);
    mesh->Vao()->DisableAttributeArray(15);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    auto program = material->m_program.get();
    if (program == nullptr)
        program = Default::GLPrograms::StandardProgram.get();
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
    BindTextureHandles(material);
    ApplyMaterialSettings(material, program);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
    ReleaseTextureHandles(material);
    OpenGLUtils::GLVAO::BindDefault();
}

void UniEngine::RenderManager::DrawTexture2D(
    const OpenGLUtils::GLTexture2D *texture, const float &depth, const glm::vec2 &center, const glm::vec2 &size)
{
    const auto program = Default::GLPrograms::ScreenProgram;
    program->Bind();
    Default::GLPrograms::ScreenVAO->Bind();
    texture->Bind(0);
    program->SetInt("screenTexture", 0);
    program->SetFloat("depth", depth);
    program->SetFloat2("center", center);
    program->SetFloat2("size", size);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UniEngine::RenderManager::DrawGizmoMeshInstanced(
    const Mesh *mesh,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices == nullptr || count == 0)
        return;
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    const auto vao = mesh->Vao();
    const OpenGLUtils::GLVBO matricesBuffer;
    matricesBuffer.SetData(static_cast<GLsizei>(count) * sizeof(glm::mat4), matrices, GL_STATIC_DRAW);
    vao->EnableAttributeArray(12);
    vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    vao->EnableAttributeArray(13);
    vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    vao->EnableAttributeArray(14);
    vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    vao->EnableAttributeArray(15);
    vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    vao->SetAttributeDivisor(12, 1);
    vao->SetAttributeDivisor(13, 1);
    vao->SetAttributeDivisor(14, 1);
    vao->SetAttributeDivisor(15, 1);

    Default::GLPrograms::GizmoInstancedProgram->Bind();
    Default::GLPrograms::GizmoInstancedProgram->SetFloat4("surfaceColor", color);
    Default::GLPrograms::GizmoInstancedProgram->SetFloat4x4("model", model);
    Default::GLPrograms::GizmoInstancedProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * count;
    glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(mesh->GetTriangleAmount()) * 3,
        GL_UNSIGNED_INT,
        0,
        static_cast<GLsizei>(count));

    vao->DisableAttributeArray(12);
    vao->DisableAttributeArray(13);
    vao->DisableAttributeArray(14);
    vao->DisableAttributeArray(15);
    OpenGLUtils::GLVAO::BindDefault();
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const Mesh *mesh,
    const glm::vec4 *colors,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &model,
    const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr || matrices == nullptr || count == 0)
        return;
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mesh->Enable();
    const auto vao = mesh->Vao();
    const OpenGLUtils::GLVBO colorsBuffer;
    colorsBuffer.SetData(static_cast<GLsizei>(count) * sizeof(glm::vec4), colors, GL_STATIC_DRAW);
    vao->EnableAttributeArray(11);
    vao->SetAttributePointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
    vao->SetAttributeDivisor(11, 1);

    const OpenGLUtils::GLVBO matricesBuffer;
    matricesBuffer.SetData(static_cast<GLsizei>(count) * sizeof(glm::mat4), matrices, GL_STATIC_DRAW);
    vao->EnableAttributeArray(12);
    vao->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    vao->EnableAttributeArray(13);
    vao->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    vao->EnableAttributeArray(14);
    vao->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    vao->EnableAttributeArray(15);
    vao->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    vao->SetAttributeDivisor(12, 1);
    vao->SetAttributeDivisor(13, 1);
    vao->SetAttributeDivisor(14, 1);
    vao->SetAttributeDivisor(15, 1);

    Default::GLPrograms::GizmoInstancedColoredProgram->Bind();
    Default::GLPrograms::GizmoInstancedColoredProgram->SetFloat4x4("model", model);
    Default::GLPrograms::GizmoInstancedColoredProgram->SetFloat4x4("scaleMatrix", scaleMatrix);
    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount() * count;
    glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(mesh->GetTriangleAmount()) * 3,
        GL_UNSIGNED_INT,
        0,
        static_cast<GLsizei>(count));

    vao->DisableAttributeArray(11);
    vao->DisableAttributeArray(12);
    vao->DisableAttributeArray(13);
    vao->DisableAttributeArray(14);
    vao->DisableAttributeArray(15);
    OpenGLUtils::GLVAO::BindDefault();
}

void UniEngine::RenderManager::DrawGizmoMesh(
    const Mesh *mesh, const glm::vec4 &color, const glm::mat4 &model, const glm::mat4 &scaleMatrix)
{
    if (mesh == nullptr)
        return;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    mesh->Enable();
    mesh->Vao()->DisableAttributeArray(12);
    mesh->Vao()->DisableAttributeArray(13);
    mesh->Vao()->DisableAttributeArray(14);
    mesh->Vao()->DisableAttributeArray(15);

    Default::GLPrograms::GizmoProgram->Bind();
    Default::GLPrograms::GizmoProgram->SetFloat4("surfaceColor", color);
    Default::GLPrograms::GizmoProgram->SetFloat4x4("model", model);
    Default::GLPrograms::GizmoProgram->SetFloat4x4("scaleMatrix", scaleMatrix);

    GetInstance().m_drawCall++;
    GetInstance().m_triangles += mesh->GetTriangleAmount();
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->GetTriangleAmount() * 3, GL_UNSIGNED_INT, 0);
    OpenGLUtils::GLVAO::BindDefault();
}
#pragma endregion

#pragma region External
void UniEngine::RenderManager::DrawGizmoMeshInstanced(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec4 &color,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &model,
    const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawGizmoMeshInstanced(mesh, color, model, matrices, count, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
        }
    }
    if (cameraComponent == sceneCamera.get())
        return;
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();

    const auto ltw = EntityManager::GetComponentData<GlobalTransform>(entity);
    glm::vec3 scale;
    glm::vec3 trans;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(ltw.m_value, scale, rotation, trans, skew, perspective);
    CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, trans, rotation);
    CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    DrawGizmoMeshInstanced(mesh, color, model, matrices, count, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec4 *colors,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &model,
    const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawGizmoMeshInstancedColored(mesh, colors, matrices, count, model, glm::scale(glm::vec3(size)));
        }
    }
    if (cameraComponent == sceneCamera.get())
        return;
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();

    const auto ltw = EntityManager::GetComponentData<GlobalTransform>(entity);
    glm::vec3 scale;
    glm::vec3 trans;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(ltw.m_value, scale, rotation, trans, skew, perspective);
    CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, trans, rotation);
    CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    DrawGizmoMeshInstancedColored(mesh, colors, matrices, count, model, glm::scale(glm::vec3(size)));
}

void RenderManager::DrawGizmoMesh(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const float &size)
{
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
        DrawGizmoMesh(mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
    }
}

void RenderManager::DrawGizmoMeshInstanced(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &model,
    const float &size)
{
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
        DrawGizmoMeshInstanced(mesh, color, model, matrices, count, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
    }
}

void RenderManager::DrawGizmoMeshInstancedColored(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 *colors,
    const glm::mat4 *matrices,
    const size_t &count,
    const glm::mat4 &model,
    const float &size)
{
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
        DrawGizmoMeshInstancedColored(mesh, colors, matrices, count, model, glm::scale(glm::vec3(size)));
    }
}

void RenderManager::DrawGizmoRay(
    const CameraComponent *cameraComponent,
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
    DrawGizmoMesh(Default::Primitives::Cylinder.get(), cameraComponent, color, model);
}

void RenderManager::DrawGizmoRays(
    const CameraComponent *cameraComponent,
    const glm::vec4 &color,
    std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
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
        Default::Primitives::Cylinder.get(), cameraComponent, color, models.data(), connections.size());
}

void RenderManager::DrawGizmoRays(
    const CameraComponent *cameraComponent, const glm::vec4 &color, std::vector<Ray> &rays, const float &width)
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
    DrawGizmoMeshInstanced(Default::Primitives::Cylinder.get(), cameraComponent, color, models.data(), rays.size());
}

void RenderManager::DrawGizmoRay(
    const CameraComponent *cameraComponent, const glm::vec4 &color, Ray &ray, const float &width)
{
    glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
    const glm::mat4 rotationMat = glm::mat4_cast(rotation);
    const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                       glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
    DrawGizmoMesh(Default::Primitives::Cylinder.get(), cameraComponent, color, model);
}

void RenderManager::DrawGizmoRay(
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    const glm::vec3 &start,
    const glm::vec3 &end,
    const float &width)
{
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
        glm::quat rotation = glm::quatLookAt(end - start, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        const glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((start + end) / 2.0f) * rotationMat *
                           glm::scale(glm::vec3(width, glm::distance(end, start) / 2.0f, width));
        DrawGizmoMesh(Default::Primitives::Cylinder.get(), cameraComponent, color, model);
    }
}

void RenderManager::DrawGizmoRays(
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    std::vector<std::pair<glm::vec3, glm::vec3>> &connections,
    const float &width)
{
    if (connections.empty())
        return;
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
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
            Default::Primitives::Cylinder.get(), cameraComponent, color, models.data(), connections.size());
    }
}

void RenderManager::DrawGizmoRays(
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    std::vector<Ray> &rays,
    const float &width)
{
    if (rays.empty())
        return;
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
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
        DrawGizmoMeshInstanced(Default::Primitives::Cylinder.get(), cameraComponent, color, models.data(), rays.size());
    }
}

void RenderManager::DrawGizmoRay(
    const CameraComponent *cameraComponent,
    const glm::vec3 &cameraPosition,
    const glm::quat &cameraRotation,
    const glm::vec4 &color,
    Ray &ray,
    const float &width)
{
    if (cameraComponent)
    {
        CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, cameraPosition, cameraRotation);
        CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
        cameraComponent->Bind();
        glm::quat rotation = glm::quatLookAt(ray.m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation *= glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
        const glm::mat4 rotationMat = glm::mat4_cast(rotation);
        const auto model = glm::translate((ray.m_start + ray.m_direction * ray.m_length / 2.0f)) * rotationMat *
                           glm::scale(glm::vec3(width, ray.m_length / 2.0f, width));
        DrawGizmoMesh(Default::Primitives::Cylinder.get(), cameraComponent, color, model);
    }
}

void RenderManager::DrawMesh(
    const Mesh *mesh,
    const Material *material,
    const glm::mat4 &model,
    const CameraComponent *cameraComponent,
    const bool &receiveShadow)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawMesh(mesh, material, model, receiveShadow);
        }
    }
    if (cameraComponent == sceneCamera.get())
        return;
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();

    const auto ltw = EntityManager::GetComponentData<GlobalTransform>(entity);
    glm::vec3 scale;
    glm::vec3 trans;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(ltw.m_value, scale, rotation, trans, skew, perspective);
    CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, trans, rotation);
    CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    DrawMesh(mesh, material, model, receiveShadow);
}

void RenderManager::DrawMeshInstanced(
    const Mesh *mesh,
    const Material *material,
    const glm::mat4 &model,
    const glm::mat4 *matrices,
    const size_t &count,
    const CameraComponent *cameraComponent,
    const bool &receiveShadow)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawMeshInstanced(mesh, material, model, matrices, count, receiveShadow);
        }
    }
    if (cameraComponent == sceneCamera.get())
        return;
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();

    const auto ltw = EntityManager::GetComponentData<GlobalTransform>(entity);
    glm::vec3 scale;
    glm::vec3 trans;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(ltw.m_value, scale, rotation, trans, skew, perspective);
    CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, trans, rotation);
    CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    DrawMeshInstanced(mesh, material, model, matrices, count, receiveShadow);
}

#pragma region DrawTexture
void UniEngine::RenderManager::DrawTexture2D(
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
    const CameraComponent *cameraComponent)
{
    if (EditorManager::GetInstance().m_enabled)
    {
        auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
        if (cameraComponent != sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawTexture2D(texture->Texture().get(), depth, center, size);
        }
    }
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();
    if (!entity.IsEnabled())
        return;
    cameraComponent->Bind();
    DrawTexture2D(texture->Texture().get(), depth, center, size);
}
void RenderManager::SetMainCamera(CameraComponent *value)
{
    if (GetInstance().m_mainCameraComponent)
    {
        GetInstance().m_mainCameraComponent->m_isMainCamera = false;
    }
    GetInstance().m_mainCameraComponent = value;
    if (GetInstance().m_mainCameraComponent)
        GetInstance().m_mainCameraComponent->m_isMainCamera = true;
}

CameraComponent *RenderManager::GetMainCamera()
{
    return GetInstance().m_mainCameraComponent;
}

void UniEngine::RenderManager::DrawTexture2D(
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

void UniEngine::RenderManager::DrawGizmoMesh(
    const Mesh *mesh,
    const CameraComponent *cameraComponent,
    const glm::vec4 &color,
    const glm::mat4 &model,
    const float &size)
{
    auto &sceneCamera = EditorManager::GetInstance().m_sceneCamera;
    if (EditorManager::GetInstance().m_enabled)
    {
        if (sceneCamera.get() && sceneCamera->IsEnabled())
        {
            CameraComponent::m_cameraInfoBlock.UpdateMatrices(
                sceneCamera.get(),
                EditorManager::GetInstance().m_sceneCameraPosition,
                EditorManager::GetInstance().m_sceneCameraRotation);
            CameraComponent::m_cameraInfoBlock.UploadMatrices(sceneCamera.get());
            sceneCamera->Bind();
            DrawGizmoMesh(mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
        }
    }
    if (cameraComponent == sceneCamera.get())
        return;
    if (cameraComponent == nullptr || !cameraComponent->IsEnabled())
        return;
    const auto entity = cameraComponent->GetOwner();
    if (entity.IsNull() || !entity.IsValid() || !entity.IsEnabled())
        return;
    const auto ltw = EntityManager::GetComponentData<GlobalTransform>(entity);
    glm::vec3 scale;
    glm::vec3 trans;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(ltw.m_value, scale, rotation, trans, skew, perspective);
    CameraComponent::m_cameraInfoBlock.UpdateMatrices(cameraComponent, trans, rotation);
    CameraComponent::m_cameraInfoBlock.UploadMatrices(cameraComponent);
    cameraComponent->Bind();
    DrawGizmoMesh(mesh, color, model, glm::scale(glm::mat4(1.0f), glm::vec3(size)));
}
#pragma endregion
#pragma endregion

#pragma region Status

size_t UniEngine::RenderManager::Triangles()
{
    return GetInstance().m_triangles;
}

size_t UniEngine::RenderManager::DrawCall()
{
    return GetInstance().m_drawCall;
}

#pragma endregion
#pragma endregion