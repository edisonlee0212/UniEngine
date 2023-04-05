//
// Created by lllll on 11/2/2021.
//
#include <StrandsRenderer.hpp>
#include "EditorLayer.hpp"
#include "Editor.hpp"
#include "Inputs.hpp"
#include "Windows.hpp"
#include "Prefab.hpp"
#include "RenderLayer.hpp"
#include "Application.hpp"
#include "Camera.hpp"
#include "ClassRegistry.hpp"
#include "DefaultResources.hpp"
#include "Joint.hpp"
#include "Lights.hpp"
#include "MeshRenderer.hpp"
#include "Particles.hpp"
#include "PhysicsLayer.hpp"
#include "PlayerController.hpp"
#include "PostProcessing.hpp"
#include "ProjectManager.hpp"
#include "RigidBody.hpp"
#include "SkinnedMeshRenderer.hpp"
#include "Utilities.hpp"
#include "Scene.hpp"

using namespace UniEngine;

void EditorLayer::OnCreate() {
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
    
    m_sceneCamera = Serialization::ProduceSerializable<Camera>();
    m_sceneCamera->m_clearColor = glm::vec3(59.0f / 255.0f, 85 / 255.0f, 143 / 255.f);
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
        auto scene = Application::GetActiveScene();
        if (Application::IsPlaying() && scene->HasPrivateComponent<RigidBody>(entity)) {
            auto rigidBody = scene->GetOrSetPrivateComponent<RigidBody>(entity).lock();
            if (!rigidBody->IsKinematic() && rigidBody->Registered()) {
                reload = true;
                readOnly = true;
            }
        }
        if (reload) {
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
        if (ImGui::Selectable("Position##Local", &m_localPositionSelected) && m_localPositionSelected) {
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
        if (ImGui::Selectable("Rotation##Local", &m_localRotationSelected) && m_localRotationSelected) {
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
        if (ImGui::Selectable("Scale##Local", &m_localScaleSelected) && m_localScaleSelected) {
            m_localRotationSelected = false;
            m_localPositionSelected = false;
        }
        if (edited) {
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

void EditorLayer::PreUpdate() {
    m_mainCameraFocusOverride = false;
    m_sceneCameraFocusOverride = false;
    if (ImGui::BeginMainMenuBar()) {
        switch (Application::GetGameStatus()) {
            case GameStatus::Stop: {
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["PlayButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Play();
                }
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["StepButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Step();
                }
                break;
            }
            case GameStatus::Playing: {
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["PauseButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Pause();
                }
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["StopButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Stop();
                }
                break;
            }
            case GameStatus::Pause: {
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["PlayButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Play();
                }
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["StepButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Step();
                }
                if (ImGui::ImageButton(
                        (ImTextureID) Editor::AssetIcons()["StopButton"]->UnsafeGetGLTexture()->Id(),
                        {15, 15},
                        {0, 1},
                        {1, 0})) {
                    Application::Stop();
                }
                break;
            }
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Project")) {
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
        if (ImGui::BeginMenu("View")) {
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
    m_mouseScreenPosition = glm::vec2(FLT_MAX, FLT_MIN);
#pragma region Scene Window
    if (m_showSceneWindow) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        if (ImGui::Begin("Scene")) {
            if (ImGui::BeginChild("SceneCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar)) {
                // Using a Child allow to fill all the space of the window.
                // It also allows customization
                if (m_sceneCameraWindowFocused) {
                    auto mp = ImGui::GetMousePos();
                    auto wp = ImGui::GetWindowPos();
                    m_mouseScreenPosition = glm::vec2(mp.x - wp.x, mp.y - wp.y - 20);
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
    if(m_showSceneWindow) RenderToSceneCamera();

    if (m_applyTransformToMainCamera && !Application::IsPlaying())
    {
        const auto scene = Application::GetActiveScene();
        if(const auto camera = scene->m_mainCamera.Get<Camera>(); camera && scene->IsEntityValid(camera->GetOwner()))
        {
            GlobalTransform globalTransform;
            globalTransform.SetPosition(m_sceneCameraPosition);
            globalTransform.SetRotation(m_sceneCameraRotation);
            scene->SetDataComponent(camera->GetOwner(), globalTransform);
        }
    }
}

void EditorLayer::LateUpdate() {
    
}

bool EditorLayer::DrawEntityMenu(const bool &enabled, const Entity &entity) {
    bool deleted = false;
    if (ImGui::BeginPopupContextItem(std::to_string(entity.GetIndex()).c_str())) {
        auto scene = GetScene();
        ImGui::Text(("Handle: " + std::to_string(scene->GetEntityHandle(entity).GetValue())).c_str());
        if (ImGui::Button("Delete")) {
            scene->DeleteEntity(entity);
            deleted = true;
        }
        if (!deleted && ImGui::Button(enabled ? "Disable" : "Enable")) {
            if (enabled) {
                scene->SetEnable(entity, false);
            } else {
                scene->SetEnable(entity, true);
            }
        }
        const std::string tag = "##Entity" + std::to_string(scene->GetEntityHandle(entity));
        if (!deleted && ImGui::BeginMenu(("Rename" + tag).c_str())) {
            static char newName[256];
            ImGui::InputText("New name", newName, 256);
            if (ImGui::Button("Confirm")) {
                scene->SetEntityName(entity, std::string(newName));
                memset(newName, 0, 256);
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    return deleted;
}

void EditorLayer::InspectComponentData(Entity entity, IDataComponent *data, DataComponentType type, bool isRoot) {
    auto &editorManager = Editor::GetInstance();
    if (editorManager.m_componentDataInspectorMap.find(type.m_typeId) !=
        editorManager.m_componentDataInspectorMap.end()) {
        if (editorManager.m_componentDataInspectorMap.at(type.m_typeId)(entity, data, isRoot)) {
            auto scene = GetScene();
            scene->SetUnsaved();
        }
    }
}

Entity EditorLayer::MouseEntitySelection(const glm::vec2 &mousePosition) {
    Entity retVal;
    m_sceneCameraEntityRecorder->Bind();
    float entityIndex = 0;
    const glm::vec2 resolution = m_sceneCameraEntityRecorder->GetResolution();
    glm::vec2 point = resolution;
    point.x = mousePosition.x;
    point.y -= mousePosition.y;
    if (point.x >= 0 && point.x < resolution.x && point.y >= 0 && point.y < resolution.y) {
        glReadPixels(point.x, point.y, 1, 1, GL_RED, GL_FLOAT, &entityIndex);
        if (entityIndex > 0) {
            auto scene = GetScene();
            retVal = scene->GetEntity(static_cast<unsigned>(entityIndex));
        }
    }
    return retVal;
}

void EditorLayer::HighLightEntityPrePassHelper(const Entity &entity) {
    auto scene = GetScene();
    if (!scene->IsEntityValid(entity) || !scene->IsEntityEnabled(entity))
        return;
    scene->ForEachChild(entity, [&](Entity child) {
        HighLightEntityPrePassHelper(child);
    });
    if (scene->HasPrivateComponent<MeshRenderer>(entity)) {
        auto mmc = scene->GetOrSetPrivateComponent<MeshRenderer>(entity).lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr) {
            DefaultResources::m_sceneHighlightPrePassProgram->SetFloat4x4(
                    "model", scene->GetDataComponent<GlobalTransform>(entity).m_value);
            mesh->Draw();
        }
    }
    if (scene->HasPrivateComponent<Particles>(entity)) {
        auto immc = scene->GetOrSetPrivateComponent<Particles>(entity).lock();
        auto material = immc->m_material.Get<Material>();
        auto mesh = immc->m_mesh.Get<Mesh>();
        if (immc->IsEnabled() && material != nullptr && mesh != nullptr) {
            DefaultResources::m_sceneHighlightPrePassInstancedProgram->SetFloat4x4(
                    "model", scene->GetDataComponent<GlobalTransform>(entity).m_value);
            mesh->DrawInstanced(immc->m_matrices);
        }
    }
    if (scene->HasPrivateComponent<SkinnedMeshRenderer>(entity)) {
        auto smmc = scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(entity).lock();
        auto material = smmc->m_material.Get<Material>();
        auto skinnedMesh = smmc->m_skinnedMesh.Get<SkinnedMesh>();
        if (smmc->IsEnabled() && material != nullptr && skinnedMesh != nullptr) {
            GlobalTransform ltw;
            if (!smmc->RagDoll())
                ltw = scene->GetDataComponent<GlobalTransform>(entity);
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedPrePassProgram->SetFloat4x4("model", ltw.m_value);
            skinnedMesh->Draw();
        }
    }
    if (scene->HasPrivateComponent<StrandsRenderer>(entity)) {
        auto mmc = scene->GetOrSetPrivateComponent<StrandsRenderer>(entity).lock();
        auto material = mmc->m_material.Get<Material>();
        auto strands = mmc->m_strands.Get<Strands>();
        if (mmc->IsEnabled() && material != nullptr && strands != nullptr) {
            DefaultResources::m_sceneHighlightStrandsPrePassProgram->SetFloat4x4(
                "model", scene->GetDataComponent<GlobalTransform>(entity).m_value);
            strands->Draw();
        }
    }
}

void EditorLayer::HighLightEntityHelper(const Entity &entity) {
    auto scene = GetScene();
    if (!scene->IsEntityValid(entity) || !scene->IsEntityEnabled(entity))
        return;
    scene->ForEachChild(entity, [&](Entity child) {
        HighLightEntityHelper(child);
    });
    if (scene->HasPrivateComponent<MeshRenderer>(entity)) {
        auto mmc = scene->GetOrSetPrivateComponent<MeshRenderer>(entity).lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr) {
            auto ltw = scene->GetDataComponent<GlobalTransform>(entity);
            DefaultResources::m_sceneHighlightProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightProgram->SetFloat3("scale", ltw.GetScale());
            mesh->Draw();
        }
    }
    if (scene->HasPrivateComponent<Particles>(entity)) {
        auto immc = scene->GetOrSetPrivateComponent<Particles>(entity).lock();
        auto material = immc->m_material.Get<Material>();
        auto mesh = immc->m_mesh.Get<Mesh>();
        if (immc->IsEnabled() && material != nullptr && mesh != nullptr) {
            auto ltw = scene->GetDataComponent<GlobalTransform>(entity);
            DefaultResources::m_sceneHighlightInstancedProgram->SetFloat4x4("model", ltw.m_value);
            mesh->DrawInstanced(immc->m_matrices);
        }
    }
    if (scene->HasPrivateComponent<SkinnedMeshRenderer>(entity)) {
        auto smmc = scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(entity).lock();
        auto material = smmc->m_material.Get<Material>();
        auto skinnedMesh = smmc->m_skinnedMesh.Get<SkinnedMesh>();
        if (smmc->IsEnabled() && material != nullptr && skinnedMesh != nullptr) {
            GlobalTransform ltw;
            if (!smmc->RagDoll())
                ltw = scene->GetDataComponent<GlobalTransform>(entity);
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat3("scale", ltw.GetScale());
            skinnedMesh->Draw();
        }
    }
    if (scene->HasPrivateComponent<StrandsRenderer>(entity)) {
        auto mmc = scene->GetOrSetPrivateComponent<StrandsRenderer>(entity).lock();
        auto material = mmc->m_material.Get<Material>();
        auto strands = mmc->m_strands.Get<Strands>();
        if (mmc->IsEnabled() && material != nullptr && strands != nullptr) {
            auto ltw = scene->GetDataComponent<GlobalTransform>(entity);
            DefaultResources::m_sceneHighlightStrandsProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightStrandsProgram->SetFloat3("scale", ltw.GetScale());
            strands->Draw();
        }
    }
}

void EditorLayer::MoveCamera(
        const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime) {
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

void EditorLayer::HighLightEntity(const Entity &entity, const glm::vec4 &color) {
    auto scene = GetScene();
    if (!scene->IsEntityValid(entity) || !scene->IsEntityEnabled(entity))
        return;
    Camera::m_cameraInfoBlock.UploadMatrices(m_sceneCamera, m_sceneCameraPosition, m_sceneCameraRotation);
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
    DefaultResources::m_sceneHighlightStrandsPrePassProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
	DefaultResources::m_sceneHighlightSkinnedPrePassProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
    DefaultResources::m_sceneHighlightPrePassInstancedProgram->SetFloat4("color", glm::vec4(1.0, 0.5, 0.0, 0.1));
    DefaultResources::m_sceneHighlightPrePassInstancedSkinnedProgram->SetFloat4("color",
                                                                                glm::vec4(1.0, 0.5, 0.0, 0.1));

    HighLightEntityPrePassHelper(entity);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    DefaultResources::m_sceneHighlightProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightStrandsProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightInstancedProgram->SetFloat4("color", color);
    DefaultResources::m_sceneHighlightInstancedSkinnedProgram->SetFloat4("color", color);

    HighLightEntityHelper(entity);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
}

static const char *HierarchyDisplayMode[]{"Archetype", "Hierarchy"};

void EditorLayer::RenderToSceneCamera() {
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;
    ProfilerLayer::StartEvent("RenderToSceneCamera");

    const auto resolution = m_sceneCamera->UnsafeGetGBuffer()->GetResolution();
    if (m_sceneCameraResolutionX != 0 && m_sceneCameraResolutionY != 0 &&
        (resolution.x != m_sceneCameraResolutionX || resolution.y != m_sceneCameraResolutionY)) {
        m_sceneCamera->ResizeResolution(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorderTexture->ReSize(
                0, GL_R32F, GL_RED, GL_FLOAT, 0, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorderRenderBuffer->AllocateStorage(
                GL_DEPTH24_STENCIL8, m_sceneCameraResolutionX, m_sceneCameraResolutionY);
        m_sceneCameraEntityRecorder->SetResolution(m_sceneCameraResolutionX, m_sceneCameraResolutionY);
    }
    m_sceneCamera->Clear();
    m_sceneCameraEntityRecorder->Clear();
    if (m_lockCamera) {
        const float elapsedTime = Application::Time().CurrentTime() - m_transitionTimer;
        float a = 1.0f - glm::pow(1.0 - elapsedTime / m_transitionTime, 4.0f);
        if (elapsedTime >= m_transitionTime)
            a = 1.0f;
        m_sceneCameraRotation = glm::mix(m_previousRotation, m_targetRotation, a);
        m_sceneCameraPosition = glm::mix(m_previousPosition, m_targetPosition, a);
        if (a >= 1.0f) {
            m_lockCamera = false;
            m_sceneCameraRotation = m_targetRotation;
            m_sceneCameraPosition = m_targetPosition;
            Camera::ReverseAngle(m_targetRotation, m_sceneCameraPitchAngle, m_sceneCameraYawAngle);
        }
    }

    if (m_sceneCamera->m_requireRendering) {
        GlobalTransform sceneCameraGT;
        sceneCameraGT.SetValue(m_sceneCameraPosition, m_sceneCameraRotation, glm::vec3(1.0f));
        renderLayer->RenderToCamera(m_sceneCamera, sceneCameraGT);
#pragma region For entity selection
        OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, true);
        OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
        OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
        OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
        m_sceneCameraEntityRecorder->Bind();
        for (auto &i: renderLayer->m_deferredRenderInstances) {
            const auto &cameraComponent = i.first;
            renderLayer->DispatchRenderCommands(
                    i.second,
                    [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                        switch (renderCommand.m_geometryType) {
                            case RenderGeometryType::Mesh: {
                                auto &program = DefaultResources::m_sceneCameraEntityRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                            case RenderGeometryType::SkinnedMesh: {
                                auto &program = DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                renderCommand.m_boneMatrices->UploadBones(
                                    std::dynamic_pointer_cast<SkinnedMesh>(renderCommand.m_renderGeometry));
                                DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                            case RenderGeometryType::Strands: {
                                auto& program = DefaultResources::m_sceneCameraEntityStrandsRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityStrandsRecorderProgram->SetInt(
                                    "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                        }
                    },
                    false);
        }
        for (auto &i: renderLayer->m_deferredInstancedRenderInstances) {
            renderLayer->DispatchRenderCommands(
                    i.second,
                    [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                        switch (renderCommand.m_geometryType) {
                            case RenderGeometryType::Mesh: {
                                auto &program = DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetFloat4x4(
                                        "model", renderCommand.m_globalTransform.m_value);
                                renderCommand.m_renderGeometry->DrawInstanced(renderCommand.m_matrices);
                                break;
                            }
                        }
                    },
                    false);
        }
        for (auto &i: renderLayer->m_forwardRenderInstances) {
            renderLayer->DispatchRenderCommands(
                    i.second,
                    [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                        switch (renderCommand.m_geometryType) {
                            case RenderGeometryType::Mesh: {
                                auto &program = DefaultResources::m_sceneCameraEntityRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                            case RenderGeometryType::SkinnedMesh: {
                                auto &program = DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                renderCommand.m_boneMatrices->UploadBones(
                                        std::dynamic_pointer_cast<SkinnedMesh>(renderCommand.m_renderGeometry));
                                DefaultResources::m_sceneCameraEntitySkinnedRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                            case RenderGeometryType::Strands: {
                                auto& program = DefaultResources::m_sceneCameraEntityStrandsRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityStrandsRecorderProgram->SetInt(
                                    "EntityIndex", renderCommand.m_owner.GetIndex());
                                renderCommand.m_renderGeometry->Draw();
                                break;
                            }
                        }
                    },
                    false);
        }
        for (auto &i: renderLayer->m_forwardInstancedRenderInstances) {
            renderLayer->DispatchRenderCommands(
                    i.second,
                    [&](const std::shared_ptr<Material> &material, const RenderCommand &renderCommand) {
                        switch (renderCommand.m_geometryType) {
                            case RenderGeometryType::Mesh: {
                                auto &program = DefaultResources::m_sceneCameraEntityInstancedRecorderProgram;
                                program->Bind();
                                program->SetFloat4x4("model", renderCommand.m_globalTransform.m_value);
                                DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetInt(
                                        "EntityIndex", renderCommand.m_owner.GetIndex());
                                DefaultResources::m_sceneCameraEntityInstancedRecorderProgram->SetFloat4x4(
                                        "model", renderCommand.m_globalTransform.m_value);
                                renderCommand.m_renderGeometry->DrawInstanced(renderCommand.m_matrices);
                                break;
                            }
                        }
                    },
                    false);
        }
#pragma endregion
    } else {
    }
    ProfilerLayer::EndEvent("RenderToSceneCamera");
}

void EditorLayer::DrawEntityNode(const Entity &entity, const unsigned &hierarchyLevel) {
    auto scene = GetScene();
    std::string title = std::to_string(entity.GetIndex()) + ": ";
    title += scene->GetEntityName(entity);
    const bool enabled = scene->IsEntityEnabled(entity);
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({1, 1, 1, 1}));
    }
    const int index = m_selectedEntityHierarchyList.size() - hierarchyLevel - 1;
    if (!m_selectedEntityHierarchyList.empty() && index >= 0 && index < m_selectedEntityHierarchyList.size() &&
        m_selectedEntityHierarchyList[index] == entity) {
        ImGui::SetNextItemOpen(true);
    }
    const bool opened = ImGui::TreeNodeEx(
            title.c_str(),
            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_NoAutoOpenOnLog |
            (m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed : ImGuiTreeNodeFlags_FramePadding));
    if (ImGui::BeginDragDropSource()) {
        auto handle = scene->GetEntityHandle(entity);
        ImGui::SetDragDropPayload("Entity", &handle, sizeof(Handle));
        ImGui::TextColored(ImVec4(0, 0, 1, 1), title.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity")) {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            scene->SetParent(scene->GetEntity(*static_cast<Handle *>(payload->Data)), entity, true);
        }
        ImGui::EndDragDropTarget();
    }
    if (enabled) {
        ImGui::PopStyleColor();
    }
    if (!m_lockEntitySelection && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        SetSelectedEntity(entity, false);
    }
    const bool deleted = DrawEntityMenu(enabled, entity);
    if (opened && !deleted) {
        ImGui::TreePush();
        scene->ForEachChild(
                entity, [=](Entity child) {
                    DrawEntityNode(child, hierarchyLevel + 1);
                });
        ImGui::TreePop();
    }
}

void EditorLayer::OnInspect() {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Scene", &m_showSceneWindow);
            ImGui::Checkbox("Camera", &m_showCameraWindow);
            ImGui::Checkbox("Entity Explorer", &m_showEntityExplorerWindow);
            ImGui::Checkbox("Entity Inspector", &m_showEntityInspectorWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }


    auto &editorManager = Editor::GetInstance();
    if (m_leftMouseButtonHold && !Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, Windows::GetWindow())) {
        m_leftMouseButtonHold = false;
    }
    if (m_rightMouseButtonHold && !Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, Windows::GetWindow())) {
        m_rightMouseButtonHold = false;
        m_startMouse = false;
    }

    auto scene = GetScene();
    if (scene && m_showEntityExplorerWindow) {
        ImGui::Begin("Entity Explorer");
        if (ImGui::BeginPopupContextWindow("NewEntityPopup")) {
            if (ImGui::Button("Create new entity")) {
                auto newEntity = scene->CreateEntity(m_basicEntityArchetype);
            }
            ImGui::EndPopup();
        }
        ImGui::Combo(
                "Display mode", &m_selectedHierarchyDisplayMode, HierarchyDisplayMode,
                IM_ARRAYSIZE(HierarchyDisplayMode));
        std::string title = scene->GetTitle();
        if (ImGui::CollapsingHeader(title.c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow)) {
            Editor::DraggableAsset(scene);
            Editor::RenameAsset(scene);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                ProjectManager::GetInstance().m_inspectingAsset = scene;
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity")) {
                    IM_ASSERT(payload->DataSize == sizeof(Handle));
                    auto payload_n = *static_cast<Handle *>(payload->Data);
                    auto newEntity = scene->GetEntity(payload_n);
                    auto parent = scene->GetParent(newEntity);
                    if (parent.GetIndex() != 0)
                        scene->RemoveChild(newEntity, parent);
                }
                ImGui::EndDragDropTarget();
            }
            if (m_selectedHierarchyDisplayMode == 0) {
                scene->UnsafeForEachEntityStorage(
                        [&](int i, const std::string &name, const DataComponentStorage &storage) {
                            if (i == 0)
                                return;
                            ImGui::Separator();
                            const std::string title = std::to_string(i) + ". " + name;
                            if (ImGui::TreeNode(title.c_str())) {
                                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
                                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
                                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
                                for (int j = 0; j < storage.m_entityAliveCount; j++) {
                                    Entity entity = storage.m_chunkArray.m_entities.at(j);
                                    std::string title = std::to_string(entity.GetIndex()) + ": ";
                                    title += scene->GetEntityName(entity);
                                    const bool enabled = scene->IsEntityEnabled(entity);
                                    if (enabled) {
                                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({1, 1, 1, 1}));
                                    }
                                    ImGui::TreeNodeEx(
                                            title.c_str(),
                                            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf |
                                            ImGuiTreeNodeFlags_NoAutoOpenOnLog |
                                            (m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed
                                                                        : ImGuiTreeNodeFlags_FramePadding));
                                    if (enabled) {
                                        ImGui::PopStyleColor();
                                    }
                                    DrawEntityMenu(enabled, entity);
                                    if (!m_lockEntitySelection && ImGui::IsItemHovered() &&
                                        ImGui::IsMouseClicked(0)) {
                                        SetSelectedEntity(entity, false);
                                    }
                                }
                                ImGui::PopStyleColor();
                                ImGui::PopStyleColor();
                                ImGui::PopStyleColor();
                                ImGui::TreePop();
                            }
                        });
            } else if (m_selectedHierarchyDisplayMode == 1) {
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
                scene->ForAllEntities([&](int i, Entity entity) {
                    if (scene->GetParent(entity).GetIndex() == 0)
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
    if (scene && m_showEntityInspectorWindow) {
        ImGui::Begin("Entity Inspector");
        ImGui::Text("Selection:");
        ImGui::SameLine();
        ImGui::Checkbox("Lock", &m_lockEntitySelection);
        ImGui::SameLine();
        ImGui::Checkbox("Highlight", &m_highlightSelection);
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            SetSelectedEntity({});
        }
        ImGui::Separator();
        if (scene->IsEntityValid(m_selectedEntity)) {
            std::string title = std::to_string(m_selectedEntity.GetIndex()) + ": ";
            title += scene->GetEntityName(m_selectedEntity);
            bool enabled = scene->IsEntityEnabled(m_selectedEntity);
            if (ImGui::Checkbox((title + "##EnabledCheckbox").c_str(), &enabled)) {
                if (scene->IsEntityEnabled(m_selectedEntity) != enabled) {
                    scene->SetEnable(m_selectedEntity, enabled);
                }
            }
            ImGui::SameLine();
            bool isStatic = scene->IsEntityStatic(m_selectedEntity);
            if (ImGui::Checkbox("Static##StaticCheckbox", &isStatic)) {
                if (scene->IsEntityStatic(m_selectedEntity) != isStatic) {
                    scene->SetEntityStatic(m_selectedEntity, enabled);
                }
            }

            bool deleted = DrawEntityMenu(scene->IsEntityEnabled(m_selectedEntity), m_selectedEntity);
            if (!deleted) {
                if (ImGui::CollapsingHeader("Data components", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::BeginPopupContextItem("DataComponentInspectorPopup")) {
                        ImGui::Text("Add data component: ");
                        ImGui::Separator();
                        for (auto &i: editorManager.m_componentDataMenuList) {
                            i.second(m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }
                    bool skip = false;
                    int i = 0;
                    scene->UnsafeForEachDataComponent(m_selectedEntity, [&](DataComponentType type, void *data) {
                        if (skip)
                            return;
                        std::string info = type.m_name;
                        info += " Size: " + std::to_string(type.m_size);
                        ImGui::Text(info.c_str());
                        ImGui::PushID(i);
                        if (ImGui::BeginPopupContextItem(
                                ("DataComponentDeletePopup" + std::to_string(i)).c_str())) {
                            if (ImGui::Button("Remove")) {
                                skip = true;
                                scene->RemoveDataComponent(m_selectedEntity, type.m_typeId);
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                        InspectComponentData(
                                m_selectedEntity,
                                static_cast<IDataComponent *>(data),
                                type,
                                scene->GetParent(m_selectedEntity).GetIndex() != 0);
                        ImGui::Separator();
                        i++;
                    });
                }

                if (ImGui::CollapsingHeader("Private components", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::BeginPopupContextItem("PrivateComponentInspectorPopup")) {
                        ImGui::Text("Add private component: ");
                        ImGui::Separator();
                        for (auto &i: editorManager.m_privateComponentMenuList) {
                            i.second(m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }

                    int i = 0;
                    bool skip = false;
                    scene->ForEachPrivateComponent(m_selectedEntity, [&](PrivateComponentElement &data) {
                        if (skip)
                            return;
                        ImGui::Checkbox(
                                data.m_privateComponentData->GetTypeName().c_str(),
                                &data.m_privateComponentData->m_enabled);
                        Editor::DraggablePrivateComponent(data.m_privateComponentData);
                        const std::string tag = "##" + data.m_privateComponentData->GetTypeName() +
                                                std::to_string(data.m_privateComponentData->GetHandle());
                        if (ImGui::BeginPopupContextItem(tag.c_str())) {
                            if (ImGui::Button(("Remove" + tag).c_str())) {
                                skip = true;
                                scene->RemovePrivateComponent(m_selectedEntity, data.m_typeId);
                            }
                            ImGui::EndPopup();
                        }
                        if (!skip) {
                            if (ImGui::TreeNodeEx(
                                    ("Component Settings##" + std::to_string(i)).c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen)) {
                                data.m_privateComponentData->OnInspect();
                                ImGui::TreePop();
                            }
                        }
                        ImGui::Separator();
                        i++;
                    });
                }
            }
        } else {
            m_selectedEntity = Entity();
        }
        ImGui::End();
    }
    
    if (scene && SceneCameraWindowFocused() && Inputs::GetKeyInternal(GLFW_KEY_DELETE, Windows::GetWindow()))
    {
        if (scene->IsEntityValid(m_selectedEntity))
        {
            scene->DeleteEntity(m_selectedEntity);
        }
    }
    
    if (m_showSceneWindow) SceneCameraWindow();
    if (m_showCameraWindow) MainCameraWindow();
    ProjectManager::OnInspect();
}

void EditorLayer::SetSelectedEntity(const Entity &entity, bool openMenu) {
    if (entity == m_selectedEntity)
        return;
    m_selectedEntityHierarchyList.clear();
    if (entity.GetIndex() == 0) {
        m_selectedEntity = Entity();
        m_lockEntitySelection = false;
        return;
    }
    auto scene = GetScene();
    if (!scene->IsEntityValid(entity))
        return;
    m_selectedEntity = entity;
    if (!openMenu)
        return;
    auto walker = entity;
    while (walker.GetIndex() != 0) {
        m_selectedEntityHierarchyList.push_back(walker);
        walker = scene->GetParent(walker);
    }
}

bool EditorLayer::MainCameraWindowFocused() {
    return m_mainCameraWindowFocused || m_mainCameraFocusOverride;
}

bool EditorLayer::SceneCameraWindowFocused() {
    return m_sceneCameraWindowFocused || m_sceneCameraFocusOverride;
}

void EditorLayer::SceneCameraWindow() {
    auto scene = GetScene();
#pragma region Scene Window
    ImVec2 viewPortSize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Scene")) {
        // Using a Child allow to fill all the space of the window.
        // It also allows customization
        static int corner = 1;
        if (ImGui::BeginChild("SceneCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Settings")) {
                    ImGui::Checkbox("Display info", &m_showSceneInfo);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            viewPortSize = ImGui::GetWindowSize();
            m_sceneCameraResolutionX = viewPortSize.x * m_sceneCameraResolutionMultiplier;
            m_sceneCameraResolutionY = (viewPortSize.y - 20) * m_sceneCameraResolutionMultiplier;
            ImVec2 overlayPos = ImGui::GetWindowPos();
            if (m_sceneCamera && m_sceneCamera->m_rendered) {
                // Because I use the texture from OpenGL, I need to invert the V from the UV.
                ImGui::Image(
                        (ImTextureID) m_sceneCamera->GetTexture()->UnsafeGetGLTexture()->Id(),
                        ImVec2(viewPortSize.x, viewPortSize.y - 20),
                        ImVec2(0, 1),
                        ImVec2(1, 0));
                CameraWindowDragAndDrop();
            } else {
                ImGui::Text("No active scene camera!");
            }
            ImVec2 window_pos = ImVec2(
                (corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
                (corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y));
            if (m_showSceneInfo) {
                ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
                if (ImGui::BeginChild("Info", ImVec2(200, 300), true, window_flags)) {
                    ImGui::Text("_");
                    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
                    std::string trisstr = "";
                    if (ImGui::IsMousePosValid()) {
                        glm::vec2 pos;
                        Inputs::GetMousePosition(pos);
                        ImGui::Text("Mouse Pos: (%.1f,%.1f)", pos.x, pos.y);
                    }
                    else {
                        ImGui::Text("Mouse Pos: <invalid>");
                    }

                    if (ImGui::Button("Reset camera")) {
                        MoveCamera(m_defaultSceneCameraRotation, m_defaultSceneCameraPosition);
                    }
                    if (ImGui::Button("Set default")) {
                        m_defaultSceneCameraPosition = m_sceneCameraPosition;
                        m_defaultSceneCameraRotation = m_sceneCameraRotation;
                    }
                    ImGui::PushItemWidth(100);
                    ImGui::Checkbox("Use background color", &m_sceneCamera->m_useClearColor);
                    ImGui::ColorEdit3("Bg color", &m_sceneCamera->m_clearColor.x);
                    ImGui::SliderFloat("Fov", &m_sceneCamera->m_fov, 1.0f, 359.f, "%.1f");
                    ImGui::DragFloat3("Position", &m_sceneCameraPosition.x, 0.1f, 0, 0, "%.1f");
                    ImGui::DragFloat("Speed", &m_velocity, 0.1f, 0, 0, "%.1f");
                    ImGui::DragFloat("Sensitivity", &m_sensitivity, 0.1f, 0, 0, "%.1f");
                    ImGui::Checkbox("Apply transform to main camera", &m_applyTransformToMainCamera);
                    ImGui::DragFloat("Resolution multiplier", &m_sceneCameraResolutionMultiplier, 0.1f, 0.1f, 4.0f);
                    Editor::DragAndDropButton<Cubemap>(m_sceneCamera->m_skybox, "Skybox", true);
                    ImGui::PopItemWidth();
                }
                ImGui::EndChild();
            }
            if (m_sceneCameraWindowFocused) {
#pragma region Scene Camera Controller
                float xOffset = 0;
                float yOffset = 0;
                if (!m_startMouse) {
                    m_lastX = m_mouseScreenPosition.x;
                    m_lastY = m_mouseScreenPosition.y;
                    m_startMouse = true;
                }
                xOffset = m_mouseScreenPosition.x - m_lastX;
                yOffset = -m_mouseScreenPosition.y + m_lastY;
                m_lastX = m_mouseScreenPosition.x;
                m_lastY = m_mouseScreenPosition.y;

                if (!m_rightMouseButtonHold &&
                    !(m_mouseScreenPosition.x < 0 || m_mouseScreenPosition.y < 0 ||
                      m_mouseScreenPosition.x > viewPortSize.x ||
                      m_mouseScreenPosition.y > viewPortSize.y) &&
                    Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, Windows::GetWindow())) {
                    m_rightMouseButtonHold = true;
                }
                if (m_rightMouseButtonHold && !m_lockCamera) {
                    glm::vec3 front = m_sceneCameraRotation * glm::vec3(0, 0, -1);
                    glm::vec3 right = m_sceneCameraRotation * glm::vec3(1, 0, 0);
                    if (Inputs::GetKeyInternal(GLFW_KEY_W, Windows::GetWindow())) {
                        m_sceneCameraPosition +=
                                front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                    }
                    if (Inputs::GetKeyInternal(GLFW_KEY_S, Windows::GetWindow())) {
                        m_sceneCameraPosition -=
                                front * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                    }
                    if (Inputs::GetKeyInternal(GLFW_KEY_A, Windows::GetWindow())) {
                        m_sceneCameraPosition -=
                                right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                    }
                    if (Inputs::GetKeyInternal(GLFW_KEY_D, Windows::GetWindow())) {
                        m_sceneCameraPosition +=
                                right * static_cast<float>(Application::Time().DeltaTime()) * m_velocity;
                    }
                    if (Inputs::GetKeyInternal(GLFW_KEY_LEFT_SHIFT, Windows::GetWindow())) {
                        m_sceneCameraPosition.y += m_velocity * static_cast<float>(Application::Time().DeltaTime());
                    }
                    if (Inputs::GetKeyInternal(GLFW_KEY_LEFT_CONTROL, Windows::GetWindow())) {
                        m_sceneCameraPosition.y -= m_velocity * static_cast<float>(Application::Time().DeltaTime());
                    }
                    if (xOffset != 0.0f || yOffset != 0.0f) {
                        m_sceneCameraYawAngle += xOffset * m_sensitivity;
                        m_sceneCameraPitchAngle += yOffset * m_sensitivity;
                        if (m_sceneCameraPitchAngle > 89.0f)
                            m_sceneCameraPitchAngle = 89.0f;
                        if (m_sceneCameraPitchAngle < -89.0f)
                            m_sceneCameraPitchAngle = -89.0f;

                        m_sceneCameraRotation =
                                Camera::ProcessMouseMovement(m_sceneCameraYawAngle, m_sceneCameraPitchAngle, false);
                    }
#pragma endregion
                }
            }
        }
#pragma region Gizmos and Entity Selection
        bool mouseSelectEntity = true;
        if (scene->IsEntityValid(m_selectedEntity)) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewPortSize.x,
                              viewPortSize.y - 20);
            glm::mat4 cameraView =
                    glm::inverse(glm::translate(m_sceneCameraPosition) * glm::mat4_cast(m_sceneCameraRotation));
            glm::mat4 cameraProjection = m_sceneCamera->GetProjection();
            const auto op = m_localPositionSelected ? ImGuizmo::OPERATION::TRANSLATE
                                                    : m_localRotationSelected ? ImGuizmo::OPERATION::ROTATE
                                                                              : ImGuizmo::OPERATION::SCALE;

            auto transform = scene->GetDataComponent<Transform>(m_selectedEntity);
            GlobalTransform parentGlobalTransform;
            Entity parentEntity = scene->GetParent(m_selectedEntity);
            if (parentEntity.GetIndex() != 0) {
                parentGlobalTransform = scene->GetDataComponent<GlobalTransform>(
                        scene->GetParent(m_selectedEntity));
            }
            auto globalTransform = scene->GetDataComponent<GlobalTransform>(m_selectedEntity);
            ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    op,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(globalTransform.m_value));
            if (ImGuizmo::IsUsing()) {
                transform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
                scene->SetDataComponent(m_selectedEntity, transform);
                transform.Decompose(
                        m_previouslyStoredPosition, m_previouslyStoredRotation, m_previouslyStoredScale);
                mouseSelectEntity = false;
            }
        }
        if (!m_lockEntitySelection && m_sceneCameraWindowFocused && mouseSelectEntity) {
            if (!m_leftMouseButtonHold &&
                !(m_mouseScreenPosition.x < 0 || m_mouseScreenPosition.y < 0 ||
                  m_mouseScreenPosition.x > viewPortSize.x ||
                  m_mouseScreenPosition.y > viewPortSize.y) &&
                Inputs::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, Windows::GetWindow())) {
                Entity focusedEntity = MouseEntitySelection(m_mouseScreenPosition);
                if (focusedEntity == Entity()) {
                    SetSelectedEntity(Entity());
                } else {
                    Entity walker = focusedEntity;
                    bool found = false;
                    while (walker.GetIndex() != 0) {
                        if (walker == m_selectedEntity) {
                            found = true;
                            break;
                        }
                        walker = scene->GetParent(walker);
                    }
                    if (found) {
                        walker = scene->GetParent(walker);
                        if (walker.GetIndex() == 0) {
                            SetSelectedEntity(focusedEntity);
                        } else {
                            SetSelectedEntity(walker);
                        }
                    } else {
                        SetSelectedEntity(focusedEntity);
                    }
                }
                m_leftMouseButtonHold = true;
            }
            if (m_highlightSelection) HighLightEntity(m_selectedEntity, glm::vec4(1.0, 0.5, 0.0, 0.8));
#pragma endregion
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            if (!m_sceneCameraWindowFocused) {
                m_rightMouseButtonHold = true;
                m_leftMouseButtonHold = true;
            }
            m_sceneCameraWindowFocused = true;
        } else {
            m_sceneCameraWindowFocused = false;
        }
        ImGui::EndChild();
    } else {
        m_sceneCameraWindowFocused = false;
    }
    m_sceneCamera->SetRequireRendering(
            !(ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed));

    ImGui::End();

    ImGui::PopStyleVar();

#pragma endregion
}

void EditorLayer::MainCameraWindow() {
    auto renderLayer = Application::GetLayer<RenderLayer>();
    if (!renderLayer)
        return;

    auto scene = GetScene();
#pragma region Window
    ImVec2 viewPortSize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Camera")) {
        static int corner = 1;
        // Using a Child allow to fill all the space of the window.
        // It also allows customization
        if (ImGui::BeginChild("MainCameraRenderer", ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5, 5});
                if (ImGui::BeginMenu("Settings")) {
#pragma region Menu
                    ImGui::Checkbox("Display info", &m_showCameraInfo);
#pragma endregion
                    ImGui::EndMenu();
                }
                ImGui::PopStyleVar();
                ImGui::EndMenuBar();
            }
            viewPortSize = ImGui::GetWindowSize();
            renderLayer->m_mainCameraResolutionX = viewPortSize.x * renderLayer->m_mainCameraResolutionMultiplier;
            renderLayer->m_mainCameraResolutionY = (viewPortSize.y - 20) * renderLayer->m_mainCameraResolutionMultiplier;
            //  Get the size of the child (i.e. the whole draw size of the windows).
            ImVec2 overlayPos = ImGui::GetWindowPos();
            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            auto mainCamera = scene->m_mainCamera.Get<Camera>();
            if (mainCamera && mainCamera->m_rendered) {
                auto id = mainCamera->GetTexture()->UnsafeGetGLTexture()->Id();
                ImGui::Image((ImTextureID) id, ImVec2(viewPortSize.x, viewPortSize.y - 20), ImVec2(0, 1),
                             ImVec2(1, 0));
                CameraWindowDragAndDrop();
            } else {
                ImGui::Text("No active main camera!");
            }

            ImVec2 window_pos = ImVec2(
                    (corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
                    (corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y));
            if (m_showCameraInfo) {
                ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                ImGui::SetNextWindowBgAlpha(0.35f);
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                                ImGuiWindowFlags_AlwaysAutoResize |
                                                ImGuiWindowFlags_NoSavedSettings |
                                                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

                if (ImGui::BeginChild("Render Info", ImVec2(200, 150), true, window_flags)) {
                    ImGui::Text("_");
                	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
                    ImGui::PushItemWidth(100);
                    ImGui::Checkbox("Auto resize", &renderLayer->m_allowAutoResize);
                    if(renderLayer->m_allowAutoResize)
                    {
                        ImGui::DragFloat("Resolution multiplier", &renderLayer->m_mainCameraResolutionMultiplier, 0.1f, 0.1f, 4.0f);
                    }
                    ImGui::PopItemWidth();
                    std::string trisstr = "";
                    if (renderLayer->m_triangles < 999)
                        trisstr += std::to_string(renderLayer->m_triangles);
                    else if (renderLayer->m_triangles < 999999)
                        trisstr += std::to_string((int) (renderLayer->m_triangles / 1000)) + "K";
                    else
                        trisstr += std::to_string((int) (renderLayer->m_triangles / 1000000)) + "M";
                    trisstr += " tris";
                    ImGui::Text(trisstr.c_str());
                    ImGui::Text("%d drawcall", renderLayer->m_drawCall);
                    ImGui::Separator();
                    if (ImGui::IsMousePosValid()) {
                        glm::vec2 pos;
                        Inputs::GetMousePosition(pos);
                        ImGui::Text("Mouse Pos: (%.1f,%.1f)", pos.x, pos.y);
                    } else {
                        ImGui::Text("Mouse Pos: <invalid>");
                    }
                }
                ImGui::EndChild();
            }
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            if (!m_mainCameraWindowFocused) {
                // m_rightMouseButtonHold = true;
                // m_leftMouseButtonHold = true;
            }
            m_mainCameraWindowFocused = true;
        } else {
            m_mainCameraWindowFocused = false;
        }

        ImGui::EndChild();
    } else {
        m_mainCameraWindowFocused = false;
    }
    auto mainCamera = scene->m_mainCamera.Get<Camera>();
    if (mainCamera) {
        mainCamera->SetRequireRendering(
                !ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed);
    }

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion
}

void EditorLayer::CameraWindowDragAndDrop() {
    AssetRef assetRef;
    if (Editor::UnsafeDroppableAsset(assetRef,
                                     {"Scene", "Prefab", "Mesh", "Strands", "Cubemap", "EnvironmentalMap"})) {
        auto scene = GetScene();
        auto asset = assetRef.Get<IAsset>();
        if (!Application::IsPlaying() && asset->GetTypeName() == "Scene") {
            auto scene = std::dynamic_pointer_cast<Scene>(asset);
            ProjectManager::SetStartScene(scene);
            Application::Attach(scene);
        } else if (asset->GetTypeName() == "Prefab") {
            auto entity = std::dynamic_pointer_cast<Prefab>(asset)->ToEntity(scene);
            scene->SetEntityName(entity, asset->GetTitle());
        } else if (asset->GetTypeName() == "Mesh") {
            Entity entity = scene->CreateEntity(asset->GetTitle());
            auto meshRenderer = scene->GetOrSetPrivateComponent<MeshRenderer>(entity).lock();
            meshRenderer->m_mesh.Set<Mesh>(std::dynamic_pointer_cast<Mesh>(asset));
            auto material = ProjectManager::CreateTemporaryAsset<Material>();
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer->m_material.Set<Material>(material);
        } else if (asset->GetTypeName() == "Strands") {
            Entity entity = scene->CreateEntity(asset->GetTitle());
            auto strandsRenderer = scene->GetOrSetPrivateComponent<StrandsRenderer>(entity).lock();
            strandsRenderer->m_strands.Set<Strands>(std::dynamic_pointer_cast<Strands>(asset));
            auto material = ProjectManager::CreateTemporaryAsset<Material>();
            material->SetProgram(DefaultResources::GLPrograms::StandardStrandsProgram);
            strandsRenderer->m_material.Set<Material>(material);
        } else if (asset->GetTypeName() == "EnvironmentalMap") {
            scene->m_environmentSettings.m_environmentalMap =
                    std::dynamic_pointer_cast<EnvironmentalMap>(asset);
        } else if (asset->GetTypeName() == "Cubemap") {
            auto mainCamera = scene->m_mainCamera.Get<Camera>();
            mainCamera->m_skybox = std::dynamic_pointer_cast<Cubemap>(asset);
        }
    }
}

bool EditorLayer::LocalPositionSelected() const {
    return m_localPositionSelected;
}

bool EditorLayer::LocalRotationSelected() const {
    return m_localRotationSelected;
}

bool EditorLayer::LocalScaleSelected() const {
    return m_localScaleSelected;
}

glm::vec3 &EditorLayer::UnsafeGetPreviouslyStoredPosition() {
    return m_previouslyStoredPosition;
}

glm::vec3 &EditorLayer::UnsafeGetPreviouslyStoredRotation() {
    return m_previouslyStoredRotation;
}

glm::vec3 &EditorLayer::UnsafeGetPreviouslyStoredScale() {
    return m_previouslyStoredScale;
}

Entity EditorLayer::GetSelectedEntity() const {
    return m_selectedEntity;
}

void EditorLayer::SetLockEntitySelection(bool value) {
    m_lockEntitySelection = value;
}

bool EditorLayer::GetLockEntitySelection() {
    return m_lockEntitySelection;
}

glm::vec2 EditorLayer::GetMouseScreenPosition() const {
    return m_mouseScreenPosition;
}
