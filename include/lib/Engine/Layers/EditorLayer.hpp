#pragma once
#include <AssetManager.hpp>
#include <Camera.hpp>
#include <OpenGLUtils.hpp>
#include <ISingleton.hpp>
#include <RenderTarget.hpp>
#include <RigidBody.hpp>
#include <Texture2D.hpp>
#include <ILayer.hpp>
namespace UniEngine
{
struct Transform;
class Folder;
enum EntityEditorSystemConfigFlags
{
    EntityEditorSystem_None = 0,
    EntityEditorSystem_EnableEntityHierarchy = 1 << 0,
    EntityEditorSystem_EnableEntityInspector = 1 << 1
};
class UNIENGINE_API EditorLayer : public ILayer
{
  private:
    bool m_enableSystemInspector = false;

    unsigned int m_configFlags = 0;
    EntityArchetype m_basicEntityArchetype;


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
    friend class Graphics;
    friend class Inputs;

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
    bool DrawEntityMenu(const bool &enabled, const Entity &entity);
    void DrawEntityNode(const Entity &entity, const unsigned &hierarchyLevel);
    void InspectComponentData(Entity entity, IDataComponent *data, DataComponentType type, bool isRoot);
    Entity MouseEntitySelection(const glm::vec2 &mousePosition);
    void HighLightEntityPrePassHelper(const Entity &entity);
    void HighLightEntityHelper(const Entity &entity);
    void FolderHierarchyHelper(const std::shared_ptr<Folder>& folder);
    void SceneCameraWindow();
    void MainCameraWindow();



    friend class Application;

    void OnCreate() override;
    void PreUpdate() override;
    void LateUpdate() override;
    void OnInspect() override;

  public:
    void CameraWindowDragAndDrop();
    Entity m_selectedEntity;
    void MoveCamera(
        const glm::quat &targetRotation, const glm::vec3 &targetPosition, const float &transitionTime = 1.0f);
    void HighLightEntity(const Entity &entity, const glm::vec4 &color);
    int m_selectedHierarchyDisplayMode = 1;
    float m_sceneCameraYawAngle = -90;
    float m_sceneCameraPitchAngle = 0;
    float m_velocity = 20.0f;
    float m_sensitivity = 0.1f;
    bool m_lockCamera;

    std::shared_ptr<Camera> m_sceneCamera;
    glm::quat m_sceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_sceneCameraPosition = glm::vec3(0, 5, 20);
    glm::quat m_defaultSceneCameraRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
    glm::vec3 m_defaultSceneCameraPosition = glm::vec3(0, 5, 20);
    void RenderToSceneCamera();
    void SetSelectedEntity(const Entity &entity, bool openMenu = true);
    bool MainCameraWindowFocused();
    bool SceneCameraWindowFocused();

};
}