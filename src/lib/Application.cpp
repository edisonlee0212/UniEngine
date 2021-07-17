#include <AnimationManager.hpp>
#include <Application.hpp>
#include <Core/OpenGLUtils.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <EntityManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <JobManager.hpp>
#include <PhysicsManager.hpp>
#include <RenderManager.hpp>
#include <ResourceManager.hpp>
#include <TransformManager.hpp>
#include <WindowManager.hpp>
using namespace UniEngine;

#pragma region Utilities

void Application::SetTimeStep(float value)
{
    auto &application = GetInstance();
    application.m_time.m_timeStep = value;
}

#pragma endregion

void Application::Init(bool fullScreen)
{
    FileIO::SetResourcePath(UNIENGINE_RESOURCE_FOLDER);
    auto &application = GetInstance();
    application.m_initialized = false;
    WindowManager::Init("UniEngine", fullScreen);
    InputManager::Init();
    JobManager::Init();
    PhysicsManager::Init();
    EntityManager::Init();
    OpenGLUtils::Init();
    ResourceManager::Init();
    TransformManager::Init();
    RenderManager::Init();
    EditorManager::Init();

    application.m_initialized = true;
#pragma region Main Camera
    EntityArchetype archetype =
        EntityManager::CreateEntityArchetype("Camera", GlobalTransform(), Transform());
    const auto mainCameraEntity = EntityManager::CreateEntity(archetype, "Main Camera");
    Transform cameraLtw;
    cameraLtw.SetPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    cameraLtw.SetEulerRotation(glm::radians(glm::vec3(0, 0, 15)));
    mainCameraEntity.SetDataComponent(cameraLtw);
    auto &mainCameraComponent = mainCameraEntity.SetPrivateComponent<CameraComponent>();
    RenderManager::SetMainCamera(&mainCameraComponent);
    mainCameraComponent.m_skybox = DefaultResources::Environmental::DefaultSkybox;
#pragma endregion
}

double ApplicationTime::TimeStep() const
{
    return m_timeStep;
}
void ApplicationTime::SetTimeStep(const double &value)
{
    m_timeStep = value;
}
double ApplicationTime::FixedDeltaTime() const
{
    return glfwGetTime() - m_lastFixedUpdateTime;
}

double ApplicationTime::DeltaTime() const
{
    return m_deltaTime;
}
double ApplicationTime::CurrentTime() const
{
    return glfwGetTime();
}

double ApplicationTime::LastFrameTime() const
{
    return m_lastUpdateTime;
}
void ApplicationTime::StartFixedUpdate()
{
    m_fixedUpdateTimeStamp = glfwGetTime();
}

void ApplicationTime::EndFixedUpdate()
{
    m_lastFixedUpdateTime = m_fixedUpdateTimeStamp;
}
void Application::PreUpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return;
    application.m_time.m_deltaTime = glfwGetTime() - application.m_time.m_frameStartTime;
    application.m_time.m_frameStartTime = glfwGetTime();
    ProfilerManager::PreUpdate();
    WindowManager::PreUpdate();

    application.m_initialized = !glfwWindowShouldClose(WindowManager::GetWindow());
    EditorManager::PreUpdate();
    RenderManager::PreUpdate();
    InputManager::PreUpdate();
    OpenGLUtils::PreUpdate();
    AnimationManager::PreUpdate();
    for (const auto &i : application.m_externalPreUpdateFunctions)
        i();
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->PreUpdate();
    }
    application.m_needFixedUpdate = false;
    auto fixedDeltaTime = application.m_time.FixedDeltaTime();
    if (fixedDeltaTime >= application.m_time.m_timeStep)
    {
        application.m_needFixedUpdate = true;
    }
    PhysicsManager::PreUpdate();
    if (application.m_needFixedUpdate)
    {
        application.m_time.StartFixedUpdate();
        for (const auto &i : application.m_externalFixedUpdateFunctions)
            i();
        if (application.m_playing)
            EntityManager::GetInstance().m_world->FixedUpdate();
        application.m_time.EndFixedUpdate();
    }
    TransformManager::PreUpdate();
}

void Application::UpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return;

    for (const auto &i : application.m_externalUpdateFunctions)
        i();
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->Update();
    }
}

bool Application::LateUpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return false;
    for (const auto &i : application.m_externalLateUpdateFunctions)
        i();
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->LateUpdate();
    }

    EntityManager::GetInstance().m_world->OnGui();

    //Post-processing happens here
    RenderManager::LateUpdate();
    //Manager settings
    InputManager::OnGui();
    ResourceManager::OnGui();
    RenderManager::OnGui();
    EditorManager::OnGui();

    //Profile
    ProfilerManager::LateUpdate();
    ProfilerManager::OnGui();

    //ImGui drawing
    EditorManager::LateUpdate();
    // Swap Window's framebuffer
    WindowManager::LateUpdate();
    application.m_time.m_lastUpdateTime = glfwGetTime();
    return application.m_initialized;
}

ApplicationTime &Application::Time()
{
    return GetInstance().m_time;
}

void Application::SetPlaying(bool value)
{
    GetInstance().m_playing = value;
}

bool Application::IsPlaying()
{
    return GetInstance().m_playing;
}

bool Application::IsInitialized()
{
    return GetInstance().m_initialized;
}

void Application::End()
{
    EntityManager::GetInstance().m_world.reset();
    PhysicsManager::Destroy();
    // glfwTerminate();
}

void Application::Run()
{
    auto &application = GetInstance();
    while (application.m_initialized)
    {
        PreUpdateInternal();
        UpdateInternal();
        application.m_initialized = LateUpdateInternal();
    }
}

void Application::RegisterPreUpdateFunction(const std::function<void()> &func)
{
    GetInstance().m_externalPreUpdateFunctions.push_back(func);
}

void Application::RegisterUpdateFunction(const std::function<void()> &func)
{
    GetInstance().m_externalUpdateFunctions.push_back(func);
}

void Application::RegisterLateUpdateFunction(const std::function<void()> &func)
{
    GetInstance().m_externalLateUpdateFunctions.push_back(func);
}
void Application::RegisterFixedUpdateFunction(const std::function<void()> &func)
{
    GetInstance().m_externalFixedUpdateFunctions.push_back(func);
}
