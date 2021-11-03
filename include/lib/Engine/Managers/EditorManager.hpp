#pragma once
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <Core/OpenGLUtils.hpp>
#include <Gui.hpp>
#include <ISingleton.hpp>
#include <RenderTarget.hpp>
#include <RigidBody.hpp>
#include <Texture2D.hpp>

namespace UniEngine
{
struct Transform;

class Folder;
class UNIENGINE_API EditorManager : public ISingleton<EditorManager>
{
    friend class ClassRegistry;
    friend class EditorLayer;
    friend class Application;
    friend class DefaultResources;
    friend class ProjectManager;
    std::map<std::string, std::shared_ptr<Texture2D>> m_assetsIcons;
    bool m_enabled = false;
    std::map<size_t, std::function<void(Entity entity, IDataComponent *data, bool isRoot)>> m_componentDataInspectorMap;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_privateComponentMenuList;
    std::vector<std::pair<size_t, std::function<void(float rank)>>> m_systemMenuList;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_componentDataMenuList;
    template <typename T1 = IPrivateComponent> static void RegisterPrivateComponent();
    template <typename T1 = ISystem> static void RegisterSystem();
    template <typename T1 = IDataComponent> static void RegisterDataComponent();
    static void InitImGui();
    static void ImGuiPreUpdate();
    static void ImGuiLateUpdate();
  public:
    static std::map<std::string, std::shared_ptr<Texture2D>>& AssetIcons();
    std::weak_ptr<IAsset> m_inspectingAsset;
    template <typename T1 = IDataComponent>
    static void RegisterComponentDataInspector(
        const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func);


    static bool DragAndDropButton(
        AssetRef &target,
        const std::string &name,
        const std::vector<std::string> &acceptableTypeNames,
        bool removable = true);
    static bool DragAndDropButton(
        PrivateComponentRef &target,
        const std::string &name,
        const std::vector<std::string> &acceptableTypeNames,
        bool removable = true);

    template <typename T = IAsset>
    static bool DragAndDropButton(AssetRef &target, const std::string &name, bool removable = true);
    template <typename T = IPrivateComponent>
    static bool DragAndDropButton(PrivateComponentRef &target, const std::string &name, bool removable = true);
    static bool DragAndDropButton(EntityRef &entityRef, const std::string &name);
    static bool DragAndDropButton(Entity &entity);

    template <typename T = IAsset> static void DraggableAsset(std::shared_ptr<T> &target);
    template <typename T = IPrivateComponent> static void DraggablePrivateComponent(std::shared_ptr<T> &target);

};

template <typename T1>
void EditorManager::RegisterComponentDataInspector(
    const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func)
{
    GetInstance().m_componentDataInspectorMap.insert_or_assign(typeid(T1).hash_code(), func);
}

template <typename T> void EditorManager::RegisterSystem()
{
    auto &editorManager = GetInstance();
    auto func = [&](float rank) {
        if (EntityManager::GetCurrentScene()->GetSystem<T>())
            return;
        auto systemName = SerializationManager::GetSerializableTypeName<T>();
        if (ImGui::Button(systemName.c_str()))
        {
            EntityManager::GetCurrentScene()->GetOrCreateSystem(systemName, rank);
        }
    };
    for (int i = 0; i < editorManager.m_systemMenuList.size(); i++)
    {
        if (editorManager.m_systemMenuList[i].first == typeid(T).hash_code())
        {
            editorManager.m_systemMenuList[i].second = func;
            return;
        }
    }
    editorManager.m_systemMenuList.emplace_back(typeid(T).hash_code(), func);
}

template <typename T> void EditorManager::RegisterPrivateComponent()
{
    auto &editorManager = GetInstance();
    auto func = [&](Entity owner) {
        if (owner.HasPrivateComponent<T>())
            return;
        if (ImGui::Button(SerializationManager::GetSerializableTypeName<T>().c_str()))
        {
            owner.GetOrSetPrivateComponent<T>();
        }
    };
    for (int i = 0; i < editorManager.m_privateComponentMenuList.size(); i++)
    {
        if (editorManager.m_privateComponentMenuList[i].first == typeid(T).hash_code())
        {
            editorManager.m_privateComponentMenuList[i].second = func;
            return;
        }
    }
    editorManager.m_privateComponentMenuList.emplace_back(typeid(T).hash_code(), func);
}

template <typename T> void EditorManager::RegisterDataComponent()
{
    auto id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code() || id == typeid(GlobalTransform).hash_code() ||
        id == typeid(GlobalTransformUpdateFlag).hash_code())
        return;
    auto func = [](Entity owner) {
        if (owner.HasDataComponent<T>())
            return;
        if (ImGui::Button(SerializationManager::GetDataComponentTypeName<T>().c_str()))
        {
            EntityManager::AddDataComponent<T>(EntityManager::GetCurrentScene(), owner, T());
        }
    };
    for (int i = 0; i < GetInstance().m_componentDataMenuList.size(); i++)
    {
        if (GetInstance().m_componentDataMenuList[i].first == typeid(T).hash_code())
        {
            GetInstance().m_componentDataMenuList[i].second = func;
            return;
        }
    }
    GetInstance().m_componentDataMenuList.emplace_back(typeid(T).hash_code(), func);
}

template <typename T> bool EditorManager::DragAndDropButton(AssetRef &target, const std::string &name, bool removable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    const std::shared_ptr<IAsset> ptr = target.Get<IAsset>();
    bool statusChanged = false;
    ImGui::Button(ptr ? ptr->m_name.c_str() : "none");
    const std::string type = SerializationManager::GetSerializableTypeName<T>();
    if (ptr)
    {
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            GetInstance().m_inspectingAsset = ptr;
        }
        const std::string tag = "##" + type + std::to_string(ptr->GetHandle());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(type.c_str(), &target.m_assetHandle, sizeof(Handle));
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

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(Handle));
            Handle payload_n = *static_cast<Handle *>(payload->Data);
            if (!ptr || payload_n.GetValue() != target.GetAssetHandle().GetValue())
            {
                target.Clear();
                target.m_assetHandle = payload_n;
                target.Update();
                statusChanged = true;
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}

template <typename T>
bool EditorManager::DragAndDropButton(PrivateComponentRef &target, const std::string &name, bool removable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    const std::shared_ptr<IPrivateComponent> ptr = target.Get<IPrivateComponent>();
    ImGui::Button(ptr ? ptr->GetOwner().GetName().c_str() : "none");
    const std::string type = SerializationManager::GetSerializableTypeName<T>();
    if (ptr)
    {
        const std::string tag = "##" + type + std::to_string(ptr->GetHandle());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(type.c_str(), &ptr, sizeof(std::shared_ptr<IPrivateComponent>));
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
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IPrivateComponent>));
            std::shared_ptr<T> payload_n =
                std::dynamic_pointer_cast<T>(*static_cast<std::shared_ptr<IPrivateComponent> *>(payload->Data));
            if (!ptr || payload_n.get() != ptr.get())
            {
                target = payload_n;
                statusChanged = true;
            }
        }
        ImGui::EndDragDropTarget();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
        {
            IM_ASSERT(payload->DataSize == sizeof(Entity));
            Entity payload_n = *static_cast<Entity *>(payload->Data);
            if (payload_n.IsValid() && payload_n.HasPrivateComponent<T>())
            {
                std::shared_ptr<T> received = payload_n.GetOrSetPrivateComponent<T>().lock();
                if (!ptr || received.get() != ptr.get())
                {
                    target = received;
                    statusChanged = true;
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}
template <typename T> void EditorManager::DraggablePrivateComponent(std::shared_ptr<T> &target)
{
    const std::shared_ptr<IPrivateComponent> ptr = std::dynamic_pointer_cast<IPrivateComponent>(target);
    const auto type = ptr->GetTypeName();
    const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(type.c_str(), &ptr, sizeof(std::shared_ptr<IPrivateComponent>));
        ImGui::TextColored(ImVec4(0, 0, 1, 1), (type + tag).c_str());
        ImGui::EndDragDropSource();
    }
    return;
}
template <typename T> void EditorManager::DraggableAsset(std::shared_ptr<T> &target)
{
    const std::shared_ptr<IAsset> ptr = std::dynamic_pointer_cast<IAsset>(target);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        GetInstance().m_inspectingAsset = ptr;
    }
    const auto type = ptr->GetTypeName();
    const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(type.c_str(), &ptr->m_handle, sizeof(Handle));
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
        ImGui::EndPopup();
    }
    return;
}

} // namespace UniEngine
