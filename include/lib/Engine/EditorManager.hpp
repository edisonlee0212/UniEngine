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
    EntityArchetype m_basicEntityArchetype;
    bool m_enabled = false;
    std::map<size_t, std::function<void(Entity entity, IDataComponent *data, bool isRoot)>>
        m_componentDataInspectorMap;
    //TODO: Remove this.
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_privateComponentMenuList;
    std::vector<std::pair<size_t, std::function<void(Entity owner)>>> m_componentDataMenuList;
    unsigned int m_configFlags = 0;

    Entity m_selectedEntity;
    bool m_enableConsole = true;
    glm::vec3 m_previouslyStoredPosition;
    glm::vec3 m_previouslyStoredRotation;
    glm::vec3 m_previouslyStoredScale;
    bool m_localPositionSelected = true;
    bool m_localRotationSelected = false;
    bool m_localScaleSelected = false;

    bool m_enableConsoleLogs = true;
    bool m_enableConsoleErrors = true;
    bool m_enableConsoleWarnings = false;
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

    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightSkinnedPrePassProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassInstancedProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightPrePassInstancedSkinnedProgram;

    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightSkinnedProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightInstancedProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneHighlightInstancedSkinnedProgram;

    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityRecorderProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntitySkinnedRecorderProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityInstancedRecorderProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_sceneCameraEntityInstancedSkinnedRecorderProgram;

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
  public:
    int m_selectedHierarchyDisplayMode = 1;
    static void MoveCamera(
        const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime = 1.0f);

    float m_sceneCameraYawAngle = -90;
    float m_sceneCameraPitchAngle = 0;
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    bool m_lockCamera;

    Camera m_sceneCamera;
    glm::quat m_sceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_sceneCameraPosition = glm::vec3(0, 5, 20);
    glm::quat m_defaultSceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_defaultSceneCameraPosition = glm::vec3(0, 5, 20);
    static void HighLightEntity(const Entity &entity, const glm::vec4 &color);
    static void LateUpdate();
    static void OnGui();
    template <typename T1 = IDataComponent>
    static void RegisterComponentDataInspector(
        const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func);
    template <typename T1 = IPrivateComponent>
    static void RegisterPrivateComponentMenu(const std::function<void(Entity owner)> &func);
    template <typename T1 = IDataComponent>
    static void RegisterComponentDataMenu(const std::function<void(Entity owner)> &func);
    static void Init();
    static void Destroy();
    static void PreUpdate();
    static void RenderToSceneCamera();
    static Entity GetSelectedEntity();
    static void SetSelectedEntity(const Entity &entity, const bool &openMenu = true);
    static Camera &GetSceneCamera();
    template <typename T = IAsset> static bool DragAndDrop(std::shared_ptr<T> &target);
    template <typename T = IAsset> static bool Draggable(std::shared_ptr<T> &target);
    static bool DragAndDrop(Entity &entity);
    static bool Draggable(const std::string &typeName, std::shared_ptr<IAsset> &target);



    static bool MainCameraWindowFocused();
    static bool SceneCameraWindowFocused();
};

template <typename T1>
void EditorManager::RegisterComponentDataInspector(
    const std::function<void(Entity entity, IDataComponent *data, bool isRoot)> &func)
{
    GetInstance().m_componentDataInspectorMap.insert_or_assign(typeid(T1).hash_code(), func);
}

template <typename T1> void EditorManager::RegisterPrivateComponentMenu(const std::function<void(Entity owner)> &func)
{
    for (int i = 0; i < GetInstance().m_privateComponentMenuList.size(); i++)
    {
        if (GetInstance().m_privateComponentMenuList[i].first == typeid(T1).hash_code())
        {
            GetInstance().m_privateComponentMenuList[i].second = func;
            return;
        }
    }
    GetInstance().m_privateComponentMenuList.emplace_back(typeid(T1).hash_code(), func);
}

template <typename T1> void EditorManager::RegisterComponentDataMenu(const std::function<void(Entity owner)> &func)
{
    for (int i = 0; i < GetInstance().m_componentDataMenuList.size(); i++)
    {
        if (GetInstance().m_componentDataMenuList[i].first == typeid(T1).hash_code())
        {
            GetInstance().m_componentDataMenuList[i].second = func;
            return;
        }
    }
    GetInstance().m_componentDataMenuList.emplace_back(typeid(T1).hash_code(), func);
}

template <typename T> bool EditorManager::DragAndDrop(std::shared_ptr<T> &target)
{
    const std::shared_ptr<IAsset> ptr = std::dynamic_pointer_cast<IAsset>(target);
    assert(!(ptr && ptr->GetHandle() == 0));
    const std::string type = SerializableFactory::GetSerializableTypeName<T>();
    bool statusChanged = false;
    ImGui::Button(ptr ? ptr->m_name.c_str() : "none");
    if (ptr)
    {
        const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(type.c_str(), &target, sizeof(std::shared_ptr<T>));
            if (ptr->m_icon)
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(ptr->m_icon->Texture()->Id()),
                    ImVec2(30, 30),
                    ImVec2(0, 1),
                    ImVec2(1, 0));
            else
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
            if (ImGui::Button(("Remove" + tag).c_str()))
            {
                target.reset();
                statusChanged = true;
            }
            ImGui::EndPopup();
        }
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type.c_str()))
        {
            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<T>));
            std::shared_ptr<T> payload_n = *static_cast<std::shared_ptr<T> *>(payload->Data);
            target = payload_n;
            statusChanged = true;
        }
        ImGui::EndDragDropTarget();
    }
    return statusChanged;
}

template <typename T> bool EditorManager::Draggable(std::shared_ptr<T> &target)
{
    const std::shared_ptr<IAsset> ptr = std::dynamic_pointer_cast<IAsset>(target);
    assert(!(ptr && ptr->GetHandle() == 0));
    const std::string type = SerializableFactory::GetSerializableTypeName<T>();
    bool removed = false;
    ImGui::Button(ptr ? ptr->m_name.c_str() : "none");
    if (ptr)
    {
        const std::string tag = "##" + type + (ptr ? std::to_string(ptr->GetHandle()) : "");
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload(type.c_str(), &target, sizeof(std::shared_ptr<T>));
            if (ptr->m_icon)
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(ptr->m_icon->Texture()->Id()),
                    ImVec2(30, 30),
                    ImVec2(0, 1),
                    ImVec2(1, 0));
            else
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
            if (ImGui::Button(("Remove" + tag).c_str()))
            {
                target.reset();
                removed = true;
            }
            ImGui::EndPopup();
        }
    }
    return removed;
}


} // namespace UniEngine
