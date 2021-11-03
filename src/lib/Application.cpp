#include <AnimationManager.hpp>
#include <Application.hpp>
#include <AssetManager.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <EntityManager.hpp>
#include <Gui.hpp>
#include <InputManager.hpp>
#include <JobManager.hpp>
#include <OpenGLUtils.hpp>
#include <PhysicsManager.hpp>
#include <ProjectManager.hpp>
#include <RenderManager.hpp>
#include <TransformManager.hpp>
#include <WindowManager.hpp>
#include <EditorLayer.hpp>
#include <ProfilerLayer.hpp>
using namespace UniEngine;

void Application::Init(const ApplicationConfigs &applicationConfigs)
{
    auto &application = GetInstance();

    PushLayer<ProfilerLayer>();
    PushLayer<EditorLayer>();

    application.m_applicationConfigs = applicationConfigs;

    WindowManager::Init("UniEngine", applicationConfigs.m_fullScreen);
    InputManager::Init();
    JobManager::Init();
    OpenGLUtils::Init();
    PhysicsManager::Init();
    AssetManager::Init();
    EntityManager::Init();
    EditorManager::InitImGui();
    RenderManager::Init();

    TransformManager::Init();

    for (auto &i : application.m_layers)
    {
        i->OnCreate();
    }
    application.m_applicationStatus = ApplicationStatus::Uninitialized;
    if (!application.m_applicationConfigs.m_projectPath.empty())
    {
        ProjectManager::CreateOrLoadProject(application.m_applicationConfigs.m_projectPath);
        application.m_applicationStatus = ApplicationStatus::Initialized;
    }
    application.m_gameStatus = GameStatus::Stop;
}
void ApplicationTime::OnInspect()
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
    WindowManager::PreUpdate();
    application.m_time.m_deltaTime = glfwGetTime() - application.m_time.m_frameStartTime;
    application.m_time.m_frameStartTime = glfwGetTime();
    EditorManager::ImGuiPreUpdate();
    OpenGLUtils::PreUpdate();
    if (application.m_applicationStatus == ApplicationStatus::Initialized)
    {
        for (const auto &i : application.m_externalPreUpdateFunctions)
            i();

        if (application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step)
        {
            EntityManager::GetInstance().m_scene->Start();
        }
        for (auto &i : application.m_layers)
        {
            i->PreUpdate();
        }
        RenderManager::PreUpdate();
        InputManager::PreUpdate();
        PhysicsManager::PreUpdate();
        TransformManager::PreUpdate();
        AnimationManager::PreUpdate();
        auto fixedDeltaTime = application.m_time.FixedDeltaTime();
        if (fixedDeltaTime >= application.m_time.m_timeStep)
        {
            application.m_time.StartFixedUpdate();
            for (const auto &i : application.m_externalFixedUpdateFunctions)
                i();
            for (auto &i : application.m_layers)
            {
                i->FixedUpdate();
            }
            if (application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step)
            {
                EntityManager::GetInstance().m_scene->FixedUpdate();
            }
            application.m_time.EndFixedUpdate();
        }
    }

}

void Application::UpdateInternal()
{
    auto &application = GetInstance();
    if (application.m_applicationStatus == ApplicationStatus::Initialized)
    {
        for (const auto &i : application.m_externalUpdateFunctions)
            i();

        for (auto &i : application.m_layers)
        {
            i->Update();
        }
        if (application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step)
        {
            EntityManager::GetInstance().m_scene->Update();
        }
    }
}

void Application::LateUpdateInternal()
{
    auto &application = GetInstance();
    if (application.m_applicationStatus == ApplicationStatus::Initialized)
    {
        for (const auto &i : application.m_externalLateUpdateFunctions)
            i();

        if (application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step)
        {
            EntityManager::GetInstance().m_scene->LateUpdate();
        }

        for (auto &i : application.m_layers)
        {
            i->LateUpdate();
        }
        for (auto &i : application.m_layers)
        {
            i->OnInspect();
        }
        // Post-processing happens here
        RenderManager::LateUpdate();
        // Manager settings
        OnInspect();
        InputManager::OnInspect();
        AssetManager::OnInspect();
        RenderManager::OnInspect();
        ConsoleManager::OnInspect();
        ProjectManager::OnInspect();

        if (application.m_gameStatus == GameStatus::Step)
            application.m_gameStatus = GameStatus::Pause;
    }
    else
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Project"))
            {
                FileUtils::SaveFile(
                    "Create or load New Project",
                    "Project",
                    {".ueproj"},
                    [&](const std::filesystem::path &path) {
                        ProjectManager::CreateOrLoadProject(path);
                        application.m_applicationStatus = ApplicationStatus::Initialized;
                    },
                    false);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
    // ImGui drawing
    EditorManager::ImGuiLateUpdate();
    // Swap Window's framebuffer
    WindowManager::LateUpdate();
    application.m_time.m_lastUpdateTime = glfwGetTime();
}

ApplicationTime &Application::Time()
{
    return GetInstance().m_time;
}

bool Application::IsInitialized()
{
    return GetInstance().m_applicationStatus == ApplicationStatus::Initialized;
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

    while (application.m_applicationStatus != ApplicationStatus::OnDestroy)
    {
        PreUpdateInternal();
        UpdateInternal();
        LateUpdateInternal();
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
    application.m_gameStatus = GameStatus::Stop;
    application.m_time.Reset();
}
void Application::OnInspect()
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
        application.m_time.OnInspect();
    }
    ImGui::End();
}
void Application::Play()
{
    auto &application = GetInstance();
    if (application.m_gameStatus != GameStatus::Pause && application.m_gameStatus != GameStatus::Stop)
        return;
    if (application.m_gameStatus == GameStatus::Stop)
    {
        auto copiedScene = AssetManager::CreateAsset<Scene>();
        copiedScene->Clone(application.m_scene);
        EntityManager::Attach(copiedScene);
    }
    application.m_gameStatus = GameStatus::Playing;
}
void Application::Stop()
{
    auto &application = GetInstance();
    if (application.m_gameStatus == GameStatus::Stop)
        return;
    application.m_gameStatus = GameStatus::Stop;
    EntityManager::Attach(application.m_scene);
}
void Application::Pause()
{
    auto &application = GetInstance();
    if (application.m_gameStatus != GameStatus::Playing)
        return;
    application.m_gameStatus = GameStatus::Pause;
}
GameStatus Application::GetGameStatus()
{
    return GetInstance().m_gameStatus;
}
void Application::Step()
{
    auto &application = GetInstance();
    if (application.m_gameStatus != GameStatus::Pause && application.m_gameStatus != GameStatus::Stop)
        return;
    if (application.m_gameStatus == GameStatus::Stop)
    {
        auto copiedScene = AssetManager::CreateAsset<Scene>();
        copiedScene->Clone(application.m_scene);
        EntityManager::Attach(copiedScene);
    }
    application.m_gameStatus = GameStatus::Step;
}
bool Application::IsPlaying()
{
    auto &application = GetInstance();
    return application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step;
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