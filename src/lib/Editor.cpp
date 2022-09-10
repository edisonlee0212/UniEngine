#include "Editor.hpp"
#include "Engine/Core/Inputs.hpp"
#include "Engine/Core/Windows.hpp"
#include <Application.hpp>
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
#include <ProjectManager.hpp>
#include <RigidBody.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <Utilities.hpp>
using namespace UniEngine;
bool Editor::UnsafeDroppableAsset(AssetRef &target, const std::vector<std::string> &typeNames)
{
    bool statusChanged = false;
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Asset"))
        {
            const std::shared_ptr<IAsset> ptr = target.Get<IAsset>();
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *static_cast<Handle *>(payload->Data);
            if (!ptr || payload_n.GetValue() != target.GetAssetHandle().GetValue())
            {
                auto asset = ProjectManager::GetAsset(payload_n);
                for (const auto &typeName : typeNames)
                {
                    if (asset && asset->GetTypeName() == typeName)
                    {
                        target.Clear();
                        target.m_assetHandle = payload_n;
                        target.Update();
                        statusChanged = true;
                        break;
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}

bool Editor::UnsafeDroppablePrivateComponent(PrivateComponentRef &target, const std::vector<std::string> &typeNames)
{
    bool statusChanged = false;
    if (ImGui::BeginDragDropTarget())
    {
        const auto currentScene = Application::GetActiveScene();
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            auto payload_n = *static_cast<Handle *>(payload->Data);
            auto entity = currentScene->GetEntity(payload_n);
            if (currentScene->IsEntityValid(entity))
            {
                for (const auto &typeName : typeNames)
                {
                    if (currentScene->HasPrivateComponent(entity, typeName))
                    {
                        const auto ptr = target.Get<IPrivateComponent>();
                        auto newPrivateComponent = currentScene->GetPrivateComponent(entity, typeName).lock();
                        target = newPrivateComponent;
                        statusChanged = true;
                        break;
                    }
                }
            }
        }
        else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PrivateComponent"))
        {

            IM_ASSERT(payload->DataSize == sizeof(Handle));
            auto payload_n = *static_cast<Handle *>(payload->Data);
            auto entity = currentScene->GetEntity(payload_n);
            for (const auto &typeName : typeNames)
            {
                if (currentScene->HasPrivateComponent(entity, typeName))
                {
                    target = currentScene->GetPrivateComponent(entity, typeName).lock();
                    statusChanged = true;
                    break;
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}

std::map<std::string, std::shared_ptr<Texture2D>> &Editor::AssetIcons()
{
    return GetInstance().m_assetsIcons;
}

bool Editor::DragAndDropButton(EntityRef &entityRef, const std::string &name, bool modifiable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    auto entity = entityRef.Get();
    if (entity.GetIndex() != 0)
    {
        auto scene = Application::GetActiveScene();
        ImGui::Button(scene->GetEntityName(entity).c_str());
        Draggable(entityRef);
        if (modifiable)
        {
            statusChanged = Rename(entityRef);
            statusChanged = Remove(entityRef) || statusChanged;
        }
    }
    else
    {
        ImGui::Button("none");
    }
    statusChanged = Droppable(entityRef) || statusChanged;
    return statusChanged;
}
bool Editor::Droppable(EntityRef &entityRef)
{
    bool statusChanged = false;
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            auto scene = Application::GetActiveScene();
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            auto payload_n = *static_cast<Handle *>(payload->Data);
            auto newEntity = scene->GetEntity(payload_n);
            if (scene->IsEntityValid(newEntity))
            {
                entityRef = newEntity;
                statusChanged = true;
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}
void Editor::Draggable(EntityRef &entityRef)
{
    auto entity = entityRef.Get();
    if (entity.GetIndex() != 0)
    {
        DraggableEntity(entity);
    }
}
void Editor::DraggableEntity(const Entity &entity)
{
    if (ImGui::BeginDragDropSource())
    {
        auto scene = Application::GetActiveScene();
        auto handle = scene->GetEntityHandle(entity);
        ImGui::SetDragDropPayload("Entity", &handle, sizeof(Handle));
        ImGui::TextColored(ImVec4(0, 0, 1, 1), scene->GetEntityName(entity).c_str());
        ImGui::EndDragDropSource();
    }
}
bool Editor::Rename(EntityRef &entityRef)
{
    auto entity = entityRef.Get();
    bool statusChanged = RenameEntity(entity);
    return statusChanged;
}
bool Editor::Remove(EntityRef &entityRef)
{
    bool statusChanged = false;
    auto entity = entityRef.Get();
    auto scene = Application::GetActiveScene();
    if (scene->IsEntityValid(entity))
    {
        const std::string tag = "##Entity" + std::to_string(scene->GetEntityHandle(entity));
        if (ImGui::BeginPopupContextItem(tag.c_str()))
        {
            if (ImGui::Button(("Remove" + tag).c_str()))
            {
                entityRef.Clear();
                statusChanged = true;
            }
            ImGui::EndPopup();
        }
    }
    return statusChanged;
}
bool Editor::RenameEntity(Entity &entity)
{
    bool statusChanged = false;
    auto scene = Application::GetActiveScene();
    if (scene->IsEntityValid(entity))
    {
        const std::string tag = "##Entity" + std::to_string(scene->GetEntityHandle(entity));
        if (ImGui::BeginPopupContextItem(tag.c_str()))
        {
            if (ImGui::BeginMenu(("Rename" + tag).c_str()))
            {
                static char newName[256];
                ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                if (ImGui::Button(("Confirm" + tag).c_str()))
                {
                    scene->SetEntityName(entity, std::string(newName));
                    memset(newName, 0, 256);
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }
    return statusChanged;
}

bool Editor::DragAndDropButton(
    AssetRef &target, const std::string &name, const std::vector<std::string> &acceptableTypeNames, bool modifiable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    auto ptr = target.Get<IAsset>();
    bool statusChanged = false;
    if (ptr)
    {
        const auto title = ptr->GetTitle();
        ImGui::Button(title.c_str());
        DraggableAsset(ptr);
        if (modifiable)
        {
            statusChanged = Rename(target);
            statusChanged = Remove(target) || statusChanged;
        }
        if (!statusChanged && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            ProjectManager::GetInstance().m_inspectingAsset = ptr;
        }
    }
    else
    {
        ImGui::Button("none");
    }
    statusChanged = UnsafeDroppableAsset(target, acceptableTypeNames) || statusChanged;
    return statusChanged;
}
bool Editor::DragAndDropButton(
    PrivateComponentRef &target,
    const std::string &name,
    const std::vector<std::string> &acceptableTypeNames,
    bool modifiable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    auto ptr = target.Get<IPrivateComponent>();
    if (ptr)
    {
        auto scene = Application::GetActiveScene();
        ImGui::Button(scene->GetEntityName(ptr->GetOwner()).c_str());
        const std::string tag = "##" + ptr->GetTypeName() + std::to_string(ptr->GetHandle());
        DraggablePrivateComponent(ptr);
        if (modifiable)
        {
            statusChanged = Remove(target);
        }
    }
    else
    {
        ImGui::Button("none");
    }
    statusChanged = UnsafeDroppablePrivateComponent(target, acceptableTypeNames) || statusChanged;
    return statusChanged;
}

void Editor::ImGuiPreUpdate()
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
}
void Editor::ImGuiLateUpdate()
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
void Editor::Init(bool docking, bool viewport)
{
#pragma region ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    if (docking)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (viewport)
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL(Windows::GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
#pragma endregion
}
