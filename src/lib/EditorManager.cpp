#include <Application.hpp>
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <Joint.hpp>
#include <Lights.hpp>
#include <MeshRenderer.hpp>
#include <Particles.hpp>
#include <PhysicsManager.hpp>
#include <PlayerController.hpp>
#include <PostProcessing.hpp>
#include <RigidBody.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <WindowManager.hpp>
using namespace UniEngine;
inline bool EditorManager::DrawEntityMenu(const bool &enabled, const Entity &entity)
{
    bool deleted = false;
    if (ImGui::BeginPopupContextItem(std::to_string(entity.GetIndex()).c_str()))
    {
        ImGui::Text(("Handle: " + std::to_string(entity.GetHandle().GetValue())).c_str());
        if (ImGui::Button("Delete"))
        {
            EntityManager::DeleteEntity(entity);
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
                EntityManager::SetEntityName(entity, std::string(newName));

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    return deleted;
}

void EditorManager::InspectComponentData(Entity entity, IDataComponent *data, DataComponentType type, bool isRoot)
{
    if (GetInstance().m_componentDataInspectorMap.find(type.m_typeId) !=
        GetInstance().m_componentDataInspectorMap.end())
    {
        GetInstance().m_componentDataInspectorMap.at(type.m_typeId)(entity, data, isRoot);
    }
}

Entity EditorManager::MouseEntitySelection(const glm::vec2 &mousePosition)
{
    Entity retVal;
    GetInstance().m_sceneCameraEntityRecorder->Bind();
    float entityIndex = 0;
    const glm::vec2 resolution = GetInstance().m_sceneCameraEntityRecorder->GetResolution();
    glm::vec2 point = resolution;
    point.x += mousePosition.x;
    point.y -= mousePosition.y;
    if (point.x >= 0 && point.x < resolution.x && point.y >= 0 && point.y < resolution.y)
    {
        glReadPixels(point.x, point.y, 1, 1, GL_RED, GL_FLOAT, &entityIndex);
        if (entityIndex > 0)
        {
            retVal = EntityManager::GetEntity(static_cast<unsigned>(entityIndex));
        }
    }
    return retVal;
}

void EditorManager::HighLightEntityPrePassHelper(const Entity &entity)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    EntityManager::ForEachChild(entity, [](Entity child) { HighLightEntityPrePassHelper(child); });
    if (entity.HasPrivateComponent<MeshRenderer>())
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            DefaultResources::m_sceneHighlightPrePassProgram->SetFloat4x4(
                "model", EntityManager::GetDataComponent<GlobalTransform>(entity).m_value);
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
                "model", EntityManager::GetDataComponent<GlobalTransform>(entity).m_value);
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
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedPrePassProgram->SetFloat4x4(
                "model", EntityManager::GetDataComponent<GlobalTransform>(entity).m_value);
            skinnedMesh->Draw();
        }
    }
}

void EditorManager::HighLightEntityHelper(const Entity &entity)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    EntityManager::ForEachChild(entity, [](Entity child) { HighLightEntityHelper(child); });
    if (entity.HasPrivateComponent<MeshRenderer>())
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        auto material = mmc->m_material.Get<Material>();
        auto mesh = mmc->m_mesh.Get<Mesh>();
        if (mmc->IsEnabled() && material != nullptr && mesh != nullptr)
        {
            auto ltw = EntityManager::GetDataComponent<GlobalTransform>(entity);
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
            auto ltw = EntityManager::GetDataComponent<GlobalTransform>(entity);
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
            auto ltw = EntityManager::GetDataComponent<GlobalTransform>(entity);
            smmc->m_finalResults->UploadBones(skinnedMesh);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat4x4("model", ltw.m_value);
            DefaultResources::m_sceneHighlightSkinnedProgram->SetFloat3("scale", ltw.GetScale());
            skinnedMesh->Draw();
        }
    }
}

void EditorManager::MoveCamera(
    const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime)
{
    auto &editorManager = GetInstance();
    editorManager.m_previousRotation = editorManager.m_sceneCameraRotation;
    editorManager.m_previousPosition = editorManager.m_sceneCameraPosition;
    editorManager.m_transitionTime = transitionTime;
    editorManager.m_transitionTimer = Application::Time().CurrentTime();
    editorManager.m_targetRotation = targetRotation;
    editorManager.m_targetPosition = targetPosition;
    editorManager.m_lockCamera = true;
    editorManager.m_leftMouseButtonHold = false;
    editorManager.m_rightMouseButtonHold = false;
    editorManager.m_startMouse = false;
}

void EditorManager::HighLightEntity(const Entity &entity, const glm::vec4 &color)
{
    if (!entity.IsValid() || !entity.IsEnabled())
        return;
    auto &manager = GetInstance();
    Camera::m_cameraInfoBlock.UpdateMatrices(
        manager.m_sceneCamera, manager.m_sceneCameraPosition, manager.m_sceneCameraRotation);
    Camera::m_cameraInfoBlock.UploadMatrices(manager.m_sceneCamera);
    manager.m_sceneCamera->Bind();
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
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
    glEnable(GL_DEPTH_TEST);
}

void EditorManager::Init()
{
#pragma region ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL(WindowManager::GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
#pragma endregion

    auto &editorManager = GetInstance();
    editorManager.m_enabled = true;
    editorManager.m_basicEntityArchetype =
        EntityManager::CreateEntityArchetype("General", GlobalTransform(), Transform());

    editorManager.m_sceneCameraEntityRecorderTexture = std::make_unique<OpenGLUtils::GLTexture2D>(
        1, GL_R32F, editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY, false);
    editorManager.m_sceneCameraEntityRecorderTexture->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    editorManager.m_sceneCameraEntityRecorderTexture->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    editorManager.m_sceneCameraEntityRecorderTexture->ReSize(
        0,
        GL_R32F,
        GL_RED,
        GL_FLOAT,
        0,
        editorManager.m_sceneCameraResolutionX,
        editorManager.m_sceneCameraResolutionY);
    editorManager.m_sceneCameraEntityRecorderRenderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    editorManager.m_sceneCameraEntityRecorder =
        std::make_unique<RenderTarget>(editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
    editorManager.m_sceneCameraEntityRecorderRenderBuffer->AllocateStorage(
        GL_DEPTH24_STENCIL8, editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
    editorManager.m_sceneCameraEntityRecorder->SetResolution(
        editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
    editorManager.m_sceneCameraEntityRecorder->AttachRenderBuffer(
        editorManager.m_sceneCameraEntityRecorderRenderBuffer.get(), GL_DEPTH_STENCIL_ATTACHMENT);
    editorManager.m_sceneCameraEntityRecorder->AttachTexture(
        editorManager.m_sceneCameraEntityRecorderTexture.get(), GL_COLOR_ATTACHMENT0);

    RegisterComponentDataInspector<GlobalTransform>([](Entity entity, IDataComponent *data, bool isRoot) {
        auto *ltw = reinterpret_cast<GlobalTransform *>(data);
        glm::vec3 er;
        glm::vec3 t;
        glm::vec3 s;
        ltw->Decompose(t, er, s);
        er = glm::degrees(er);
        ImGui::InputFloat3("Position##Global", &t.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Rotation##Global", &er.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Scale##Global", &s.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
    });
    RegisterComponentDataInspector<Transform>([&](Entity entity, IDataComponent *data, bool isRoot) {
        static Entity previousEntity;
        auto *ltp = static_cast<Transform *>(static_cast<void *>(data));
        bool edited = false;
        bool reload = previousEntity != entity;
        if (Application::IsPlaying() && entity.HasPrivateComponent<RigidBody>())
        {
            auto rigidBody = entity.GetOrSetPrivateComponent<RigidBody>().lock();
            if (rigidBody->m_kinematic && rigidBody->m_currentRegistered)
            {
                reload = true;
            }
        }
        if (reload)
        {
            previousEntity = entity;
            ltp->Decompose(
                editorManager.m_previouslyStoredPosition,
                editorManager.m_previouslyStoredRotation,
                editorManager.m_previouslyStoredScale);
            editorManager.m_previouslyStoredRotation = glm::degrees(editorManager.m_previouslyStoredRotation);
            editorManager.m_localPositionSelected = true;
            editorManager.m_localRotationSelected = false;
            editorManager.m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3("##LocalPosition", &editorManager.m_previouslyStoredPosition.x, 0.1f, 0, 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Position##Local", &editorManager.m_localPositionSelected) &&
            editorManager.m_localPositionSelected)
        {
            editorManager.m_localRotationSelected = false;
            editorManager.m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3("##LocalRotation", &editorManager.m_previouslyStoredRotation.x, 1.0f, 0, 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Rotation##Local", &editorManager.m_localRotationSelected) &&
            editorManager.m_localRotationSelected)
        {
            editorManager.m_localPositionSelected = false;
            editorManager.m_localScaleSelected = false;
        }
        if (ImGui::DragFloat3("##LocalScale", &editorManager.m_previouslyStoredScale.x, 0.01f, 0, 0))
            edited = true;
        ImGui::SameLine();
        if (ImGui::Selectable("Scale##Local", &editorManager.m_localScaleSelected) &&
            editorManager.m_localScaleSelected)
        {
            editorManager.m_localRotationSelected = false;
            editorManager.m_localPositionSelected = false;
        }
        if (edited)
        {
            ltp->m_value = glm::translate(editorManager.m_previouslyStoredPosition) *
                           glm::mat4_cast(glm::quat(glm::radians(editorManager.m_previouslyStoredRotation))) *
                           glm::scale(editorManager.m_previouslyStoredScale);
        }
    });
    RegisterComponentDataInspector<Ray>([&](Entity entity, IDataComponent *data, bool isRoot) {
        auto *ray = static_cast<Ray *>(static_cast<void *>(data));
        ImGui::InputFloat3("Start", &ray->m_start.x);
        ImGui::InputFloat3("Direction", &ray->m_direction.x);
        ImGui::InputFloat("Length", &ray->m_length);
    });

    editorManager.m_selectedEntity = Entity();
    editorManager.m_configFlags += EntityEditorSystem_EnableEntityHierarchy;
    editorManager.m_configFlags += EntityEditorSystem_EnableEntityInspector;

    editorManager.m_sceneCamera = SerializationManager::ProduceSerializable<Camera>();
    editorManager.m_sceneCamera->m_clearColor = glm::vec3(0.5f);
    editorManager.m_sceneCamera->m_useClearColor = false;
    editorManager.m_sceneCamera->OnCreate();
}

void EditorManager::Destroy()
{
}
static const char *HierarchyDisplayMode[]{"Archetype", "Hierarchy"};

void EditorManager::PreUpdate()
{
#pragma region ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
#pragma endregion
#pragma region Dock
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    static bool openDock = true;
    ImGui::Begin("Root DockSpace", &openDock, window_flags);
    ImGui::PopStyleVar();
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();
#pragma endregion
    auto &editorManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::Button(Application::GetInstance().m_playing ? "Pause" : "Play"))
        {
            Application::GetInstance().m_playing = !Application::GetInstance().m_playing;
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Project"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void EditorManager::RenderToSceneCamera()
{
    ProfilerManager::StartEvent("RenderToSceneCamera");
    auto &editorManager = GetInstance();
    auto &renderManager = RenderManager::GetInstance();

    const auto resolution = editorManager.m_sceneCamera->m_gBuffer->GetResolution();
    if (editorManager.m_sceneCameraResolutionX != 0 && editorManager.m_sceneCameraResolutionY != 0 &&
        (resolution.x != editorManager.m_sceneCameraResolutionX ||
         resolution.y != editorManager.m_sceneCameraResolutionY))
    {
        editorManager.m_sceneCamera->ResizeResolution(
            editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
        editorManager.m_sceneCameraEntityRecorderTexture->ReSize(
            0,
            GL_R32F,
            GL_RED,
            GL_FLOAT,
            0,
            editorManager.m_sceneCameraResolutionX,
            editorManager.m_sceneCameraResolutionY);
        editorManager.m_sceneCameraEntityRecorderRenderBuffer->AllocateStorage(
            GL_DEPTH24_STENCIL8, editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
        editorManager.m_sceneCameraEntityRecorder->SetResolution(
            editorManager.m_sceneCameraResolutionX, editorManager.m_sceneCameraResolutionY);
    }
    editorManager.m_sceneCamera->Clear();
    editorManager.m_sceneCameraEntityRecorder->Clear();
    if (editorManager.m_lockCamera)
    {
        const float elapsedTime = Application::Time().CurrentTime() - editorManager.m_transitionTimer;
        float a = 1.0f - glm::pow(1.0 - elapsedTime / editorManager.m_transitionTime, 4.0f);
        if (elapsedTime >= editorManager.m_transitionTime)
            a = 1.0f;
        editorManager.m_sceneCameraRotation =
            glm::mix(editorManager.m_previousRotation, editorManager.m_targetRotation, a);
        editorManager.m_sceneCameraPosition =
            glm::mix(editorManager.m_previousPosition, editorManager.m_targetPosition, a);
        if (a >= 1.0f)
        {
            editorManager.m_lockCamera = false;
            editorManager.m_sceneCameraRotation = editorManager.m_targetRotation;
            editorManager.m_sceneCameraPosition = editorManager.m_targetPosition;
            Camera::ReverseAngle(
                editorManager.m_targetRotation,
                editorManager.m_sceneCameraPitchAngle,
                editorManager.m_sceneCameraYawAngle);
        }
    }

    if (editorManager.m_enabled && editorManager.m_sceneCamera->IsEnabled())
    {
        Camera::m_cameraInfoBlock.UpdateMatrices(
            editorManager.m_sceneCamera, editorManager.m_sceneCameraPosition, editorManager.m_sceneCameraRotation);
        Camera::m_cameraInfoBlock.UploadMatrices(editorManager.m_sceneCamera);
#pragma region For entity selection
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        editorManager.m_sceneCameraEntityRecorder->Bind();
        for (auto &i : renderManager.m_deferredRenderInstances)
        {
            const auto &cameraComponent = i.first;
            RenderManager::DispatchRenderCommands(
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
        for (auto &i : renderManager.m_deferredInstancedRenderInstances)
        {
            RenderManager::DispatchRenderCommands(
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
        for (auto &i : renderManager.m_forwardRenderInstances)
        {
            RenderManager::DispatchRenderCommands(
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
        for (auto &i : renderManager.m_forwardInstancedRenderInstances)
        {
            RenderManager::DispatchRenderCommands(
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
        RenderManager::ApplyShadowMapSettings();
        RenderManager::ApplyEnvironmentalSettings(editorManager.m_sceneCamera);
        RenderManager::RenderToCamera(editorManager.m_sceneCamera);
    }
    ProfilerManager::EndEvent("RenderToSceneCamera");
}

Entity EditorManager::GetSelectedEntity()
{
    return GetInstance().m_selectedEntity;
}

void EditorManager::DrawEntityNode(const Entity &entity, const unsigned &hierarchyLevel)
{
    auto &manager = GetInstance();
    std::string title = std::to_string(entity.GetIndex()) + ": ";
    title += entity.GetName();
    const bool enabled = entity.IsEnabled();
    if (enabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({1, 1, 1, 1}));
    }
    const int index = manager.m_selectedEntityHierarchyList.size() - hierarchyLevel - 1;
    if (!manager.m_selectedEntityHierarchyList.empty() && index >= 0 &&
        index < manager.m_selectedEntityHierarchyList.size() && manager.m_selectedEntityHierarchyList[index] == entity)
    {
        ImGui::SetNextItemOpen(true);
    }
    const bool opened = ImGui::TreeNodeEx(
        title.c_str(),
        ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_NoAutoOpenOnLog |
            (GetInstance().m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed : ImGuiTreeNodeFlags_FramePadding));
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
            EntityManager::SetParent(*static_cast<Entity *>(payload->Data), entity, true);
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
        EntityManager::ForEachChild(entity, [=](Entity child) { DrawEntityNode(child, hierarchyLevel + 1); });
        ImGui::TreePop();
    }
}
void EditorManager::OnInspect()
{
    auto &editorManager = GetInstance();
    if (editorManager.m_leftMouseButtonHold &&
        !InputManager::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, WindowManager::GetWindow()))
    {
        editorManager.m_leftMouseButtonHold = false;
    }
    if (editorManager.m_rightMouseButtonHold &&
        !InputManager::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, WindowManager::GetWindow()))
    {
        editorManager.m_rightMouseButtonHold = false;
        editorManager.m_startMouse = false;
    }
#pragma region Entity Explorer
    if (editorManager.m_configFlags & EntityEditorSystem_EnableEntityHierarchy)
    {
        ImGui::Begin("Entity Explorer");
        if (ImGui::BeginPopupContextWindow("DataComponentInspectorPopup"))
        {
            if (ImGui::Button("Create new entity"))
            {
                auto newEntity = EntityManager::CreateEntity(editorManager.m_basicEntityArchetype);
            }
            ImGui::EndPopup();
        }
        ImGui::Combo(
            "Display mode",
            &editorManager.m_selectedHierarchyDisplayMode,
            HierarchyDisplayMode,
            IM_ARRAYSIZE(HierarchyDisplayMode));
        std::string title = EntityManager::GetCurrentScene()->m_name;
        if (!EntityManager::GetCurrentScene()->m_saved)
        {
            title += " *";
        }
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(Entity));
                    Entity payload_n = *static_cast<Entity *>(payload->Data);
                    auto parent = EntityManager::GetParent(payload_n);
                    if (!parent.IsNull())
                        EntityManager::RemoveChild(payload_n, parent);
                }
                ImGui::EndDragDropTarget();
            }
            if (editorManager.m_selectedHierarchyDisplayMode == 0)
            {
                EntityManager::UnsafeForEachEntityStorage(
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
                                        (editorManager.m_selectedEntity == entity ? ImGuiTreeNodeFlags_Framed
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
            else if (editorManager.m_selectedHierarchyDisplayMode == 1)
            {
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
                EntityManager::ForAllEntities([](int i, Entity entity) {
                    if (EntityManager::GetParent(entity).IsNull())
                        DrawEntityNode(entity, 0);
                });
                editorManager.m_selectedEntityHierarchyList.clear();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
    }
#pragma endregion
#pragma region Entity Inspector
    if (editorManager.m_configFlags & EntityEditorSystem_EnableEntityInspector)
    {
        ImGui::Begin("Entity Inspector");
        if (editorManager.m_selectedEntity.IsValid())
        {
            std::string title = std::to_string(editorManager.m_selectedEntity.GetIndex()) + ": ";
            title += editorManager.m_selectedEntity.GetName();
            bool enabled = editorManager.m_selectedEntity.IsEnabled();
            if (ImGui::Checkbox((title + "##EnabledCheckbox").c_str(), &enabled))
            {
                if (editorManager.m_selectedEntity.IsEnabled() != enabled)
                {
                    editorManager.m_selectedEntity.SetEnabled(enabled);
                }
            }
            ImGui::SameLine();
            bool deleted = DrawEntityMenu(editorManager.m_selectedEntity.IsEnabled(), editorManager.m_selectedEntity);
            ImGui::Separator();
            if (!deleted)
            {
                if (ImGui::CollapsingHeader("Data components", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::BeginPopupContextItem("DataComponentInspectorPopup"))
                    {
                        ImGui::Text("Add data component: ");
                        ImGui::Separator();
                        for (auto &i : editorManager.m_componentDataMenuList)
                        {
                            i.second(editorManager.m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }
                    bool skip = false;
                    int i = 0;
                    EntityManager::UnsafeForEachDataComponent(
                        editorManager.m_selectedEntity,
                        [&skip, &i, &editorManager](DataComponentType type, void *data) {
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
                                    EntityManager::RemoveDataComponent(editorManager.m_selectedEntity, type.m_typeId);
                                }
                                ImGui::EndPopup();
                            }
                            ImGui::PopID();
                            InspectComponentData(
                                editorManager.m_selectedEntity,
                                static_cast<IDataComponent *>(data),
                                type,
                                EntityManager::GetParent(editorManager.m_selectedEntity).IsNull());
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
                        for (auto &i : editorManager.m_privateComponentMenuList)
                        {
                            i.second(editorManager.m_selectedEntity);
                        }
                        ImGui::Separator();
                        ImGui::EndPopup();
                    }

                    int i = 0;
                    bool skip = false;
                    EntityManager::ForEachPrivateComponent(
                        editorManager.m_selectedEntity, [&i, &skip, &editorManager](PrivateComponentElement &data) {
                            if (skip)
                                return;
                            ImGui::Checkbox(
                                data.m_privateComponentData->GetTypeName().c_str(),
                                &data.m_privateComponentData->m_enabled);
                            DraggablePrivateComponent(data.m_privateComponentData);
                            const std::string tag = "##" + data.m_privateComponentData->GetTypeName() +
                                                    std::to_string(data.m_privateComponentData->GetHandle());
                            if (ImGui::BeginPopupContextItem(tag.c_str()))
                            {
                                if (ImGui::Button(("Remove" + tag).c_str()))
                                {
                                    skip = true;
                                    EntityManager::RemovePrivateComponent(
                                        editorManager.m_selectedEntity, data.m_typeId);
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
            editorManager.m_selectedEntity = Entity();
        }
        ImGui::End();
    }
#pragma endregion
    if (InputManager::GetKeyInternal(GLFW_KEY_DELETE, WindowManager::GetWindow()))
    {
        if (editorManager.m_selectedEntity.IsValid())
        {
            EntityManager::DeleteEntity(editorManager.m_selectedEntity);
        }
    }
    MainCameraWindow();
    SceneCameraWindow();


#pragma region Asset Inspection
    ImGui::Begin("Asset Inspector");
    if (!editorManager.m_inspectingAsset.expired())
    {
        editorManager.m_inspectingAsset.lock()->OnInspect();
    }
    else
    {
        ImGui::Text("None");
    }
    ImGui::End();
#pragma endregion
#pragma region System Inspection
    ImGui::Begin("System Inspector");
    for (auto &i : EntityManager::GetCurrentScene().get()->m_systems)
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
    ImGui::End();
#pragma endregion
}
void EditorManager::LateUpdate()
{
#pragma region ImGui
    RenderTarget::BindDefault();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this
    // code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
#pragma endregion
}

void EditorManager::SetSelectedEntity(const Entity &entity, bool openMenu)
{
    auto &manager = GetInstance();
    if (entity == manager.m_selectedEntity)
        return;
    manager.m_selectedEntityHierarchyList.clear();
    if (entity.IsNull())
    {
        manager.m_selectedEntity = Entity();
        return;
    }
    if (!entity.IsValid())
        return;
    GetInstance().m_selectedEntity = entity;
    if (!openMenu)
        return;
    auto walker = entity;
    while (!walker.IsNull())
    {
        manager.m_selectedEntityHierarchyList.push_back(walker);
        walker = EntityManager::GetParent(walker);
    }
}

std::weak_ptr<Camera> EditorManager::GetSceneCamera()
{
    return GetInstance().m_sceneCamera;
}

bool EditorManager::DragAndDropButton(Entity &entity)
{
    bool statusChanged = false;
    // const std::string type = "Entity";
    ImGui::Button(entity.IsValid() ? entity.GetName().c_str() : "none");
    if (entity.IsValid())
    {
        const std::string tag = "##Entity" + std::to_string(entity.GetIndex());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("Entity", &entity, sizeof(Entity));
            ImGui::TextColored(ImVec4(0, 0, 1, 1), (entity.GetName() + tag).c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginPopupContextItem(tag.c_str()))
        {
            if (ImGui::BeginMenu(("Rename" + tag).c_str()))
            {
                static char newName[256];
                ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                if (ImGui::Button(("Confirm" + tag).c_str()))
                    entity.SetName(std::string(newName));
                ImGui::EndMenu();
            }
            if (ImGui::Button(("Remove" + tag).c_str()))
            {
                entity = Entity();
                statusChanged = true;
            }
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(Entity));
            entity = *static_cast<Entity *>(payload->Data);
            statusChanged = true;
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}
bool EditorManager::MainCameraWindowFocused()
{
    return GetInstance().m_mainCameraWindowFocused;
}
bool EditorManager::SceneCameraWindowFocused()
{
    return GetInstance().m_sceneCameraWindowFocused;
}
void EditorManager::SceneCameraWindow()
{
    auto &editorManager = GetInstance();
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
#pragma region Menu
                    if (ImGui::Button("Reset camera"))
                    {
                        MoveCamera(
                            editorManager.m_defaultSceneCameraRotation, editorManager.m_defaultSceneCameraPosition);
                    }
                    if (ImGui::Button("Set default"))
                    {
                        editorManager.m_defaultSceneCameraPosition = editorManager.m_sceneCameraPosition;
                        editorManager.m_defaultSceneCameraRotation = editorManager.m_sceneCameraRotation;
                    }
                    ImGui::SliderFloat("FOV", &editorManager.m_sceneCamera->m_fov, 30.0f, 120.0f);
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
            editorManager.m_sceneCameraResolutionX = viewPortSize.x;
            editorManager.m_sceneCameraResolutionY = viewPortSize.y;

            bool cameraActive = false;
            if (!RenderManager::GetMainCamera().expired())
            {
                auto mainCamera = RenderManager::GetMainCamera().lock();
                auto entity = mainCamera->GetOwner();
                if (entity.IsEnabled() && mainCamera->IsEnabled())
                {
                    // Because I use the texture from OpenGL, I need to invert the V from the UV.
                    ImGui::Image(
                        (ImTextureID)editorManager.m_sceneCamera->GetTexture()->UnsafeGetGLTexture()->Id(),
                        viewPortSize,
                        ImVec2(0, 1),
                        ImVec2(1, 0));
                    CameraWindowDragAndDrop();

                    cameraActive = true;
                }
            }
            if (!cameraActive)
            {
                ImGui::Text("No active main camera!");
            }

            glm::vec2 mousePosition = glm::vec2(FLT_MAX, FLT_MIN);
            if (editorManager.m_sceneCameraWindowFocused)
            {
                bool valid = InputManager::GetMousePositionInternal(ImGui::GetCurrentWindowRead(), mousePosition);
                float xOffset = 0;
                float yOffset = 0;
                if (valid)
                {
                    if (!editorManager.m_startMouse)
                    {
                        editorManager.m_lastX = mousePosition.x;
                        editorManager.m_lastY = mousePosition.y;
                        editorManager.m_startMouse = true;
                    }
                    xOffset = mousePosition.x - editorManager.m_lastX;
                    yOffset = -mousePosition.y + editorManager.m_lastY;
                    editorManager.m_lastX = mousePosition.x;
                    editorManager.m_lastY = mousePosition.y;
#pragma region Scene Camera Controller
                    if (!editorManager.m_rightMouseButtonHold &&
                        !(mousePosition.x > 0 || mousePosition.y < 0 || mousePosition.x < -viewPortSize.x ||
                          mousePosition.y > viewPortSize.y) &&
                        InputManager::GetMouseInternal(GLFW_MOUSE_BUTTON_RIGHT, WindowManager::GetWindow()))
                    {
                        editorManager.m_rightMouseButtonHold = true;
                    }
                    if (editorManager.m_rightMouseButtonHold && !editorManager.m_lockCamera)
                    {
                        glm::vec3 front = editorManager.m_sceneCameraRotation * glm::vec3(0, 0, -1);
                        glm::vec3 right = editorManager.m_sceneCameraRotation * glm::vec3(1, 0, 0);
                        if (InputManager::GetKeyInternal(GLFW_KEY_W, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition +=
                                front * static_cast<float>(Application::Time().DeltaTime()) * editorManager.m_velocity;
                        }
                        if (InputManager::GetKeyInternal(GLFW_KEY_S, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition -=
                                front * static_cast<float>(Application::Time().DeltaTime()) * editorManager.m_velocity;
                        }
                        if (InputManager::GetKeyInternal(GLFW_KEY_A, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition -=
                                right * static_cast<float>(Application::Time().DeltaTime()) * editorManager.m_velocity;
                        }
                        if (InputManager::GetKeyInternal(GLFW_KEY_D, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition +=
                                right * static_cast<float>(Application::Time().DeltaTime()) * editorManager.m_velocity;
                        }
                        if (InputManager::GetKeyInternal(GLFW_KEY_LEFT_SHIFT, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition.y +=
                                editorManager.m_velocity * static_cast<float>(Application::Time().DeltaTime());
                        }
                        if (InputManager::GetKeyInternal(GLFW_KEY_LEFT_CONTROL, WindowManager::GetWindow()))
                        {
                            editorManager.m_sceneCameraPosition.y -=
                                editorManager.m_velocity * static_cast<float>(Application::Time().DeltaTime());
                        }
                        if (xOffset != 0.0f || yOffset != 0.0f)
                        {
                            editorManager.m_sceneCameraYawAngle += xOffset * editorManager.m_sensitivity;
                            editorManager.m_sceneCameraPitchAngle += yOffset * editorManager.m_sensitivity;
                            if (editorManager.m_sceneCameraPitchAngle > 89.0f)
                                editorManager.m_sceneCameraPitchAngle = 89.0f;
                            if (editorManager.m_sceneCameraPitchAngle < -89.0f)
                                editorManager.m_sceneCameraPitchAngle = -89.0f;

                            editorManager.m_sceneCameraRotation = Camera::ProcessMouseMovement(
                                editorManager.m_sceneCameraYawAngle, editorManager.m_sceneCameraPitchAngle, false);
                        }
                    }
#pragma endregion
                }
            }
#pragma region Gizmos and Entity Selection
            bool mouseSelectEntity = true;
            if (editorManager.m_selectedEntity.IsValid())
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewPortSize.x, viewPortSize.y);
                glm::mat4 cameraView = glm::inverse(
                    glm::translate(editorManager.m_sceneCameraPosition) *
                    glm::mat4_cast(editorManager.m_sceneCameraRotation));
                glm::mat4 cameraProjection = editorManager.m_sceneCamera->GetProjection();
                const auto op = editorManager.m_localPositionSelected   ? ImGuizmo::OPERATION::TRANSLATE
                                : editorManager.m_localRotationSelected ? ImGuizmo::OPERATION::ROTATE
                                                                        : ImGuizmo::OPERATION::SCALE;

                auto transform = editorManager.m_selectedEntity.GetDataComponent<Transform>();
                GlobalTransform parentGlobalTransform;
                Entity parentEntity = EntityManager::GetParent(editorManager.m_selectedEntity);
                if (!parentEntity.IsNull())
                {
                    parentGlobalTransform =
                        EntityManager::GetParent(editorManager.m_selectedEntity).GetDataComponent<GlobalTransform>();
                }
                auto globalTransform = editorManager.m_selectedEntity.GetDataComponent<GlobalTransform>();
                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView),
                    glm::value_ptr(cameraProjection),
                    op,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(globalTransform.m_value));
                if (ImGuizmo::IsUsing())
                {
                    transform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
                    editorManager.m_selectedEntity.SetDataComponent(transform);
                    transform.Decompose(
                        editorManager.m_previouslyStoredPosition,
                        editorManager.m_previouslyStoredRotation,
                        editorManager.m_previouslyStoredScale);
                    mouseSelectEntity = false;
                }
            }
            if (editorManager.m_sceneCameraWindowFocused && mouseSelectEntity)
            {
                if (!editorManager.m_leftMouseButtonHold &&
                    !(mousePosition.x > 0 || mousePosition.y < 0 || mousePosition.x < -viewPortSize.x ||
                      mousePosition.y > viewPortSize.y) &&
                    InputManager::GetMouseInternal(GLFW_MOUSE_BUTTON_LEFT, WindowManager::GetWindow()))
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
                            if (walker == editorManager.m_selectedEntity)
                            {
                                found = true;
                                break;
                            }
                            walker = EntityManager::GetParent(walker);
                        }
                        if (found)
                        {
                            walker = EntityManager::GetParent(walker);
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
                    editorManager.m_leftMouseButtonHold = true;
                }
            }
            HighLightEntity(editorManager.m_selectedEntity, glm::vec4(1.0, 0.5, 0.0, 0.8));
#pragma endregion
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        {
            if (!editorManager.m_sceneCameraWindowFocused)
            {
                editorManager.m_rightMouseButtonHold = true;
                editorManager.m_leftMouseButtonHold = true;
            }
            editorManager.m_sceneCameraWindowFocused = true;
        }
        else
        {
            editorManager.m_sceneCameraWindowFocused = false;
        }
        ImGui::EndChild();
    }
    else
    {
        editorManager.m_sceneCameraWindowFocused = false;
    }
    editorManager.m_sceneCamera->SetEnabled(
        !(ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed));
    ImGui::End();
    ImGui::PopStyleVar();

#pragma endregion
}
void EditorManager::MainCameraWindow()
{
    auto &renderManager = RenderManager::GetInstance();
    auto &editorManager = GetInstance();
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
                    ImGui::Checkbox("Display info", &renderManager.m_enableInfoWindow);
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
            renderManager.m_mainCameraResolutionX = viewPortSize.x;
            renderManager.m_mainCameraResolutionY = viewPortSize.y;
            // UNIENGINE_LOG(std::to_string(viewPortSize.x) + ", " + std::to_string(viewPortSize.y));
            //  Get the size of the child (i.e. the whole draw size of the windows).
            ImVec2 overlayPos = ImGui::GetWindowPos();
            // Because I use the texture from OpenGL, I need to invert the V from the UV.
            bool cameraActive = false;
            if (!RenderManager::GetMainCamera().expired())
            {
                auto mainCamera = RenderManager::GetMainCamera().lock();
                auto entity = mainCamera->GetOwner();
                if (entity.IsEnabled() && mainCamera->IsEnabled())
                {
                    auto id = mainCamera->GetTexture()->UnsafeGetGLTexture()->Id();
                    ImGui::Image((ImTextureID)id, viewPortSize, ImVec2(0, 1), ImVec2(1, 0));
                    CameraWindowDragAndDrop();
                    cameraActive = true;
                }
            }
            if (!cameraActive)
            {
                ImGui::Text("No active main camera!");
            }

            ImVec2 window_pos = ImVec2(
                (corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
                (corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y) + 20);
            if (renderManager.m_enableInfoWindow)
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
                    if (renderManager.m_triangles < 999)
                        trisstr += std::to_string(renderManager.m_triangles);
                    else if (renderManager.m_triangles < 999999)
                        trisstr += std::to_string((int)(renderManager.m_triangles / 1000)) + "K";
                    else
                        trisstr += std::to_string((int)(renderManager.m_triangles / 1000000)) + "M";
                    trisstr += " tris";
                    ImGui::Text(trisstr.c_str());
                    ImGui::Text("%d drawcall", renderManager.m_drawCall);
                    ImGui::Separator();
                    if (ImGui::IsMousePosValid())
                    {
                        glm::vec2 pos;
                        InputManager::GetMousePositionInternal(ImGui::GetCurrentWindowRead(), pos);
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
            if (!editorManager.m_mainCameraWindowFocused)
            {
                // editorManager.m_rightMouseButtonHold = true;
                // editorManager.m_leftMouseButtonHold = true;
            }
            editorManager.m_mainCameraWindowFocused = true;
        }
        else
        {
            editorManager.m_mainCameraWindowFocused = false;
        }

        ImGui::EndChild();
    }
    else
    {
        editorManager.m_mainCameraWindowFocused = false;
    }
    renderManager.m_mainCameraViewable =
        !(ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed);
    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion
}
void EditorManager::CameraWindowDragAndDrop()
{
    if (ImGui::BeginDragDropTarget())
    {
        const std::string sceneTypeHash = SerializationManager::GetSerializableTypeName<Scene>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(sceneTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
            std::shared_ptr<Scene> payload_n =
                std::dynamic_pointer_cast<Scene>(*(std::shared_ptr<IAsset> *)payload->Data);
            EntityManager::Attach(payload_n);
        }
        const std::string modelTypeHash = SerializationManager::GetSerializableTypeName<Prefab>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(modelTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
            std::shared_ptr<Prefab> payload_n =
                std::dynamic_pointer_cast<Prefab>(*(std::shared_ptr<IAsset> *)payload->Data);
            EntityArchetype archetype = EntityManager::CreateEntityArchetype("Default", Transform(), GlobalTransform());
            payload_n->ToEntity();
        }
        const std::string texture2DTypeHash = SerializationManager::GetSerializableTypeName<Texture2D>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(texture2DTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
            std::shared_ptr<Texture2D> payload_n =
                std::dynamic_pointer_cast<Texture2D>(*(std::shared_ptr<IAsset> *)payload->Data);
            EntityArchetype archetype = EntityManager::CreateEntityArchetype("Default", Transform(), GlobalTransform());
            AssetManager::ToEntity(archetype, payload_n);
        }
        const std::string meshTypeHash = SerializationManager::GetSerializableTypeName<Mesh>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(meshTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
            std::shared_ptr<Mesh> payload_n =
                std::dynamic_pointer_cast<Mesh>(*(std::shared_ptr<IAsset> *)payload->Data);
            Entity entity = EntityManager::CreateEntity("Mesh");
            auto meshRenderer = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
            meshRenderer->m_mesh.Set<Mesh>(payload_n);
            auto material = AssetManager::CreateAsset<Material>();
            material->SetProgram(DefaultResources::GLPrograms::StandardProgram);
            meshRenderer->m_material.Set<Material>(material);
        }

        const std::string environmentalMapTypeHash = SerializationManager::GetSerializableTypeName<EnvironmentalMap>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(environmentalMapTypeHash.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
            std::shared_ptr<EnvironmentalMap> payload_n =
                std::dynamic_pointer_cast<EnvironmentalMap>(*(std::shared_ptr<IAsset> *)payload->Data);
            RenderManager::GetInstance().m_environmentalMap = payload_n;
        }

        const std::string cubeMapTypeHash = SerializationManager::GetSerializableTypeName<Cubemap>();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(cubeMapTypeHash.c_str()))
        {
            if (!RenderManager::GetMainCamera().expired())
            {
                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
                std::shared_ptr<Cubemap> payload_n =
                    std::dynamic_pointer_cast<Cubemap>(*(std::shared_ptr<IAsset> *)payload->Data);
                auto mainCamera = RenderManager::GetMainCamera().lock();
                mainCamera->m_skybox = payload_n;
            }
        }
        ImGui::EndDragDropTarget();
    }
}
bool EditorManager::DragAndDropButton(EntityRef &entityRef, const std::string &name)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    auto entity = entityRef.Get();
    ImGui::Button(!entity.IsNull() ? entity.GetName().c_str() : "none");
    if (!entity.IsNull())
    {
        const std::string tag = "##Entity" + std::to_string(entity.GetIndex());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("Entity", &entity, sizeof(Entity));
            ImGui::TextColored(ImVec4(0, 0, 1, 1), (entity.GetName() + tag).c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginPopupContextItem(tag.c_str()))
        {
            if (ImGui::BeginMenu(("Rename" + tag).c_str()))
            {
                static char newName[256];
                ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                if (ImGui::Button(("Confirm" + tag).c_str()))
                    entity.SetName(std::string(newName));
                ImGui::EndMenu();
            }
            if (ImGui::Button(("Remove" + tag).c_str()))
            {
                entityRef.Clear();
                statusChanged = true;
            }
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(Entity));
            auto payload_n = *static_cast<Entity *>(payload->Data);
            if (entity != payload_n)
            {
                entityRef = payload_n;
                statusChanged = true;
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}
