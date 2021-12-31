#include <Application.hpp>
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <ClassRegistry.hpp>
#include <DefaultResources.hpp>
#include "Editor.hpp"
#include "Engine/Core/Inputs.hpp"
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
#include "Engine/Core/Windows.hpp"
using namespace UniEngine;
std::map<std::string, std::shared_ptr<Texture2D>> &Editor::AssetIcons()
{
    return GetInstance().m_assetsIcons;
}

bool Editor::DragAndDropButton(Entity &entity)
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

bool Editor::DragAndDropButton(EntityRef &entityRef, const std::string &name)
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

bool Editor::DragAndDropButton(
    AssetRef &target, const std::string &name, const std::vector<std::string> &acceptableTypeNames, bool removable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    const std::shared_ptr<IAsset> ptr = target.Get<IAsset>();
    bool statusChanged = false;
    ImGui::Button(ptr ? ptr->m_name.c_str() : "none");
    if (ptr)
    {
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            Editor::GetInstance().m_inspectingAsset = ptr;
        }
        const std::string tag = "##" + ptr->GetTypeName() + std::to_string(ptr->GetHandle());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(ptr->GetTypeName().c_str(), &target.m_assetHandle, sizeof(Handle));
            ImGui::TextColored(ImVec4(0, 0, 1, 1), (ptr->m_name + tag).c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginPopupContextItem(tag.c_str()))
        {
            if (ImGui::BeginMenu(("Rename" + tag).c_str()))
            {
                static char newName[256];
                ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                if (ImGui::Button(("Confirm" + tag).c_str()))
                    ptr->m_name = std::string(newName);
                ImGui::EndMenu();
            }
            if (removable)
            {
                if (ImGui::Button(("Remove" + tag).c_str()))
                {
                    target.Clear();
                    statusChanged = true;
                }
            }
            ImGui::EndPopup();
        }
    }
    for (const auto &typeName : acceptableTypeNames)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(typeName.c_str()))
            {
                IM_ASSERT(payload->DataSize == sizeof(Handle));
                Handle payload_n = *static_cast<Handle *>(payload->Data);
                if (!ptr || payload_n.GetValue() != target.GetAssetHandle().GetValue())
                {
                    target.m_assetHandle = payload_n;
                    target.Update();
                    statusChanged = true;
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
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
void Editor::InitImGui()
{
#pragma region ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#ifdef NDEBUG
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
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
bool Editor::DragAndDropButton(
    PrivateComponentRef &target,
    const std::string &name,
    const std::vector<std::string> &acceptableTypeNames,
    bool removable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    const std::shared_ptr<IPrivateComponent> ptr = target.Get<IPrivateComponent>();
    ImGui::Button(ptr ? ptr->GetOwner().GetName().c_str() : "none");
    if (ptr)
    {
        const std::string tag = "##" + ptr->GetTypeName() + std::to_string(ptr->GetHandle());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(ptr->GetTypeName().c_str(), &ptr, sizeof(std::shared_ptr<IPrivateComponent>));
            ImGui::TextColored(ImVec4(0, 0, 1, 1), (ptr->GetOwner().GetName() + tag).c_str());
            ImGui::EndDragDropSource();
        }
        if (removable)
        {
            if (ImGui::BeginPopupContextItem(tag.c_str()))
            {

                if (ImGui::Button(("Remove" + tag).c_str()))
                {
                    target.Clear();
                    statusChanged = true;
                }
                ImGui::EndPopup();
            }
        }
    }
    for (const auto &type : acceptableTypeNames)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type.c_str()))
            {
                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IPrivateComponent>));
                std::shared_ptr<IPrivateComponent> payload_n =
                    *static_cast<std::shared_ptr<IPrivateComponent> *>(payload->Data);
                if (!ptr || payload_n.get() != ptr.get())
                {
                    target = payload_n;
                    statusChanged = true;
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
    return statusChanged;
}
