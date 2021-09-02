#include <AnimationManager.hpp>
#include <Application.hpp>
#include <AssetManager.hpp>
#include <OpenGLUtils.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <EntityManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <JobManager.hpp>
#include <PhysicsManager.hpp>
#include <ProjectManager.hpp>
#include <RenderManager.hpp>
#include <TransformManager.hpp>
#include <WindowManager.hpp>
using namespace UniEngine;

void Application::Init(bool fullScreen)
{
    auto &application = GetInstance();
    application.m_initialized = false;
    WindowManager::Init("UniEngine", fullScreen);
    InputManager::Init();
    JobManager::Init();
    OpenGLUtils::Init();
    PhysicsManager::Init();

    AssetManager::Init();

    EntityManager::Init();
    ProjectManager::Init();
    TransformManager::Init();
    RenderManager::Init();
    EditorManager::Init();

    application.m_initialized = true;
    application.m_playing = false;
    ProfilerManager::GetOrCreateProfiler<CPUTimeProfiler>("CPU Time");

    EntityManager::GetSystem<PhysicsSystem>()->Enable();
}
void ApplicationTime::OnGui()
{
    if (ImGui::CollapsingHeader("Time Settings"))
    {
        float timeStep = m_timeStep;
        if (ImGui::DragFloat("Time step", &timeStep, 0.001f, 0.001f, 1.0f))
        {
            m_timeStep = timeStep;
        }
    }
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
    application.m_initialized = !glfwWindowShouldClose(WindowManager::GetWindow());

    WindowManager::PreUpdate();

    EditorManager::PreUpdate();
    ProfilerManager::PreUpdate();
    ProfilerManager::StartEvent("PreUpdate");
    ProfilerManager::StartEvent("Internals");

    RenderManager::PreUpdate();
    InputManager::PreUpdate();
    OpenGLUtils::PreUpdate();

    ProfilerManager::EndEvent("Internals");
    ProfilerManager::StartEvent("Externals");
    for (const auto &i : application.m_externalPreUpdateFunctions)
        i();
    ProfilerManager::EndEvent("Externals");
    if (application.m_playing)
    {
        ProfilerManager::StartEvent("Scene");
        EntityManager::GetInstance().m_scene->PreUpdate();
        ProfilerManager::EndEvent("Scene");
    }
    AnimationManager::PreUpdate();
    application.m_needFixedUpdate = false;
    auto fixedDeltaTime = application.m_time.FixedDeltaTime();
    if (fixedDeltaTime >= application.m_time.m_timeStep)
    {
        application.m_needFixedUpdate = true;
    }
    PhysicsManager::PreUpdate();
    if (application.m_needFixedUpdate)
    {
        ProfilerManager::StartEvent("FixedUpdate");
        application.m_time.StartFixedUpdate();
        ProfilerManager::StartEvent("Externals");
        for (const auto &i : application.m_externalFixedUpdateFunctions)
            i();
        ProfilerManager::EndEvent("Externals");
        if (application.m_playing)
        {
            ProfilerManager::StartEvent("Scene");
            EntityManager::GetInstance().m_scene->FixedUpdate();
            ProfilerManager::EndEvent("Scene");
        }
        application.m_time.EndFixedUpdate();
        ProfilerManager::EndEvent("FixedUpdate");
    }
    TransformManager::PreUpdate();
    ProfilerManager::EndEvent("PreUpdate");
}

void Application::UpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return;
    ProfilerManager::StartEvent("Set");
    ProfilerManager::StartEvent("Externals");
    for (const auto &i : application.m_externalUpdateFunctions)
        i();
    ProfilerManager::EndEvent("Externals");
    if (application.m_playing)
    {
        ProfilerManager::StartEvent("Scene");
        EntityManager::GetInstance().m_scene->Update();
        ProfilerManager::EndEvent("Scene");
    }
    ProfilerManager::EndEvent("Set");
}

bool Application::LateUpdateInternal()
{
    auto &application = GetInstance();
    if (!application.m_initialized)
        return false;
    ProfilerManager::StartEvent("LateUpdate");
    ProfilerManager::StartEvent("Externals");
    for (const auto &i : application.m_externalLateUpdateFunctions)
        i();
    ProfilerManager::EndEvent("Externals");
    if (application.m_playing)
    {
        ProfilerManager::StartEvent("Scene");
        EntityManager::GetInstance().m_scene->LateUpdate();
        ProfilerManager::EndEvent("Scene");
    }
    ProfilerManager::StartEvent("Internals");
    EntityManager::GetInstance().m_scene->OnGui();

    // Post-processing happens here
    RenderManager::LateUpdate();
    // Manager settings
    OnGui();
    InputManager::OnGui();
    AssetManager::OnGui();
    RenderManager::OnGui();
    EditorManager::OnGui();
    ConsoleManager::OnGui();
    ProjectManager::OnGui();
    ProfilerManager::EndEvent("Internals");
    // Profile
    ProfilerManager::EndEvent("LateUpdate");
    ProfilerManager::LateUpdate();
    ProfilerManager::OnGui();

    // ImGui drawing
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
    EntityManager::GetInstance().m_scene.reset();
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
void Application::Reset()
{
    auto &application = GetInstance();
    application.m_externalPreUpdateFunctions.clear();
    application.m_externalUpdateFunctions.clear();
    application.m_externalFixedUpdateFunctions.clear();
    application.m_externalLateUpdateFunctions.clear();
    application.m_playing = false;
    application.m_needFixedUpdate = false;
    application.m_time.Reset();
}
void Application::OnGui()
{
    auto &application = GetInstance();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("App Settings", &application.m_enableSettingsMenu);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (!application.m_enableSettingsMenu)
        return;
    if (ImGui::Begin("Application Settings"))
    {
        application.m_time.OnGui();
    }
    ImGui::End();
}
void ApplicationTime::Reset()
{
    m_lastFixedUpdateTime = 0;
    m_lastUpdateTime = 0;
    m_timeStep = 0.016;
    m_frameStartTime = 0;
    m_deltaTime = 0;
    m_fixedUpdateTimeStamp = 0;
}