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
enum EntityEditorSystemConfigFlags
{
    EntityEditorSystem_None = 0,
    EntityEditorSystem_EnableEntityHierarchy = 1 << 0,
    EntityEditorSystem_EnableEntityInspector = 1 << 1
};
class UNIENGINE_API EditorManager : public ISingleton<EditorManager>
{
    friend class ClassRegistry;
    friend class DefaultResources;
    friend class ProjectManager;
    std::map<std::string, std::shared_ptr<Texture2D>> m_assetsIcons;
    EntityArchetype m_basicEntityArchetype;
    bool m_enabled = false;
    std::map<size_t, std::function<void(Entity entity, IDataComponent *data, bool isRoot)>> m_componentDataInspectorMap;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_privateComponentMenuList;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_componentDataMenuList;
    unsigned int m_configFlags = 0;

    Entity m_selectedEntity;

    glm::vec3 m_previouslyStoredPosition;
    glm::vec3 m_previouslyStoredRotation;
    glm::vec3 m_previouslyStoredScale;
    bool m_localPositionSelected = true;
    bool m_localRotationSelected = false;
    bool m_localScaleSelected = false;

    bool m_sceneCameraWindowFocused = false;
    bool m_mainCameraWindowFocused = false;
#pragma region Transfer

    glm::quat m_previousRotation;
    glm::vec3 m_previousPosition;
    glm::quat m_targetRotation;
    glm::vec3 m_targetPosition;
    float m_transitionTime;
    float m_transitionTimer;
#pragma endregion

#pragma region Scene Camera
    friend class RenderManager;
    friend class InputManager;

    std::unique_ptr<RenderTarget> m_sceneCameraEntityRecorder;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_sceneCameraEntityRecorderTexture;
    std::unique_ptr<OpenGLUtils::GLRenderBuffer> m_sceneCameraEntityRecorderRenderBuffer;

    std::vector<Entity> m_selectedEntityHierarchyList;

    int m_sceneCameraResolutionX = 1;
    int m_sceneCameraResolutionY = 1;
    float m_lastX = 0;
    float m_lastY = 0;
    float m_lastScrollY = 0;
    bool m_startMouse = false;
    bool m_startScroll = false;
    bool m_leftMouseButtonHold = false;
    bool m_rightMouseButtonHold = false;
#pragma endregion
    static bool DrawEntityMenu(const bool &enabled, const Entity &entity);
    static void DrawEntityNode(const Entity &entity, const unsigned &hierarchyLevel);
    static void InspectComponentData(Entity entity, IDataComponent *data, DataComponentType type, bool isRoot);
    static Entity MouseEntitySelection(const glm::vec2 &mousePosition);
    static void HighLightEntityPrePassHelper(const Entity &entity);
    static void HighLightEntityHelper(const Entity &entity);

    static void SceneCameraWindow();
    static void MainCameraWindow();

    static void CameraWindowDragAndDrop();
    template <typename T1 = IPrivateComponent> static void RegisterPrivateComponent();
    template <typename T1 = IDataComponent> static void RegisterDataComponent();

  public:
    int m_selectedHierarchyDisplayMode = 1;
    static void MoveCamera(
        const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime = 1.0f);

    float m_sceneCameraYawAngle = -90;
    float m_sceneCameraPitchAngle = 0;
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    bool m_lockCamera;
    std::weak_ptr<IAsset> m_inspectingAsset;
    std::shared_ptr<Camera> m_sceneCamera;
    glm::quat m_sceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_sceneCameraPosition = glm::vec3(0, 5, 20);
    glm::quat m_defaultSceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_defaultSceneCameraPosition = glm::vec3(0, 5, 20);
    static void HighLightEntity(const Entity &entity, const glm::vec4 &color);
    static void LateUpdate();
    static void OnInspect();
    template <typename T1 = IDataComponent>
    static void RegisterComponentDataInspector(
        const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func);

    static void Init();
    static void Destroy();
    static void PreUpdate();
    static void RenderToSceneCamera();
    static Entity GetSelectedEntity();
    static void SetSelectedEntity(const Entity &entity, bool openMenu = true);
    static std::weak_ptr<Camera> GetSceneCamera();

    static bool DragAndDropButton(
        AssetRef &target,
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
    static bool MainCameraWindowFocused();
    static bool SceneCameraWindowFocused();
};

template <typename T1>
void EditorManager::RegisterComponentDataInspector(
    const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func)
{
    GetInstance().m_componentDataInspectorMap.insert_or_assign(typeid(T1).hash_code(), func);
}

template <typename T> void EditorManager::RegisterPrivateComponent()
{
    auto &editorManager = GetInstance();
    auto func = [&](Entity owner) {
        if (owner.HasPrivateComponent<T>())
            return;
        if (ImGui::SmallButton(SerializationManager::GetSerializableTypeName<T>().c_str()))
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
        if (ImGui::SmallButton(SerializationManager::GetDataComponentTypeName<T>().c_str()))
        {
            EntityManager::AddDataComponent<T>(owner, T());
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
        const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(type.c_str(), &target.m_assetHandle, sizeof(Handle));
            /*if (ptr->m_icon)
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(ptr->m_icon->UnsafeGetGLTexture()->Id()),
                    ImVec2(30, 30),
                    ImVec2(0, 1),
                    ImVec2(1, 0));
            else
             */
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

            if (ImGui::BeginMenu("I/O"))
            {
                FileUtils::SaveFile(
                    ("Save " + type + tag).c_str(),
                    type,
                    {AssetManager::GetExtension<T>()},
                    [&](const std::filesystem::path &filePath) {
                        try
                        {
                            ptr->SetPathAndSave(ProjectManager::GetRelativePath(filePath));
                            UNIENGINE_LOG("Saved to " + filePath.string());
                        }
                        catch (std::exception &e)
                        {
                            UNIENGINE_ERROR("Failed to save to " + filePath.string());
                        }
                    });

                FileUtils::OpenFile(
                    ("Load " + type + tag).c_str(),
                    type,
                    {AssetManager::GetExtension<T>()},
                    [&](const std::filesystem::path &filePath) {
                        try
                        {
                            ptr->SetPathAndLoad(ProjectManager::GetRelativePath(filePath));
                            UNIENGINE_LOG("Loaded from " + filePath.string());
                        }
                        catch (std::exception &e)
                        {
                            UNIENGINE_ERROR("Failed to load from " + filePath.string());
                        }
                    });

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
        const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
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
        /*if (ptr->m_icon)
            ImGui::Image(
                reinterpret_cast<ImTextureID>(ptr->m_icon->UnsafeGetGLTexture()->Id()),
                ImVec2(30, 30),
                ImVec2(0, 1),
                ImVec2(1, 0));
        else
         */
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

        if (ImGui::BeginMenu("I/O"))
        {
            auto &extensions = AssetManager::GetInstance().m_defaultExtensions[type];
            FileUtils::SaveFile(
                ("Save " + type + tag).c_str(), type, extensions, [&](const std::filesystem::path &filePath) {
                    try
                    {
                        ptr->SetPathAndSave(ProjectManager::GetRelativePath(filePath));
                        UNIENGINE_LOG("Saved to " + filePath.string());
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to save to " + filePath.string());
                    }
                });

            FileUtils::OpenFile(
                ("Load " + type + tag).c_str(), type, extensions, [&](const std::filesystem::path &filePath) {
                    try
                    {
                        ptr->SetPathAndLoad(ProjectManager::GetRelativePath(filePath));
                        UNIENGINE_LOG("Loaded from " + filePath.string());
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to load from " + filePath.string());
                    }
                });

            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    return;
}

} // namespace UniEngine
