#pragma once
#include "Application.hpp"
#include "Camera.hpp"
#include "ISingleton.hpp"
#include "OpenGLUtils.hpp"
#include "ProjectManager.hpp"
#include "RenderTarget.hpp"
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "Texture2D.hpp"
namespace UniEngine
{
struct Transform;

class Folder;
class UNIENGINE_API Editor : public ISingleton<Editor>
{
    friend class ClassRegistry;
    friend class EditorLayer;
    friend class Application;
    friend class DefaultResources;
    friend class ProjectManager;
    friend class Scene;
    std::map<std::string, std::shared_ptr<Texture2D>> m_assetsIcons;
    bool m_enabled = false;
    std::map<size_t, std::function<bool(Entity entity, IDataComponent *data, bool isRoot)>> m_componentDataInspectorMap;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_privateComponentMenuList;
    std::vector<std::pair<size_t, std::function<void(float rank)>>> m_systemMenuList;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_componentDataMenuList;
    template <typename T1 = IPrivateComponent> static void RegisterPrivateComponent();
    template <typename T1 = ISystem> static void RegisterSystem();
    template <typename T1 = IDataComponent> static void RegisterDataComponent();
    static void Init(bool docking, bool viewport);
    static void ImGuiPreUpdate();
    static void ImGuiLateUpdate();

    std::vector<std::weak_ptr<AssetRecord>> m_assetRecordBus;
    std::map<std::string, std::vector<AssetRef>> m_assetRefBus;
    std::map<std::string, std::vector<PrivateComponentRef>> m_privateComponentRefBus;
    std::map<std::string, std::vector<EntityRef>> m_entityRefBus;

  public:
    static std::map<std::string, std::shared_ptr<Texture2D>> &AssetIcons();

    template <typename T1 = IDataComponent>
    static void RegisterComponentDataInspector(
        const std::function<bool(Entity entity, IDataComponent *data, bool isRoot)> &func);

    static bool DragAndDropButton(
        AssetRef &target,
        const std::string &name,
        const std::vector<std::string> &acceptableTypeNames,
        bool modifiable = true);
    static bool DragAndDropButton(
        PrivateComponentRef &target,
        const std::string &name,
        const std::vector<std::string> &acceptableTypeNames,
        bool modifiable = true);

    template <typename T = IAsset>
    static bool DragAndDropButton(AssetRef &target, const std::string &name, bool modifiable = true);
    template <typename T = IPrivateComponent>
    static bool DragAndDropButton(PrivateComponentRef &target, const std::string &name, bool modifiable = true);
    static bool DragAndDropButton(EntityRef &entityRef, const std::string &name, bool modifiable = true);

    template <typename T = IAsset> static void Draggable(AssetRef &target);
    template <typename T = IPrivateComponent> static void Draggable(PrivateComponentRef &target);
    static void Draggable(EntityRef &entityRef);

    template <typename T = IAsset> static void DraggableAsset(const std::shared_ptr<T> &target);
    template <typename T = IPrivateComponent> static void DraggablePrivateComponent(const std::shared_ptr<T> &target);
    static void DraggableEntity(const Entity &entity);

    static bool UnsafeDroppableAsset(AssetRef &target, const std::vector<std::string> &typeNames);
    static bool UnsafeDroppablePrivateComponent(PrivateComponentRef &target, const std::vector<std::string> &typeNames);

    template <typename T = IAsset> static bool Droppable(AssetRef &target);
    template <typename T = IPrivateComponent> static bool Droppable(PrivateComponentRef &target);
    static bool Droppable(EntityRef &entityRef);

    template <typename T = IAsset> static bool Rename(AssetRef &target);
    static bool Rename(EntityRef &entityRef);

    template <typename T = IAsset> static bool RenameAsset(const std::shared_ptr<T> &target);
    static bool RenameEntity(Entity &entity);

    template <typename T = IAsset> static bool Remove(AssetRef &target);
    template <typename T = IPrivateComponent> static bool Remove(PrivateComponentRef &target);
    static bool Remove(EntityRef &entityRef);
};

template <typename T1>
void Editor::RegisterComponentDataInspector(
    const std::function<bool(Entity entity, IDataComponent *data, bool isRoot)> &func)
{
    GetInstance().m_componentDataInspectorMap.insert_or_assign(typeid(T1).hash_code(), func);
}

template <typename T> void Editor::RegisterSystem()
{
    auto &editorManager = GetInstance();
    auto scene = Application::GetActiveScene();
    auto func = [&](float rank) {
        if (scene->GetSystem<T>())
            return;
        auto systemName = Serialization::GetSerializableTypeName<T>();
        if (ImGui::Button(systemName.c_str()))
        {
            scene->GetOrCreateSystem(systemName, rank);
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

template <typename T> void Editor::RegisterPrivateComponent()
{
    auto &editorManager = GetInstance();
    auto func = [&](Entity owner) {
        auto scene = Application::GetActiveScene();
        if (scene->HasPrivateComponent<T>(owner))
            return;
        if (ImGui::Button(Serialization::GetSerializableTypeName<T>().c_str()))
        {
            scene->GetOrSetPrivateComponent<T>(owner);
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

template <typename T> void Editor::RegisterDataComponent()
{
    auto id = typeid(T).hash_code();
    if (id == typeid(Transform).hash_code() || id == typeid(GlobalTransform).hash_code() ||
        id == typeid(GlobalTransformUpdateFlag).hash_code())
        return;
    auto func = [](Entity owner) {
        auto scene = Application::GetActiveScene();
        if (scene->HasPrivateComponent<T>(owner))
            return;
        if (ImGui::Button(Serialization::GetDataComponentTypeName<T>().c_str()))
        {
            scene->AddDataComponent<T>(owner, T());
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

template <typename T> bool Editor::DragAndDropButton(AssetRef &target, const std::string &name, bool modifiable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    const auto ptr = target.Get<IAsset>();
    bool statusChanged = false;
    if (ptr)
    {
        ImGui::Button(ptr->GetTitle().c_str());
        Draggable(target);
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
    statusChanged = Droppable<T>(target) || statusChanged;
    return statusChanged;
}

template <typename T>
bool Editor::DragAndDropButton(PrivateComponentRef &target, const std::string &name, bool modifiable)
{
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    bool statusChanged = false;
    const auto ptr = target.Get<IPrivateComponent>();
    if (ptr)
    {
        auto scene = Application::GetActiveScene();
        if(!scene->IsEntityValid(ptr->GetOwner()))
        {
            target.Clear();
            ImGui::Button("none");
            return true;
        }
        ImGui::Button(scene->GetEntityName(ptr->GetOwner()).c_str());
        Draggable(target);
        if (modifiable)
        {
            statusChanged = Remove(target) || statusChanged;
        }
    }
    else
    {
        ImGui::Button("none");
    }
    statusChanged = Droppable<T>(target) || statusChanged;
    return statusChanged;
}
template <typename T> void Editor::DraggablePrivateComponent(const std::shared_ptr<T> &target)
{
    const auto ptr = std::dynamic_pointer_cast<IPrivateComponent>(target);
    if (ptr)
    {
        const auto type = ptr->GetTypeName();
        auto entity = ptr->GetOwner();
        auto scene = Application::GetActiveScene();
        if (scene->IsEntityValid(entity))
        {
            if (ImGui::BeginDragDropSource())
            {
                auto handle = scene->GetEntityHandle(entity);
                ImGui::SetDragDropPayload("PrivateComponent", &handle, sizeof(Handle));
                ImGui::TextColored(ImVec4(0, 0, 1, 1), type.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}
template <typename T> void Editor::DraggableAsset(const std::shared_ptr<T> &target)
{
    if (ImGui::BeginDragDropSource())
    {
        const auto ptr = std::dynamic_pointer_cast<IAsset>(target);
        if (ptr)
        {
            const auto title = ptr->GetTitle();
            ImGui::SetDragDropPayload("Asset", &ptr->m_handle, sizeof(Handle));
            ImGui::TextColored(ImVec4(0, 0, 1, 1), title.c_str());
        }
        ImGui::EndDragDropSource();
    }
}
template <typename T> void Editor::Draggable(AssetRef &target)
{
    DraggableAsset(target.Get<IAsset>());
}
template <typename T> bool Editor::Droppable(AssetRef &target)
{
    return UnsafeDroppableAsset(target, {Serialization::GetSerializableTypeName<T>()});
}

template <typename T> bool Editor::Rename(AssetRef &target)
{
    return RenameAsset(target.Get<IAsset>());
}
template <typename T> bool Editor::Remove(AssetRef &target)
{
    bool statusChanged = false;
    auto ptr = target.Get<IAsset>();
    if (ptr)
    {
        const std::string type = ptr->GetTypeName();
        const std::string tag = "##" + type + std::to_string(ptr->GetHandle());
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
    return statusChanged;
}
template <typename T> bool Editor::Remove(PrivateComponentRef &target)
{
    bool statusChanged = false;
    auto ptr = target.Get<IPrivateComponent>();
    if (ptr)
    {
        const std::string type = ptr->GetTypeName();
        const std::string tag = "##" + type + std::to_string(ptr->GetHandle());
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
    return statusChanged;
}

template <typename T> bool Editor::RenameAsset(const std::shared_ptr<T> &target)
{
    bool statusChanged = false;
    auto ptr = std::dynamic_pointer_cast<IAsset>(target);
    const std::string type = ptr->GetTypeName();
    const std::string tag = "##" + type + std::to_string(ptr->GetHandle());
    if (ImGui::BeginPopupContextItem(tag.c_str()))
    {
        if (!ptr->IsTemporary())
        {
            if (ImGui::BeginMenu(("Rename" + tag).c_str()))
            {
                static char newName[256];
                ImGui::InputText(("New name" + tag).c_str(), newName, 256);
                if (ImGui::Button(("Confirm" + tag).c_str()))
                {
                    bool succeed = ptr->SetPathAndSave(ptr->GetProjectRelativePath().replace_filename(
                        std::string(newName) + ptr->GetAssetRecord().lock()->GetAssetExtension()));
                    if (succeed)
                        memset(newName, 0, 256);
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndPopup();
    }
    return statusChanged;
}
template <typename T> void Editor::Draggable(PrivateComponentRef &target)
{
    DraggablePrivateComponent(target.Get<IPrivateComponent>());
}

template <typename T> bool Editor::Droppable(PrivateComponentRef &target)
{
    return UnsafeDroppablePrivateComponent(target, {Serialization::GetSerializableTypeName<T>()});
}

} // namespace UniEngine
