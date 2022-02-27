//
// Created by lllll on 11/2/2021.
//
#include "EditorLayer.hpp"
#include "Editor.hpp"
#include "Engine/Core/Inputs.hpp"
#include "Engine/Core/Windows.hpp"
#include "RenderLayer.hpp"
#include <Application.hpp>
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <DefaultResources.hpp>
#include <Joint.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <Particles.hpp>
#include <PhysicsLayer.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <RigidBody.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <Utilities.hpp>
using namespace UniEngine;
void EditorLayer::OnCreate()
{
    m_basicEntityArchetype = Entities::CreateEntityArchetype("General", GlobalTransform(), Transform());

    m_sceneCameraEntityRecorderTexture = std::make_unique<OpenGLUtils::GLTexture2D>(
        1, GL_R32F, m_sceneCameraResolutionX, m_sceneCameraResolutionY, false);
    m_sceneCameraEntityRecorderTexture->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_sceneCameraEntityRecorderTexture->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_sceneCameraEntityRecorderTexture->ReSize(
        0, GL_R32F, GL_RED, GL_FLOAT, 0, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    m_sceneCameraEntityRecorderRenderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    m_sceneCameraEntityRecorder = std::make_unique<RenderTarget>(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    m_sceneCameraEntityRecorderRenderBuffer->AllocateStorage(
        GL_DEPTH24_STENCIL8, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    m_sceneCameraEntityRecorder->SetResolution(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    m_sceneCameraEntityRecorder->AttachRenderBuffer(
        m_sceneCameraEntityRecorderRenderBuffer.get(), GL_DEPTH_STENCIL_ATTACHMENT);
    m_sceneCameraEntityRecorder->AttachTexture(m_sceneCameraEntityRecorderTexture.get(), GL_COLOR_ATTACHMENT0);
    m_selectedEntity = Entity();
    m_configFlags += EntityEditorSystem_EnableEntityHierarchy;
    m_configFlags += EntityEditorSystem_EnableEntityInspector;

    m_sceneCamera = Serialization::ProduceSerializable<Camera>();
    m_sceneCamera->m_clearColor = glm::vec3(0.5f);
    m_sceneCamera->m_useClearColor = false;
    m_sceneCamera->OnCreate();

    Editor::RegisterComponentDataInspector<GlobalTransform>([](Entity entity, IDataComponent *data, bool isRoot) {
        auto *ltw = reinterpret_cast<GlobalTransform *>(data);
        glm::vec3 er;
        glm::vec3 t;
        glm::vec3 s;
        ltw->Decompose(t, er, s);
        er = glm::degrees(er);
        ImGui::DragFloat3("Position##Global", &t.x, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
        ImGui::DragFloat3("Rotation##Global", &er.x, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
        ImGui::DragFloat3("Scale##Global", &s.x, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ReadOnly);
        return false;
    });
    Editor::RegisterComponentDataInspector<Transform>([&](Entity entity, IDataComponent *data, bool isRoot) {
        static Entity previousEntity;
        auto *ltp = static_cast<Transform *>(static_cast<void *>(data));
        bool edited = false;
        bool reload = previousEntity != entity;
        bool readOnly = false;
        if (Application::IsPlaying() && entity.HasPrivateComponent<RigidBody>())
        {
            auto rigidBody = entity.GetOrSetPrivateComponent<RigidBody>().lock();
            if (!rigidBody->IsKinematic() && rigidBody->Registered())
            {
                reload = true;
                readOnly = true;
            }
        }
        if (reload)
        {
            previousEntity = entity;
            ltp->Decompose(m_previouslyStoredPosition, m_previouslyStoredRotation, m_previouslyStoredScale);
            m_previouslyStoredRotation = glm::degrees(m_previouslyStoredRotation);
            m_localPositionSelected = true;
            m_localRotationSelected = false;
            m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3(
                "##LocalPosition",
                &m_previouslyStoredPosition.x,
                0.1f,
                0,
                0,
                "%.3f",
                readOnly ? ImGuiSliderFlags_ReadOnly : 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Position##Local", &m_localPositionSelected) && m_localPositionSelected)
        {
            m_localRotationSelected = false;
            m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3(
                "##LocalRotation",
                &m_previouslyStoredRotation.x,
                1.0f,
                0,
                0,
                "%.3f",
                readOnly ? ImGuiSliderFlags_ReadOnly : 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Rotation##Local", &m_localRotationSelected) && m_localRotationSelected)
        {
            m_localPositionSelected = false;
            m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3(
                "##LocalScale",
                &m_previouslyStoredScale.x,
                0.01f,
                0,
                0,
                "%.3f",
                readOnly ? ImGuiSliderFlags_ReadOnly : 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Scale##Local", &m_localScaleSelected) && m_localScaleSelected)
        {
            m_localRotationSelected = false;
            m_localPositionSelected = false;
        }
        if (edited)
        {
            ltp->m_value = glm::translate(m_previouslyStoredPosition) *
                           glm::mat4_cast(glm::quat(glm::radians(m_previouslyStoredRotation))) *
                           glm::scale(m_previouslyStoredScale);
        }
        return edited;
    });
    Editor::RegisterComponentDataInspector<Ray>([&](Entity entity, IDataComponent *data, bool isRoot) {
        auto *ray = static_cast<Ray *>(static_cast<void *>(data));
        bool changed = false;
        if (ImGui::InputFloat3("Start", &ray->m_start.x))
            changed = true;
        if (ImGui::InputFloat3("Direction", &ray->m_direction.x))
            changed = true;
        if (ImGui::InputFloat("Length", &ray->m_length))
            changed = true;
        return changed;
    });
}
void EditorLayer::PreUpdate()
{
    m_mainCameraFocusOverride = false;
    m_sceneCameraFocusOverride = false;
    if (ImGui::BeginMainMenuBar())
    {
        switch (Application::GetGameStatus())
        {
        case GameStatus::Stop: {
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["PlayButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Play();
            }
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["StepButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Step();
            }
            break;
        }
        case GameStatus::Playing: {
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["PauseButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Pause();
            }
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["StopButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Stop();
            }
            break;
        }
        case GameStatus::Pause: {
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["PlayButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Play();
            }
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["StepButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Step();
            }
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["StopButton"]->UnsafeGetGLTexture()->Id(),
                    {15, 15},
                    {0, 1},
                    {1, 0}))
            {
                Application::Stop();
            }
            break;
        }
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Project"))
        {
            ImGui::EndMenu();
        }
        /*
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        */
        if (ImGui::BeginMenu("View"))
        {
            ImGui::EndMenu();
        }
        /*
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }
        */
        ImGui::EndMainMenuBar();
    }

    RenderToSceneCamera();
}
void EditorLayer::LateUpdate()
{
}

bool EditorLayer::DrawEntityMenu(const bool &enabled, const Entity &entity)
{
    bool deleted = false;
    if (ImGui::BeginPopupContextItem(std::to_string(entity.GetIndex()).c_str()))
    {
        ImGui::Text(("Handle: " + std::to_string(entity.GetHandle().GetValue())).c_str());
        if (ImGui::Button("Delete"))
        {
            Entities::DeleteEntity(Entities::GetCurrentScene(), entity);
            deleted = true;
        }
        if (!deleted && ImGui::Button(enabled ? "Disable" : "Enable"))
        {
            if (enabled)
            {
                entity.SetEnabled(false);
            }
            else
            {
                entity.SetEnabled(true);
            }
        }
        if (!deleted && ImGui::BeginMenu("Rename"))
        {
            static char newName[256];
            ImGui::InputText("New name", newName, 256);
            if (ImGui::Button("Confirm"))
                Entities::SetEntityName(Entities::GetCurrentScene(), entity, std::string(newName));

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    return deleted;
}
void EditorLayer::InspectComponentData(Entity entity, IDataComponent *data, DataComponentType type, bool isRoot)
{
    auto &editorManager = Editor::GetInstance();
    if (editorManager.m_componentDataInspectorMap.find(type.m_typeId) !=
        editorManager.m_componentDataInspectorMap.end())
    {
        if (editorManager.m_componentDataInspectorMap.at(type.m_typeId)(entity, data, isRoot))
        {
            Entities::GetCurrentScene()->SetUnsaved();
        }
    }
}

Entity EditorLayer::MouseEntitySelection(const glm::vec2 &mousePosition)
{
    Entity retVal;
    m_sceneCameraEntityRecorder->Bind();
    float entityIndex = 0;
    const glm::vec2 resolution = m_sceneCameraEntityRecorder->GetResolution();
    glm::vec2 point = resolution;
    point.x += mousePosition.x;
    point.y -= mousePosition.y;
    if (point.x >= 0 && point.x < resolution.x && point.y >= 0 && point.y < resolution.y)
    {
        glReadPixels(point.x, point.y, 1, 1, GL_RED, GL_FLOAT, &entityIndex);
        if (entityIndex > 0)
        {
            retVal = Entities::GetEntity(Entities::GetCurrentScene(), static_cast<unsigned>(entityIndex));
        }
    }
    return retVal;
}

void EditorLayer::HighLightEntityPrePassHelper(const Entity &entity)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    Entities::ForEachChild(Entities::GetCurrentScene(), entity, [&](const std::shared_ptr<Scene> &scene, Entity child) {
        HighLightEntityPrePassHelper(child);
    });
    if (entity.HasPrivateComponent<MeshRenderer>())
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            DefaultResources::m_sceneHighlightPrePassProgram->SetFloat4x4(
                "model", Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity).m_value);
            mesh->Draw();
        }
    }
    if (entity.HasPrivateComponent<Particles>())
    {
        auto immc = entity.GetOrSetPrivateComponent<Particles>().lock();
        auto material = immc->m_material.Get<Material>();
        auto mesh = immc->m_mesh.Get<Mesh>();
        if (immc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            DefaultResources::m_sceneHighlightPrePassInstancedProgram->SetFloat4x4(
                "model", Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity).m_value);
            mesh->DrawInstanced(immc->m_matrices);
        }
    }
    if (entity.HasPrivateComponent<SkinnedMeshRenderer>())
    {
        auto smmc = entity.GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
        auto material = smmc->m_material.Get<Material>();
        auto skinnedMesh = smmc->m_skinnedMesh.Get<SkinnedMesh>();
        if (smmc->IsEnabled() && material != nullptr && skinnedMesh != nullptr)
        {
            GlobalTransform ltw;
            if (!smmc->RagDoll())
                ltw = Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity);
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedPrePassProgram->SetFloat4x4("model", ltw.m_value);
            skinnedMesh->Draw();
        }
    }
}

void EditorLayer::HighLightEntityHelper(const Entity &entity)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    Entities::ForEachChild(Entities::GetCurrentScene(), entity, [&](const std::shared_ptr<Scene> &scene, Entity child) {
        HighLightEntityHelper(child);
    });
    if (entity.HasPrivateComponent<MeshRenderer>())
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            auto ltw = Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity);
            DefaultResources::m_sceneHighlightProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightProgram->SetFloat3("scale", ltw.GetScale());
            mesh->Draw();
        }
    }
    if (entity.HasPrivateComponent<Particles>())
    {
        auto immc = entity.GetOrSetPrivateComponent<Particles>().lock();
        auto material = immc->m_material.Get<Material>();
        auto mesh = immc->m_mesh.Get<Mesh>();
        if (immc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            auto ltw = Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity);
            DefaultResources::m_sceneHighlightInstancedProgram->SetFloat4x4("model", ltw.m_value);
            mesh->DrawInstanced(immc->m_matrices);
        }
    }
    if (entity.HasPrivateComponent<SkinnedMeshRenderer>())
    {
        auto smmc = entity.GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
        auto material = smmc->m_material.Get<Material>();
        auto skinnedMesh = smmc->m_skinnedMesh.Get<SkinnedMesh>();
        if (smmc->IsEnabled() && material != nullptr && skinnedMesh != nullptr)
        {
            GlobalTransform ltw;
            if (!smmc->RagDoll())
                ltw = Entities::GetDataComponent<GlobalTransform>(Entities::GetCurrentScene(), entity);
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat3("scale", ltw.GetScale());
            skinnedMesh->Draw();
        }
    }
}

void EditorLayer::MoveCamera(
    const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime)
{
    m_previousRotation = m_sceneCameraRotation;
    m_previousPosition = m_sceneCameraPosition;
    m_transitionTime = transitionTime;
    m_transitionTimer = Application::Time().CurrentTime();
    m_targetRotation = targetRotation;
    m_targetPosition = targetPosition;
    m_lockCamera = true;
    m_leftMouseButtonHold = false;
    m_rightMouseButtonHold = false;
    m_startMouse = false;
}

void EditorLayer::HighLightEntity(const Entity &entity, const glm::vec4 &color)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    Camera::m_cameraInfoBlock.UpdateMatrices(m_sceneCamera, m_sceneCameraPosition, m_sceneCameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(m_sceneCamera);
    m_sceneCamera->Bind();
    OpenGLUtils::SetEnable(OpenGLCapability::StencilTest, true);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    DefaultResources::m_sceneHighlightPrePassProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
    DefaultResources::m_sceneHighlightSkinnedPrePassProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
    DefaultResources::m_sceneHighlightPrePassInstancedProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
    DefaultResources::m_sceneHighlightPrePassInstancedSkinnedProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));

    HighLightEntityPrePassHelper(entity);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    DefaultResources::m_sceneHighlightProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightInstancedProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightInstancedSkinnedProgram->SetFloat4("color", color);

    HighLightEntityHelper(entity);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
}
static const char *HierarchyDisplayMode[]{"Archetype", "Hierarchy"};

void EditorLayer::RenderToSceneCamera()
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    ProfilerLayer::StartEvent("RenderToSceneCamera");

    const auto resolution = m_sceneCamera->UnsafeGetGBuffer()->GetResolution();
    if (m_sceneCameraResolutionX != 0 && m_sceneCameraResolutionY != 0 &&
        (resolution.x != m_sceneCameraResolutionX || resolution.y != m_sceneCameraResolutionY))
    {
        m_sceneCamera->ResizeResolution(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorderTexture->ReSize(
            0, GL_R32F, GL_RED, GL_FLOAT, 0, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorderRenderBuffer->AllocateStorage(
            GL_DEPTH24_STENCIL8, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorder->SetResolution(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    }
    m_sceneCamera->Clear();
    m_sceneCameraEntityRecorder->Clear();
    if (m_lockCamera)
    {
        const float elapsedTime = Application::Time().CurrentTime() - m_transitionTimer;
        float a = 1.0f - glm::pow(1.0 - elapsedTime / m_transitionTime, 4.0f);
        if (elapsedTime >= m_transitionTime)
            a = 1.0f;
        m_sceneCameraRotation = glm::mix(m_previousRotation, m_targetRotation, a);
        m_sceneCameraPosition = glm::mix(m_previousPosition, m_targetPosition, a);
        if (a >= 1.0f)
        {
            m_lockCamera = false;
            m_sceneCameraRotation = m_targetRotation;
            m_sceneCameraPosition = m_targetPosition;
            Camera::ReverseAngle(m_targetRotation, m_sceneCameraPitchAngle, m_sceneCameraYawAngle);
        }
    }

    if (m_sceneCamera->m_requireRendering)
    {
        Camera::m_cameraInfoBlock.UpdateMatrices(m_sceneCamera, m_sceneCameraPosition, m_sceneCameraRotation);
        Camera::m_cameraInfoBlock.UploadMatrices(m_sceneCamera);
#pragma region For entity selection
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
        OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
        OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
        OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
        m_sceneCameraEntityRecorder->Bind();
        for (auto &i : renderLayer->m_deferredRenderInstances)
        {
            const auto &cameraComponent = i.first;
            renderLayer->DispatchRenderCommands(
                i.second,
                [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                    switch (renderCommand.m_meshType)
                    {
                    case RenderCommandMeshType::Default: {
                        auto &program = DefaultResources::m_sceneCameraEntityRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        DefaultResources::m_sceneCameraEntityRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        renderCommand.m_mesh.lock()->Draw();
                        break;
                    }
                    case RenderCommandMeshType::Skinned: {
                        auto &program = DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        renderCommand.m_boneMatrices.lock()->UploadBones(renderCommand.m_skinnedMesh.lock());
                        DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        renderCommand.m_skinnedMesh.lock()->Draw();
                        break;
                    }
                    }
                },
                false);
        }
        for (auto &i : renderLayer->m_deferredInstancedRenderInstances)
        {
            renderLayer->DispatchRenderCommands(
                i.second,
                [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                    switch (renderCommand.m_meshType)
                    {
                    case RenderCommandMeshType::Default: {
                        if (renderCommand.m_matrices.expired())
                            break;
                        auto &program = DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetFloat4x4(
                            "model", renderCommand.m_globalTransform.m_value);
                        renderCommand.m_mesh.lock()->DrawInstanced(renderCommand.m_matrices.lock());
                        break;
                    }
                    }
                },
                false);
        }
        for (auto &i : renderLayer->m_forwardRenderInstances)
        {
            renderLayer->DispatchRenderCommands(
                i.second,
                [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                    switch (renderCommand.m_meshType)
                    {
                    case RenderCommandMeshType::Default: {
                        auto &program = DefaultResources::m_sceneCameraEntityRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        DefaultResources::m_sceneCameraEntityRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        renderCommand.m_mesh.lock()->Draw();
                        break;
                    }
                    case RenderCommandMeshType::Skinned: {
                        auto &program = DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        renderCommand.m_boneMatrices.lock()->UploadBones(renderCommand.m_skinnedMesh.lock());
                        DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        renderCommand.m_skinnedMesh.lock()->Draw();
                        break;
                    }
                    }
                },
                false);
        }
        for (auto &i : renderLayer->m_forwardInstancedRenderInstances)
        {
            renderLayer->DispatchRenderCommands(
                i.second,
                [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                    switch (renderCommand.m_meshType)
                    {
                    case RenderCommandMeshType::Default: {
                        if (renderCommand.m_matrices.expired())
                            break;
                        auto &program = DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
                        program->Bind();
                        program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                        DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetInt(
                            "EntityIndex", renderCommand.m_owner.GetIndex());
                        DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetFloat4x4(
                            "model", renderCommand.m_globalTransform.m_value);
                        renderCommand.m_mesh.lock()->DrawInstanced(renderCommand.m_matrices.lock());
                        break;
                    }
                    }
                },
                false);
        }
#pragma endregion
        renderLayer->ApplyShadowMapSettings();
        renderLayer->ApplyEnvironmentalSettings(m_sceneCamera);
        GlobalTransform sceneCameraGT;
        sceneCameraGT.SetValue(m_sceneCameraPosition, m_sceneCameraRotation, glm::vec3(1.0f));
        renderLayer->RenderToCamera(m_sceneCamera, sceneCameraGT);
    }
    else
    {
    }
    ProfilerLayer::EndEvent("RenderToSceneCamera");
}

void EditorLayer::DrawEntityNode(const Entity &entity, const unsigned &hierarchyLevel)
{
    std::string title = std::to_string(entity.GetIndex()) + ": ";
    title += entity.GetName();
    const bool enabled = entity.IsEnabled();
    if (enabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({1, 1, 1, 1}));
    }
    const int index = m_selectedEntityHierarchyList.size() - hierarchyLevel - 1;
    if (!m_selectedEntityHierarchyList.empty() && index >= 0 && index < m_selectedEntityHierarchyList.size() &&
        m_selectedEntityHierarchyList[index] == entity)
    {
        ImGui::SetNextItemOpen(true);
    }
    const bool opened = ImGui::TreeNodeEx(
        title.c_str(),
        ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_NoAutoOpenOnLog |
            (m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed : ImGuiTreeNodeFlags_FramePadding));
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("Entity", &entity, sizeof(Entity));
        ImGui::TextColored(ImVec4(0, 0, 1, 1), title.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(Entity));
            Entities::SetParent(Entities::GetCurrentScene(), *static_cast<Entity *>(payload->Data), entity, true);
        }
        ImGui::EndDragDropTarget();
    }
    if (enabled)
    {
        ImGui::PopStyleColor();
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        SetSelectedEntity(entity, false);
    }
    const bool deleted = DrawEntityMenu(enabled, entity);
    if (opened && !deleted)
    {
        ImGui::TreePush();
        Entities::ForEachChild(
            Entities::GetCurrentScene(), entity, [=](const std::shared_ptr<Scene> &scene, Entity child) {
                DrawEntityNode(child, hierarchyLevel + 1);
            });
        ImGui::TreePop();
    }
}
void EditorLayer::OnInspect()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("System Inspector", &m_enableSystemInspector);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_leftMouseButtonHold && !Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, Windows::GetWindow()))
    {
        m_leftMouseButtonHold = false;
    }
    if (m_rightMouseButtonHold && !Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, Windows::GetWindow()))
    {
        m_rightMouseButtonHold = false;
        m_startMouse = false;
    }
    auto scene = Entities::GetCurrentScene();
    if (scene && m_configFlags & EntityEditorSystem_EnableEntityHierarchy)
    {
        ImGui::Begin("Entity Explorer");
        if (ImGui::BeginPopupContextWindow("NewEntityPopup"))
        {
            if (ImGui::Button("Create new entity"))
            {
                auto newEntity = Entities::CreateEntity(Entities::GetCurrentScene(), m_basicEntityArchetype);
            }
            ImGui::EndPopup();
        }
        ImGui::Combo(
            "Display mode", &m_selectedHierarchyDisplayMode, HierarchyDisplayMode, IM_ARRAYSIZE(HierarchyDisplayMode));
        std::string title = Entities::GetCurrentScene()->m_name;
        if (!Entities::GetCurrentScene()->m_saved)
        {
            title += " *";
        }
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
        {
            const auto type = scene->GetTypeName();
            const std::string tag = "##" + type + (scene ? std::to_string(scene->GetHandle()) : "");
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload(type.c_str(), &scene->m_handle, sizeof(Handle));
                ImGui::TextColored(ImVec4(0, 0, 1, 1), (scene->m_name + tag).c_str());
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginPopupContextItem(tag.c_str()))
            {
                if (ImGui::BeginMenu(("Rename" + tag).c_str()))
                {
                    static char newName[256];
                    ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                    if (ImGui::Button(("Confirm" + tag).c_str()))
                    {
                        scene->SetName(std::string(newName));
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                Editor::GetInstance().m_inspectingAsset = scene;
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(Entity));
                    Entity payload_n = *static_cast<Entity *>(payload->Data);
                    auto parent = Entities::GetParent(Entities::GetCurrentScene(), payload_n);
                    if (!parent.IsNull())
                        Entities::RemoveChild(Entities::GetCurrentScene(), payload_n, parent);
                }
                ImGui::EndDragDropTarget();
            }
            if (m_selectedHierarchyDisplayMode == 0)
            {
                Entities::UnsafeForEachEntityStorage(
                    Entities::GetCurrentScene(),
                    [&](int i, const std::string &name, const DataComponentStorage &storage) {
                        if (i == 0)
                            return;
                        ImGui::Separator();
                        const std::string title = std::to_string(i) + ". " + name;
                        if (ImGui::TreeNode(title.c_str()))
                        {
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
                            for (int j = 0; j < storage.m_entityAliveCount; j++)
                            {
                                Entity entity = storage.m_chunkArray.m_entities.at(j);
                                std::string title = std::to_string(entity.GetIndex()) + ": ";
                                title += entity.GetName();
                                const bool enabled = entity.IsEnabled();
                                if (enabled)
                                {
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({1, 1, 1, 1}));
                                }
                                ImGui::TreeNodeEx(
                                    title.c_str(),
                                    ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf |
                                        ImGuiTreeNodeFlags_NoAutoOpenOnLog |
                                        (m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed
                                                                    : ImGuiTreeNodeFlags_FramePadding));
                                if (enabled)
                                {
                                    ImGui::PopStyleColor();
                                }
                                DrawEntityMenu(enabled, entity);
                                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
                                {
                                    SetSelectedEntity(entity, false);
                                }
                            }
                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                            ImGui::TreePop();
                        }
                    });
            }
            else if (m_selectedHierarchyDisplayMode == 1)
            {
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
                Entities::ForAllEntities(Entities::GetCurrentScene(), [&](int i, Entity entity) {
                    if (Entities::GetParent(Entities::GetCurrentScene(), entity).IsNull())
                        DrawEntityNode(entity, 0);
                });
                m_selectedEntityHierarchyList.clear();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
    }
    if (scene && m_configFlags & EntityEditorSystem_EnableEntityInspector)
    {
        ImGui::Begin("Entity Inspector");
        if (m_selectedEntity.IsValid())
        {
            std::string title = std::to_string(m_selectedEntity.GetIndex()) + ": ";
            title += m_selectedEntity.GetName();
            bool enabled = m_selectedEntity.IsEnabled();
            if (ImGui::Checkbox((title + "##EnabledCheckbox").c_str(), &enabled))
            {
                if (m_selectedEntity.IsEnabled() != enabled)
                {
                    m_selectedEntity.SetEnabled(enabled);
                }
            }
            ImGui::SameLine();
            bool isStatic = m_selectedEntity.IsStatic();
            if (ImGui::Checkbox("Static##StaticCheckbox", &isStatic))
            {
                if (m_selectedEntity.IsStatic() != isStatic)
                {
                    m_selectedEntity.SetStatic(isStatic);
                }
            }

            bool deleted = DrawEntityMenu(m_selectedEntity.IsEnabled(), m_selectedEntity);
            if (!deleted)
            {
                if (ImGui::CollapsingHeader("Data components", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::BeginPopupContextItem("DataComponentInspectorPopup"))
                    {
                        ImGui::Text("Add data component: ");
                        ImGui::Separator();
                        for (auto &i : Editor::GetInstance().m_componentDataMenuList)
                        {
                            i.second(m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }
                    bool skip = false;
                    int i = 0;
                    Entities::UnsafeForEachDataComponent(
                        Entities::GetCurrentScene(), m_selectedEntity, [&](DataComponentType type, void *data) {
                            if (skip)
                                return;
                            std::string info = type.m_name;
                            info += " Size: " + std::to_string(type.m_size);
                            ImGui::Text(info.c_str());
                            ImGui::PushID(i);
                            if (ImGui::BeginPopupContextItem(("DataComponentDeletePopup" + std::to_string(i)).c_str()))
                            {
                                if (ImGui::Button("Remove"))
                                {
                                    skip = true;
                                    Entities::RemoveDataComponent(
                                        Entities::GetCurrentScene(), m_selectedEntity, type.m_typeId);
                                }
                                ImGui::EndPopup();
                            }
                            ImGui::PopID();
                            InspectComponentData(
                                m_selectedEntity,
                                static_cast<IDataComponent *>(data),
                                type,
                                Entities::GetParent(Entities::GetCurrentScene(), m_selectedEntity).IsNull());
                            ImGui::Separator();
                            i++;
                        });
                }

                if (ImGui::CollapsingHeader("Private components", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::BeginPopupContextItem("PrivateComponentInspectorPopup"))
                    {
                        ImGui::Text("Add private component: ");
                        ImGui::Separator();
                        for (auto &i : Editor::GetInstance().m_privateComponentMenuList)
                        {
                            i.second(m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }

                    int i = 0;
                    bool skip = false;
                    Entities::ForEachPrivateComponent(
                        Entities::GetCurrentScene(), m_selectedEntity, [&](PrivateComponentElement &data) {
                            if (skip)
                                return;
                            ImGui::Checkbox(
                                data.m_privateComponentData->GetTypeName().c_str(),
                                &data.m_privateComponentData->m_enabled);
                            Editor::DraggablePrivateComponent(data.m_privateComponentData);
                            const std::string tag = "##" + data.m_privateComponentData->GetTypeName() +
                                                    std::to_string(data.m_privateComponentData->GetHandle());
                            if (ImGui::BeginPopupContextItem(tag.c_str()))
                            {
                                if (ImGui::Button(("Remove" + tag).c_str()))
                                {
                                    skip = true;
                                    Entities::RemovePrivateComponent(
                                        Entities::GetCurrentScene(), m_selectedEntity, data.m_typeId);
                                }
                                ImGui::EndPopup();
                            }
                            if (!skip)
                            {
                                if (ImGui::TreeNodeEx(
                                        ("Component Settings##" + std::to_string(i)).c_str(),
                                        ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    data.m_privateComponentData->OnInspect();
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::Separator();
                            i++;
                        });
                }
            }
        }
        else
        {
            m_selectedEntity = Entity();
        }
        ImGui::End();
    }
    if (scene && Inputs::GetKeyInternal(GLFW_KEY_DELETE, Windows::GetWindow()))
    {
        if (m_selectedEntity.IsValid())
        {
            Entities::DeleteEntity(Entities::GetCurrentScene(), m_selectedEntity);
        }
    }
    MainCameraWindow();
    SceneCameraWindow();
    auto &projectManager = ProjectManager::GetInstance();

    if (ImGui::Begin("Project"))
    {
        if (projectManager.m_currentProject)
        {
            if (ImGui::BeginDragDropTarget())
            {
                auto &assetManager = AssetManager::GetInstance();
                for (const auto &i : assetManager.m_defaultExtensions)
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(i.first.c_str()))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(Handle));
                        Handle payload_n = *(Handle *)payload->Data;
                        AssetRef assetRef;
                        assetRef.m_assetHandle = payload_n;
                        assetRef.Update();
                        // If current folder doesn't contain file with same name
                        auto tempFileName = assetRef.m_value->m_name;
                        auto fileExtension = assetManager.m_defaultExtensions[assetRef.m_assetTypeName].at(0);
                        auto folderPath = projectManager.m_projectPath.parent_path() /
                                          projectManager.m_currentFocusedFolder->m_relativePath;
                        std::filesystem::path filePath = folderPath / (tempFileName + fileExtension);
                        if (assetRef.m_value->GetPath() !=
                            projectManager.m_currentFocusedFolder->m_relativePath / (tempFileName + fileExtension))
                        {
                            int index = 0;
                            while (std::filesystem::exists(filePath))
                            {
                                index++;
                                filePath =
                                    folderPath / (tempFileName + "(" + std::to_string(index) + ")" + fileExtension);
                            }
                            assetRef.m_value->SetPathAndSave(ProjectManager::GetRelativePath(filePath));
                        }
                    }
                }
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(Entity));
                    auto prefab = AssetManager::CreateAsset<Prefab>();
                    prefab->FromEntity(*static_cast<Entity *>(payload->Data));
                    // If current folder doesn't contain file with same name
                    auto tempFileName = prefab->m_name;
                    auto fileExtension = assetManager.m_defaultExtensions["Prefab"].at(0);
                    auto folderPath = projectManager.m_projectPath.parent_path() /
                                      projectManager.m_currentFocusedFolder->m_relativePath;
                    std::filesystem::path filePath = folderPath / (tempFileName + fileExtension);
                    int index = 0;
                    while (std::filesystem::exists(filePath))
                    {
                        index++;
                        filePath = folderPath / (tempFileName + "(" + std::to_string(index) + ")" + fileExtension);
                    }
                    prefab->SetPathAndSave(ProjectManager::GetRelativePath(filePath));
                }

                ImGui::EndDragDropTarget();
            }
            static glm::vec2 thumbnailSizePadding = {96.0f, 8.0f};
            float cellSize = thumbnailSizePadding.x + thumbnailSizePadding.y;
            static float size1 = 200;
            static float size2 = 200;
            static float h = 100;
            auto avail = ImGui::GetContentRegionAvail();
            size2 = glm::max(avail.x - size1, cellSize + 8.0f);
            size1 = glm::max(avail.x - size2, 32.0f);
            h = avail.y;
            ImGui::Splitter(true, 8.0, size1, size2, 32.0f, cellSize + 8.0f, h);
            ImGui::BeginChild("1", ImVec2(size1, h), true);
            FolderHierarchyHelper(ProjectManager::GetInstance().m_currentProject->m_projectFolder);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("2", ImVec2(size2 - 5.0f, h), true);
            if (ImGui::ImageButton(
                    (ImTextureID)Editor::AssetIcons()["RefreshButton"]->UnsafeGetGLTexture()->Id(),
                    {16, 16},
                    {0, 1},
                    {1, 0}))
            {
                ProjectManager::UpdateFolderMetadata(projectManager.m_currentFocusedFolder, false);
            }

            if (projectManager.m_currentFocusedFolder != projectManager.m_currentProject->m_projectFolder)
            {
                ImGui::SameLine();
                if (ImGui::ImageButton(
                        (ImTextureID)Editor::AssetIcons()["BackButton"]->UnsafeGetGLTexture()->Id(),
                        {16, 16},
                        {0, 1},
                        {1, 0}))
                {
                    projectManager.m_currentFocusedFolder = projectManager.m_currentFocusedFolder->m_parent.lock();
                }
            }
            ImGui::Separator();

            bool updated = false;
            auto &assetManager = AssetManager::GetInstance();
            if (ImGui::BeginPopupContextWindow("NewAssetPopup"))
            {
                if (ImGui::Button("New folder..."))
                {
                    auto newPath = ProjectManager::GenerateNewPath(
                        (projectManager.m_projectPath.parent_path() /
                         projectManager.m_currentFocusedFolder->m_relativePath / "New Folder")
                            .string(),
                        "");
                    std::filesystem::create_directories(newPath);
                    ProjectManager::ScanProjectFolder(false);
                }
                if (ImGui::BeginMenu("Create new asset..."))
                {
                    for (auto &i : assetManager.m_defaultExtensions)
                    {
                        if (ImGui::Button(i.first.c_str()))
                        {
                            std::string newFileName = "New " + i.first;
                            auto newHandle = Handle();
                            auto newAsset = AssetManager::UnsafeCreateAsset(i.first, newHandle, newFileName);
                            auto newPath = ProjectManager::GenerateNewPath(
                                (projectManager.m_projectPath.parent_path() /
                                 projectManager.m_currentFocusedFolder->m_relativePath / newFileName)
                                    .string(),
                                AssetManager::GetExtension(i.first)[0]);
                            newAsset->SetPathAndSave(ProjectManager::GetRelativePath(newPath));
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            float panelWidth = ImGui::GetContentRegionAvailWidth();
            int columnCount = glm::max(1, (int)(panelWidth / cellSize));
            ImGui::Columns(columnCount, 0, false);
            auto &editorManager = Editor::GetInstance();
            if (!updated)
            {
                for (auto &i : projectManager.m_currentFocusedFolder->m_children)
                {
                    ImGui::Image(
                        (ImTextureID)Editor::AssetIcons()["Folder"]->UnsafeGetGLTexture()->Id(),
                        {thumbnailSizePadding.x, thumbnailSizePadding.x},
                        {0, 1},
                        {1, 0});
                    bool itemHovered = false;
                    if (ImGui::IsItemHovered())
                    {
                        itemHovered = true;
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            projectManager.m_currentFocusedFolder = i.second;
                            updated = true;
                            break;
                        }
                    }
                    const std::string tag = "##Folder" + i.first;
                    if (ImGui::BeginPopupContextItem(tag.c_str()))
                    {
                        if (ImGui::BeginMenu(("Rename" + tag).c_str()))
                        {
                            static char newName[256] = {0};
                            ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                            if (ImGui::Button(("Confirm" + tag).c_str()))
                            {
                                i.second->Rename(std::string(newName));
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::Button(("Remove" + tag).c_str()))
                        {
                            std::filesystem::remove_all(
                                projectManager.m_projectPath.parent_path() / i.second->m_relativePath);
                            ProjectManager::ScanProjectFolder(false);
                            updated = true;
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            break;
                        }
                        ImGui::EndPopup();
                    }
                    if (itemHovered)
                        ImGui::PushStyleColor(ImGuiCol_Text, {1, 1, 0, 1});
                    ImGui::TextWrapped(i.second->m_name.c_str());
                    if (itemHovered)
                        ImGui::PopStyleColor(1);
                    ImGui::NextColumn();
                }
            }
            if (!updated)
            {
                for (auto &i : projectManager.m_currentFocusedFolder->m_folderMetadata.m_fileRecords)
                {
                    ImTextureID textureId = 0;
                    auto fileName = i.second.m_relativeFilePath.filename();
                    if (fileName.string() == ".ueassetregistry" || fileName.string() == ".uemetadata" ||
                        fileName.string() == ".ueproj" || fileName.extension().string() == ".ueproj")
                        continue;
                    if (fileName.extension().string() == ".ueproj")
                    {
                        textureId = (ImTextureID)Editor::AssetIcons()["Project"]->UnsafeGetGLTexture()->Id();
                    }
                    else
                    {
                        auto iconSearch = Editor::AssetIcons().find(i.second.m_typeName);
                        if (iconSearch != Editor::AssetIcons().end())
                        {
                            textureId = (ImTextureID)iconSearch->second->UnsafeGetGLTexture()->Id();
                        }
                        else
                        {
                            textureId = (ImTextureID)Editor::AssetIcons()["Binary"]->UnsafeGetGLTexture()->Id();
                        }
                    }
                    static std::shared_ptr<IAsset> asset;
                    bool itemFocused = false;
                    if (asset && asset->m_handle.GetValue() == i.first.GetValue())
                    {
                        itemFocused = true;
                    }
                    ImGui::Image(textureId, {thumbnailSizePadding.x, thumbnailSizePadding.x}, {0, 1}, {1, 0});

                    bool itemHovered = false;
                    if (ImGui::IsItemHovered())
                    {
                        itemHovered = true;
                        if (ImGui::IsMouseDoubleClicked(0) && AssetManager::IsAsset(i.second.m_typeName))
                        {
                            // If it's an asset then inspect.
                            asset = AssetManager::Get(i.first);
                            if (asset)
                                Editor::GetInstance().m_inspectingAsset = asset;
                        }
                    }
                    const std::string tag = "##" + i.second.m_typeName + std::to_string(i.first.GetValue());
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        ImGui::SetDragDropPayload(i.second.m_typeName.c_str(), &i.first, sizeof(Handle));
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), (i.second.m_name + tag).c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginPopupContextItem(tag.c_str()))
                    {
                        if (ImGui::BeginMenu(("Rename" + tag).c_str()))
                        {
                            static char newName[256] = {0};
                            ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                            if (ImGui::Button(("Confirm" + tag).c_str()))
                            {
                                auto originalName = i.second.m_name;
                                auto newFileName = std::string(newName);
                                auto newPath = i.second.m_relativeFilePath;
                                newPath.replace_filename(newFileName + newPath.extension().string());
                                std::filesystem::rename(
                                    projectManager.m_projectPath.parent_path() / i.second.m_relativeFilePath,
                                    projectManager.m_projectPath.parent_path() / newPath);
                                // Asset path
                                auto search1 = assetManager.m_assets.find(i.first);
                                if (search1 != assetManager.m_assets.end())
                                {
                                    if (!search1->second.expired())
                                    {
                                        auto searchAsset = search1->second.lock();
                                        searchAsset->SetPath(newPath);
                                    }
                                }

                                // FolderMetadata
                                auto originalHandle = projectManager.m_currentFocusedFolder->m_folderMetadata
                                                          .m_fileMap[i.second.m_relativeFilePath.string()];
                                projectManager.m_currentFocusedFolder->m_folderMetadata.m_fileMap.erase(
                                    i.second.m_relativeFilePath.string());
                                i.second.m_relativeFilePath = newPath;
                                i.second.m_fileName = newPath.filename().string();
                                projectManager.m_currentFocusedFolder->m_folderMetadata.m_fileMap[newPath.string()] =
                                    originalHandle;
                                projectManager.m_currentFocusedFolder->m_folderMetadata.Save(
                                    projectManager.m_projectPath.parent_path() /
                                    projectManager.m_currentFocusedFolder->m_relativePath / ".uemetadata");
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::Button(("Remove" + tag).c_str()))
                        {
                            asset = AssetManager::Get(i.first);
                            if (asset)
                            {
                                asset->m_projectRelativePath.clear();
                            }
                            std::filesystem::remove(
                                projectManager.m_projectPath.parent_path() / i.second.m_relativeFilePath);
                            projectManager.m_assetRegistry.RemoveFile(i.first);
                            ProjectManager::UpdateFolderMetadata(projectManager.m_currentFocusedFolder, false);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            break;
                        }
                        ImGui::EndPopup();
                    }
                    if (itemFocused)
                        ImGui::PushStyleColor(ImGuiCol_Text, {1, 0, 0, 1});
                    else if (itemHovered)
                        ImGui::PushStyleColor(ImGuiCol_Text, {1, 1, 0, 1});
                    ImGui::TextWrapped(fileName.string().c_str());
                    if (itemFocused || itemHovered)
                        ImGui::PopStyleColor(1);
                    ImGui::NextColumn();
                }
            }

            ImGui::Columns(1);
            // ImGui::SliderFloat("Thumbnail Size", &thumbnailSizePadding.x, 16, 512);
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("No project loaded!");
        }
    }
    ImGui::End();
    if (scene && m_enableSystemInspector)
    {
        if (ImGui::Begin("System Inspector"))
        {
            if (ImGui::BeginPopupContextWindow("SystemInspectorPopup"))
            {
                ImGui::Text("Add system: ");
                ImGui::Separator();
                static float rank = 0.0f;
                ImGui::DragFloat("Rank", &rank, 1.0f, 0.0f, 999.0f);
                for (auto &i : Editor::GetInstance().m_systemMenuList)
                {
                    i.second(rank);
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }
            for (auto &i : Entities::GetCurrentScene()->m_systems)
            {
                if (ImGui::CollapsingHeader(i.second->GetTypeName().c_str()))
                {
                    bool enabled = i.second->Enabled();
                    if (ImGui::Checkbox("Enabled", &enabled))
                    {
                        if (i.second->Enabled() != enabled)
                        {
                            if (enabled)
                            {
                                i.second->Enable();
                            }
                            else
                            {
                                i.second->Disable();
                            }
                        }
                    }
                    i.second->OnInspect();
                }
            }
        }
        ImGui::End();
    }
    if (!Editor::GetInstance().m_inspectingAsset.expired())
    {
        if (ImGui::Begin("Asset Inspector"))
        {
            if (!Editor::GetInstance().m_inspectingAsset.expired())
            {
                auto asset = Editor::GetInstance().m_inspectingAsset.lock();
                ImGui::Text("Type:");
                ImGui::SameLine();
                ImGui::Text(asset->GetTypeName().c_str());
                ImGui::Separator();
                ImGui::Text("Name:");
                ImGui::SameLine();
                ImGui::Button(asset->m_name.c_str());
                Editor::DraggableAsset(asset);
                if (!asset->GetPath().empty())
                {
                    if (ImGui::Button("Save"))
                    {
                        asset->Save();
                    }
                    ImGui::SameLine();
                    FileUtils::SaveFile(
                        "Reset path & save",
                        asset->GetTypeName(),
                        AssetManager::GetInstance().m_defaultExtensions[asset->GetTypeName()],
                        [&](const std::filesystem::path &path) {
                            asset->SetPathAndSave(ProjectManager::GetRelativePath(path));
                        },
                        true);
                }
                else
                {
                    ImGui::Text("Temporary asset");
                    ImGui::SameLine();
                    FileUtils::SaveFile(
                        "Allocate path & save",
                        asset->GetTypeName(),
                        AssetManager::GetInstance().m_defaultExtensions[asset->GetTypeName()],
                        [&](const std::filesystem::path &path) {
                            asset->SetPathAndSave(ProjectManager::GetRelativePath(path));
                        },
                        true);
                }

                FileUtils::SaveFile(
                    "Export...",
                    asset->GetTypeName(),
                    AssetManager::GetInstance().m_defaultExtensions[asset->GetTypeName()],
                    [&](const std::filesystem::path &path) { asset->Export(path); },
                    false);
                ImGui::SameLine();
                FileUtils::OpenFile(
                    "Import...",
                    asset->GetTypeName(),
                    AssetManager::GetInstance().m_defaultExtensions[asset->GetTypeName()],
                    [&](const std::filesystem::path &path) { asset->Import(path); },
                    false);

                ImGui::Separator();
                asset->OnInspect();
            }
            else
            {
                ImGui::Text("None");
            }
        }
        ImGui::End();
    }
}

void EditorLayer::FolderHierarchyHelper(const std::shared_ptr<Folder> &folder)
{
    auto &focusFolder = ProjectManager::GetInstance().m_currentFocusedFolder;
    const bool opened = ImGui::TreeNodeEx(
        folder->m_name.c_str(),
        ImGuiTreeNodeFlags_OpenOnArrow |
            (folder == focusFolder ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        focusFolder = folder;
    }
    if (opened)
    {
        for (const auto &i : folder->m_children)
        {
            FolderHierarchyHelper(i.second);
        }
        ImGui::TreePop();
    }
}

void EditorLayer::SetSelectedEntity(const Entity &entity, bool openMenu)
{
    if (entity == m_selectedEntity)
        return;
    m_selectedEntityHierarchyList.clear();
    if (entity.IsNull())
    {
        m_selectedEntity = Entity();
        return;
    }
    if (!entity.IsValid())
        return;
    m_selectedEntity = entity;
    if (!openMenu)
        return;
    auto walker = entity;
    while (!walker.IsNull())
    {
        m_selectedEntityHierarchyList.push_back(walker);
        walker = Entities::GetParent(Entities::GetCurrentScene(), walker);
    }
}

bool EditorLayer::MainCameraWindowFocused()
{
    return m_mainCameraWindowFocused || m_mainCameraFocusOverride;
}
bool EditorLayer::SceneCameraWindowFocused()
{
    return m_sceneCameraWindowFocused || m_sceneCameraFocusOverride;
}
void EditorLayer::SceneCameraWindow()
{
#pragma region Scene Window
    ImVec2 viewPortSize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Scene"))
    {
        // Using a Child allow to fill all the space of the window.
        // It also allows customization
        if (ImGui::BeginChild("SceneCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5, 5});
                if (ImGui::BeginMenu("Settings"))
                {
                    if (ImGui::Button("Reset camera"))
                    {
                        MoveCamera(m_defaultSceneCameraRotation, m_defaultSceneCameraPosition);
                    }
                    if (ImGui::Button("Set default"))
                    {
                        m_defaultSceneCameraPosition = m_sceneCameraPosition;
                        m_defaultSceneCameraRotation = m_sceneCameraRotation;
                    }
                    ImGui::SliderFloat("FOV", &m_sceneCamera->m_fov, 30.0f, 120.0f);
                    ImGui::DragFloat3("Position", &m_sceneCameraPosition.x, 0.1f);
                    ImGui::DragFloat("Speed", &m_velocity, 0.1f);
                    ImGui::DragFloat("Sensitivity", &m_sensitivity, 0.1f);
                    ImGui::EndMenu();
                }
                ImGui::PopStyleVar();
                ImGui::EndMenuBar();
            }
            viewPortSize = ImGui::GetWindowSize();
            viewPortSize.y -= 20;
            if (viewPortSize.y < 0)
                viewPortSize.y = 0;
            m_sceneCameraResolutionX = viewPortSize.x;
            m_sceneCameraResolutionY = viewPortSize.y;
            if (m_sceneCamera && m_sceneCamera->m_rendered)
            {
                // Because I use the texture from OpenGL, I need to invert the V from the UV.
                ImGui::Image(
                    (ImTextureID)m_sceneCamera->GetTexture()->UnsafeGetGLTexture()->Id(),
                    viewPortSize,
                    ImVec2(0, 1),
                    ImVec2(1, 0));
                CameraWindowDragAndDrop();
            }
            else
            {
                ImGui::Text("No active main camera!");
            }
            glm::vec2 mousePosition = glm::vec2(FLT_MAX, FLT_MIN);
            if (m_sceneCameraWindowFocused)
            {
                bool valid = Inputs::GetMousePositionInternal(ImGui::GetCurrentWindowRead(), mousePosition);
                float xOffset = 0;
                float yOffset = 0;
                if (valid)
                {
                    if (!m_startMouse)
                    {
                        m_lastX = mousePosition.x;
                        m_lastY = mousePosition.y;
                        m_startMouse = true;
                    }
                    xOffset = mousePosition.x - m_lastX;
                    yOffset = -mousePosition.y + m_lastY;
                    m_lastX = mousePosition.x;
                    m_lastY = mousePosition.y;
#pragma region Scene Camera Controller
                    if (!m_rightMouseButtonHold &&
                        !(mousePosition.x > 0 || mousePosition.y < 0 || mousePosition.x < -viewPortSize.x ||
                          mousePosition.y > viewPortSize.y) &&
                        Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, Windows::GetWindow()))
                    {
                        m_rightMouseButtonHold = true;
                    }
                    if (m_rightMouseButtonHold && !m_lockCamera)
                    {
                        glm::vec3 front = m_sceneCameraRotation * glm::vec3(0, 0, -1);
                        glm::vec3 right = m_sceneCameraRotation * glm::vec3(1, 0, 0);
                        if (Inputs::GetKeyInternal(GLFW_KEY_W, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition +=
                                front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                        }
                        if (Inputs::GetKeyInternal(GLFW_KEY_S, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition -=
                                front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                        }
                        if (Inputs::GetKeyInternal(GLFW_KEY_A, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition -=
                                right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                        }
                        if (Inputs::GetKeyInternal(GLFW_KEY_D, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition +=
                                right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                        }
                        if (Inputs::GetKeyInternal(GLFW_KEY_LEFT_SHIFT, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition.y += m_velocity * static_cast<float>(Application::Time().DeltaTime());
                        }
                        if (Inputs::GetKeyInternal(GLFW_KEY_LEFT_CONTROL, Windows::GetWindow()))
                        {
                            m_sceneCameraPosition.y -= m_velocity * static_cast<float>(Application::Time().DeltaTime());
                        }
                        if (xOffset != 0.0f || yOffset != 0.0f)
                        {
                            m_sceneCameraYawAngle += xOffset * m_sensitivity;
                            m_sceneCameraPitchAngle += yOffset * m_sensitivity;
                            if (m_sceneCameraPitchAngle > 89.0f)
                                m_sceneCameraPitchAngle = 89.0f;
                            if (m_sceneCameraPitchAngle < -89.0f)
                                m_sceneCameraPitchAngle = -89.0f;

                            m_sceneCameraRotation =
                                Camera::ProcessMouseMovement(m_sceneCameraYawAngle, m_sceneCameraPitchAngle, false);
                        }
                    }
#pragma endregion
                }
            }
#pragma region Gizmos and Entity Selection
            bool mouseSelectEntity = true;
            if (m_selectedEntity.IsValid())
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewPortSize.x, viewPortSize.y);
                glm::mat4 cameraView =
                    glm::inverse(glm::translate(m_sceneCameraPosition) * glm::mat4_cast(m_sceneCameraRotation));
                glm::mat4 cameraProjection = m_sceneCamera->GetProjection();
                const auto op = m_localPositionSelected   ? ImGuizmo::OPERATION::TRANSLATE
                                : m_localRotationSelected ? ImGuizmo::OPERATION::ROTATE
                                                          : ImGuizmo::OPERATION::SCALE;

                auto transform = m_selectedEntity.GetDataComponent<Transform>();
                GlobalTransform parentGlobalTransform;
                Entity parentEntity = Entities::GetParent(Entities::GetCurrentScene(), m_selectedEntity);
                if (!parentEntity.IsNull())
                {
                    parentGlobalTransform = Entities::GetParent(Entities::GetCurrentScene(), m_selectedEntity)
                                                .GetDataComponent<GlobalTransform>();
                }
                auto globalTransform = m_selectedEntity.GetDataComponent<GlobalTransform>();
                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    op,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(globalTransform.m_value));
                if (ImGuizmo::IsUsing())
                {
                    transform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
                    m_selectedEntity.SetDataComponent(transform);
                    transform.Decompose(
                        m_previouslyStoredPosition, m_previouslyStoredRotation, m_previouslyStoredScale);
                    mouseSelectEntity = false;
                }
            }
            if (m_sceneCameraWindowFocused && mouseSelectEntity)
            {
                if (!m_leftMouseButtonHold &&
                    !(mousePosition.x > 0 || mousePosition.y < 0 || mousePosition.x < -viewPortSize.x ||
                      mousePosition.y > viewPortSize.y) &&
                    Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, Windows::GetWindow()))
                {
                    Entity focusedEntity = MouseEntitySelection(mousePosition);
                    if (focusedEntity == Entity())
                    {
                        SetSelectedEntity(Entity());
                    }
                    else
                    {
                        Entity walker = focusedEntity;
                        bool found = false;
                        while (!walker.IsNull())
                        {
                            if (walker == m_selectedEntity)
                            {
                                found = true;
                                break;
                            }
                            walker = Entities::GetParent(Entities::GetCurrentScene(), walker);
                        }
                        if (found)
                        {
                            walker = Entities::GetParent(Entities::GetCurrentScene(), walker);
                            if (walker.IsNull())
                            {
                                SetSelectedEntity(focusedEntity);
                            }
                            else
                            {
                                SetSelectedEntity(walker);
                            }
                        }
                        else
                        {
                            SetSelectedEntity(focusedEntity);
                        }
                    }
                    m_leftMouseButtonHold = true;
                }
            }
            HighLightEntity(m_selectedEntity, glm::vec4(1.0, 0.5, 0.0, 0.8));
#pragma endregion
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        {
            if (!m_sceneCameraWindowFocused)
            {
                m_rightMouseButtonHold = true;
                m_leftMouseButtonHold = true;
            }
            m_sceneCameraWindowFocused = true;
        }
        else
        {
            m_sceneCameraWindowFocused = false;
        }
        ImGui::EndChild();
    }
    else
    {
        m_sceneCameraWindowFocused = false;
    }
    m_sceneCamera->SetRequireRendering(
        !(ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed));
    ImGui::End();
    ImGui::PopStyleVar();

#pragma endregion
}
void EditorLayer::MainCameraWindow()
{
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;

    auto scene = Entities::GetCurrentScene();
#pragma region Window
    ImVec2 viewPortSize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Camera"))
    {
        static int corner = 1;
        // Using a Child allow to fill all the space of the window.
        // It also allows customization
        if (ImGui::BeginChild("MainCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5, 5});
                if (ImGui::BeginMenu("Settings"))
                {
#pragma region Menu
                    ImGui::Checkbox("Display info", &renderLayer->m_enableInfoWindow);
#pragma endregion
                    ImGui::EndMenu();
                }
                ImGui::PopStyleVar();
                ImGui::EndMenuBar();
            }
            viewPortSize = ImGui::GetWindowSize();
            viewPortSize.y -= 20;
            if (viewPortSize.y < 0)
                viewPortSize.y = 0;
            renderLayer->m_mainCameraResolutionX = viewPortSize.x;
            renderLayer->m_mainCameraResolutionY = viewPortSize.y;
            // UNIENGINE_LOG(std::to_string(viewPortSize.x) + ", " + std::to_string(viewPortSize.y));
            //  Get the size of the child (i.e. the whole draw size of the windows).
            ImVec2 overlayPos = ImGui::GetWindowPos();
            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            auto mainCamera = scene->m_mainCamera.Get<Camera>();
            if (mainCamera && mainCamera->m_rendered)
            {
                auto id = mainCamera->GetTexture()->UnsafeGetGLTexture()->Id();
                ImGui::Image((ImTextureID)id, viewPortSize, ImVec2(0, 1), ImVec2(1, 0));
                CameraWindowDragAndDrop();
            }
            else
            {
                ImGui::Text("No active main camera!");
            }

            ImVec2 window_pos = ImVec2(
                (corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
                (corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y) + 20);
            if (renderLayer->m_enableInfoWindow)
            {
                ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

                if (ImGui::BeginChild("Render Info", ImVec2(200, 75), false, window_flags))
                {
                    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
                    std::string trisstr = "";
                    if (renderLayer->m_triangles < 999)
                        trisstr += std::to_string(renderLayer->m_triangles);
                    else if (renderLayer->m_triangles < 999999)
                        trisstr += std::to_string((int)(renderLayer->m_triangles / 1000)) + "K";
                    else
                        trisstr += std::to_string((int)(renderLayer->m_triangles / 1000000)) + "M";
                    trisstr += " tris";
                    ImGui::Text(trisstr.c_str());
                    ImGui::Text("%d drawcall", renderLayer->m_drawCall);
                    ImGui::Separator();
                    if (ImGui::IsMousePosValid())
                    {
                        glm::vec2 pos;
                        Inputs::GetMousePositionInternal(ImGui::GetCurrentWindowRead(), pos);
                        ImGui::Text("Mouse Pos: (%.1f,%.1f)", pos.x, pos.y);
                    }
                    else
                    {
                        ImGui::Text("Mouse Pos: <invalid>");
                    }
                }
                ImGui::EndChild();
            }
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        {
            if (!m_mainCameraWindowFocused)
            {
                // m_rightMouseButtonHold = true;
                // m_leftMouseButtonHold = true;
            }
            m_mainCameraWindowFocused = true;
        }
        else
        {
            m_mainCameraWindowFocused = false;
        }

        ImGui::EndChild();
    }
    else
    {
        m_mainCameraWindowFocused = false;
    }
    auto mainCamera = scene->m_mainCamera.Get<Camera>();
    if (mainCamera)
    {
        mainCamera->SetRequireRendering(
            !ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed);
    }

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion
}
void EditorLayer::CameraWindowDragAndDrop()
{
    if (ImGui::BeginDragDropTarget())
    {
        if (!Application::IsPlaying())
        {
            const std::string sceneTypeHash = Serialization::GetSerializableTypeName<Scene>();
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(sceneTypeHash.c_str()))
            {
                IM_ASSERT(payload->DataSize == sizeof(Handle));
                Handle payload_n = *(Handle *)payload->Data;
                AssetRef assetRef;
                assetRef.m_assetHandle = payload_n;
                assetRef.Update();
                Application::GetInstance().m_scene = assetRef.Get<Scene>();
                Entities::Attach(Application::GetInstance().m_scene);
            }
        }
        const std::string modelTypeHash = Serialization::GetSerializableTypeName<Prefab>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(modelTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *(Handle *)payload->Data;
            AssetRef assetRef;
            assetRef.m_assetHandle = payload_n;
            assetRef.Update();
            EntityArchetype archetype = Entities::CreateEntityArchetype("Default", Transform(), GlobalTransform());
            std::dynamic_pointer_cast<Prefab>(assetRef.m_value)->ToEntity();
        }
        const std::string texture2DTypeHash = Serialization::GetSerializableTypeName<Texture2D>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(texture2DTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *(Handle *)payload->Data;
            AssetRef assetRef;
            assetRef.m_assetHandle = payload_n;
            assetRef.Update();
            EntityArchetype archetype = Entities::CreateEntityArchetype("Default", Transform(), GlobalTransform());
            AssetManager::ToEntity(archetype, std::dynamic_pointer_cast<Texture2D>(assetRef.m_value));
        }
        const std::string meshTypeHash = Serialization::GetSerializableTypeName<Mesh>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(meshTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *(Handle *)payload->Data;
            AssetRef assetRef;
            assetRef.m_assetHandle = payload_n;
            assetRef.Update();
            Entity entity = Entities::CreateEntity(Entities::GetCurrentScene(), "Mesh");
            auto meshRenderer = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
            meshRenderer->m_mesh.Set<Mesh>(std::dynamic_pointer_cast<Mesh>(assetRef.m_value));
            auto material = AssetManager::CreateAsset<Material>();
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer->m_material.Set<Material>(material);
        }

        const std::string environmentalMapTypeHash = Serialization::GetSerializableTypeName<EnvironmentalMap>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(environmentalMapTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *(Handle *)payload->Data;
            AssetRef assetRef;
            assetRef.m_assetHandle = payload_n;
            assetRef.Update();
            Entities::GetCurrentScene()->m_environmentSettings.m_environmentalMap =
                std::dynamic_pointer_cast<EnvironmentalMap>(assetRef.m_value);
        }

        const std::string cubeMapTypeHash = Serialization::GetSerializableTypeName<Cubemap>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(cubeMapTypeHash.c_str()))
        {
            auto mainCamera = Entities::GetCurrentScene()->m_mainCamera.Get<Camera>();
            if (mainCamera)
            {
                IM_ASSERT(payload->DataSize == sizeof(Handle));
                Handle payload_n = *(Handle *)payload->Data;
                AssetRef assetRef;
                assetRef.m_assetHandle = payload_n;
                assetRef.Update();
                mainCamera->m_skybox = std::dynamic_pointer_cast<Cubemap>(assetRef.m_value);
            }
        }
        ImGui::EndDragDropTarget();
    }
}