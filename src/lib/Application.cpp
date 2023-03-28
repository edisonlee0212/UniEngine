#include "Application.hpp"
#include "AnimationLayer.hpp"
#include "ConsoleLayer.hpp"
#include "DefaultResources.hpp"
#include "Editor.hpp"
#include "EditorLayer.hpp"
#include "Entities.hpp"
#include "Graphics.hpp"
#include "Inputs.hpp"
#include "Jobs.hpp"
#include "OpenGLUtils.hpp"
#include "PhysicsLayer.hpp"
#include "ProfilerLayer.hpp"
#include "ProjectManager.hpp"
#include "RenderLayer.hpp"
#include "Scene.hpp"
#include "TransformLayer.hpp"
#include "Windows.hpp"
using namespace UniEngine;

void Application::Create(const ApplicationConfigs &applicationConfigs)
{
    auto &application = GetInstance();
    Windows::Init(applicationConfigs.m_applicationName, applicationConfigs.m_fullScreen);
    OpenGLUtils::Init();
    application.m_applicationConfigs = applicationConfigs;

    Inputs::Init();
    Jobs::Init();
    DefaultResources::Load();
    Entities::Init();
    Editor::Init(applicationConfigs.m_enableDocking, applicationConfigs.m_enableViewport);

    PushLayer<ProfilerLayer>();
    PushLayer<TransformLayer>();
    PushLayer<AnimationLayer>();
    PushLayer<EditorLayer>();
    PushLayer<RenderLayer>();
    PushLayer<PhysicsLayer>();
    PushLayer<ConsoleLayer>();
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
    Windows::PreUpdate();
    application.m_time.m_deltaTime = glfwGetTime() - application.m_time.m_frameStartTime;
    application.m_time.m_frameStartTime = glfwGetTime();
    Editor::ImGuiPreUpdate();
    OpenGLUtils::PreUpdate();
    if (application.m_applicationStatus == ApplicationStatus::Initialized)
    {
        Inputs::PreUpdate();
        for (const auto &i : application.m_externalPreUpdateFunctions)
            i();

        if (application.m_gameStatus == GameStatus::Playing || application.m_gameStatus == GameStatus::Step)
        {
            application.m_activeScene->Start();
        }
        for (auto &i : application.m_layers)
        {
            i->PreUpdate();
        }
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
                application.m_activeScene->FixedUpdate();
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
            application.m_activeScene->Update();
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
            application.m_activeScene->LateUpdate();
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
        // Manager settings
        OnInspect();
        if (application.m_gameStatus == GameStatus::Step)
            application.m_gameStatus = GameStatus::Pause;
    }
    else
    {
        if (ImGui::BeginMainMenuBar())
        {
            FileUtils::SaveFile(
                "Create or load New Project",
                "Project",
                {".ueproj"},
                [&](const std::filesystem::path &path) {
                    ProjectManager::GetOrCreateProject(path);
                    if (ProjectManager::GetInstance().m_projectFolder)
                    {
                        Windows::ResizeWindow(
                            application.m_applicationConfigs.m_defaultWindowSize.x,
                            application.m_applicationConfigs.m_defaultWindowSize.y);
                        application.m_applicationStatus = ApplicationStatus::Initialized;
                    }
                },
                false);
            ImGui::EndMainMenuBar();
        }
    }
    // ImGui drawing
    Editor::ImGuiLateUpdate();
    // Swap Window's framebuffer
    Windows::LateUpdate();
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
    auto &application = GetInstance();
	application.m_activeScene.reset();
    ProjectManager::OnDestroy();
    DefaultResources::OnDestroy();
    for (auto &i : application.m_layers)
    {
        i->OnDestroy();
    }
    glfwTerminate();
}

void Application::Start()
{
    auto &application = GetInstance();

    for (auto &i : application.m_layers)
    {
        i->OnCreate();
    }
    application.m_applicationStatus = ApplicationStatus::Uninitialized;
    if (!application.m_applicationConfigs.m_projectPath.empty())
    {
        ProjectManager::GetOrCreateProject(application.m_applicationConfigs.m_projectPath);
        if (ProjectManager::GetInstance().m_projectFolder)
        {
            Windows::ResizeWindow(
                application.m_applicationConfigs.m_defaultWindowSize.x,
                application.m_applicationConfigs.m_defaultWindowSize.y);
            application.m_applicationStatus = ApplicationStatus::Initialized;
        }
    }
    application.m_gameStatus = GameStatus::Stop;
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

void Application::RegisterPostAttachSceneFunction(
    const std::function<void(const std::shared_ptr<Scene> &newScene)> &func)
{
    GetInstance().m_postAttachSceneFunctions.push_back(func);
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
            ImGui::Checkbox("Application Settings", &application.m_enableSettingsMenu);
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

std::shared_ptr<Scene> Application::GetActiveScene()
{
    auto &application = GetInstance();
    return application.m_activeScene;
}

void Application::Attach(const std::shared_ptr<Scene> &scene)
{
    if (Application::IsPlaying())
    {
        UNIENGINE_ERROR("Stop Application to attach scene");
    }
    auto &application = GetInstance();
    application.m_activeScene = scene;
    for (auto &func : application.m_postAttachSceneFunctions)
    {
        func(scene);
    }
    for (auto &layer : application.m_layers)
    {
        layer->m_scene = scene;
    }
}

void Application::Play()
{
    auto &application = GetInstance();
    auto &projectManager = ProjectManager::GetInstance();
    if (application.m_gameStatus != GameStatus::Pause && application.m_gameStatus != GameStatus::Stop)
        return;
    if (application.m_gameStatus == GameStatus::Stop)
    {
        auto copiedScene = ProjectManager::CreateTemporaryAsset<Scene>();
        Scene::Clone(projectManager.GetStartScene().lock(), copiedScene);
        Attach(copiedScene);
    }
    application.m_gameStatus = GameStatus::Playing;
}
void Application::Stop()
{
    auto &application = GetInstance();
    auto &projectManager = ProjectManager::GetInstance();
    if (application.m_gameStatus == GameStatus::Stop)
        return;
    application.m_gameStatus = GameStatus::Stop;
    Attach(projectManager.GetStartScene().lock());
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
    auto &projectManager = ProjectManager::GetInstance();
    auto &application = GetInstance();
    if (application.m_gameStatus != GameStatus::Pause && application.m_gameStatus != GameStatus::Stop)
        return;
    if (application.m_gameStatus == GameStatus::Stop)
    {
        auto copiedScene = ProjectManager::CreateTemporaryAsset<Scene>();
        Scene::Clone(projectManager.GetStartScene().lock(), copiedScene);
        Attach(copiedScene);
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