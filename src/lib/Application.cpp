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
#pragma region Internal Systems
    ResourceManager::Init();
    TransformManager::Init();
    RenderManager::Init();
    EditorManager::Init();
#pragma endregion
    application.m_initialized = true;
#pragma region Main Camera
    EntityArchetype archetype =
        EntityManager::CreateEntityArchetype("Camera", GlobalTransform(), Transform(), CameraLayerMask());
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
    ProfilerManager::GetEngineProfiler().StartEvent("PreUpdate");
    ProfilerManager::GetEngineProfiler().StartEvent("Internal PreUpdate");
    glfwPollEvents();
    application.m_initialized = !glfwWindowShouldClose(WindowManager::GetWindow());
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
    EditorManager::PreUpdate();

    OpenGLUtils::PreUpdate();

    WindowManager::PreUpdate();

    ProfilerManager::GetEngineProfiler().StartEvent("AnimationManager PreUpdate");
    AnimationManager::PreUpdate();
    ProfilerManager::GetEngineProfiler().EndEvent("AnimationManager PreUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("RenderManager PreUpdate");
    RenderManager::PreUpdate();
    ProfilerManager::GetEngineProfiler().EndEvent("RenderManager PreUpdate");
    ProfilerManager::GetEngineProfiler().EndEvent("Internal PreUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("External PreUpdate");
    for (const auto &i : application.m_externalPreUpdateFunctions)
        i();
    ProfilerManager::GetEngineProfiler().EndEvent("External PreUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("Systems PreUpdate");
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->PreUpdate();
    }
    ProfilerManager::GetEngineProfiler().EndEvent("Systems PreUpdate");

    ProfilerManager::GetEngineProfiler().EndEvent("PreUpdate");
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
    ProfilerManager::GetEngineProfiler().StartEvent("Update");

    ProfilerManager::GetEngineProfiler().StartEvent("Internal Update");
    EditorManager::Update();
    ProfilerManager::GetEngineProfiler().EndEvent("Internal Update");

    ProfilerManager::GetEngineProfiler().StartEvent("External Update");
    for (const auto &i : application.m_externalUpdateFunctions)
        i();
    ProfilerManager::GetEngineProfiler().EndEvent("External Update");

    ProfilerManager::GetEngineProfiler().StartEvent("Systems Update");
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->Update();
    }
    ProfilerManager::GetEngineProfiler().EndEvent("Systems Update");

    ProfilerManager::GetEngineProfiler().EndEvent("Update");
}

bool Application::LateUpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return false;
    ProfilerManager::GetEngineProfiler().StartEvent("LateUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("Internal LateUpdate");
    InputManager::LateUpdate();

    WindowManager::LateUpdate();
    ResourceManager::OnGui();

    EditorManager::LateUpdate();

    RenderManager::LateUpdate();
    RenderManager::OnGui();
    ProfilerManager::GetEngineProfiler().EndEvent("Internal LateUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("External LateUpdate");
    for (const auto &i : application.m_externalLateUpdateFunctions)
        i();
    ProfilerManager::GetEngineProfiler().EndEvent("External LateUpdate");

    ProfilerManager::GetEngineProfiler().StartEvent("Systems LateUpdate");
    if (application.m_playing)
    {
        EntityManager::GetInstance().m_world->LateUpdate();
    }

    ProfilerManager::GetEngineProfiler().EndEvent("Systems LateUpdate");

    ProfilerManager::GetEngineProfiler().EndEvent("LateUpdate");
    ProfilerManager::LateUpdate();
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
    // Swap Window's framebuffer
    WindowManager::Swap();
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

