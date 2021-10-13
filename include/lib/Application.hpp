#pragma once
#include <Utilities.hpp>
#include <EntityManager.hpp>
#include <ISingleton.hpp>
#include <InputManager.hpp>
#include <ProfilerManager.hpp>
#include <RenderManager.hpp>
#include <Scene.hpp>
#include <ClassRegistry.hpp>
namespace UniEngine
{
class UNIENGINE_API ApplicationTime
{
    friend class Scene;
    friend class Application;
    double m_lastFixedUpdateTime = 0;
    double m_lastUpdateTime = 0;
    double m_timeStep = 0.016;
    double m_frameStartTime = 0;
    double m_deltaTime = 0;
    double m_fixedUpdateTimeStamp = 0;
    void StartFixedUpdate();
    void EndFixedUpdate();

  public:
    void Reset();
    void OnInspect();
    void SetTimeStep(const double &value);
    [[nodiscard]] double CurrentTime() const;
    [[nodiscard]] double TimeStep() const;
    [[nodiscard]] double FixedDeltaTime() const;
    [[nodiscard]] double DeltaTime() const;
    [[nodiscard]] double LastFrameTime() const;
};

struct UNIENGINE_API ApplicationConfigs{
    std::filesystem::path m_projectPath;
    bool m_fullScreen = false;
};
enum class UNIENGINE_API ApplicationStatus{
    WelcomeScreen,
    Initialized,
    OnDestroy
};
enum class UNIENGINE_API GameStatus{
    Stop,
    Pause,
    Step,
    Playing
};
class UNIENGINE_API Application final : ISingleton<Application>
{
    friend class EntityManager;
    friend class WindowManager;
    friend class EditorManager;
    friend class ProjectManager;
    ApplicationConfigs m_applicationConfigs;
    ApplicationStatus m_applicationStatus = ApplicationStatus::WelcomeScreen;
    GameStatus m_gameStatus = GameStatus::Stop;

    std::vector<std::function<void()>> m_externalPreUpdateFunctions;
    std::vector<std::function<void()>> m_externalUpdateFunctions;
    std::vector<std::function<void()>> m_externalFixedUpdateFunctions;
    std::vector<std::function<void()>> m_externalLateUpdateFunctions;

    static void PreUpdateInternal();
    static void UpdateInternal();
    static void LateUpdateInternal();
    ApplicationTime m_time;
    friend class Scene;
    bool m_needFixedUpdate = false;
    static void OnInspect();
    bool m_enableSettingsMenu = false;

    std::shared_ptr<Scene> m_scene;

  public:
    static bool IsPlaying();
    static void Reset();
    static ApplicationTime &Time();
    static void Play();
    static void Step();
    static void Stop();
    static void Pause();
    static GameStatus GameStatus();
    // You are only allowed to create entity after this.
    static bool IsInitialized();
    static void Init(const ApplicationConfigs& applicationConfigs);
    static void End();
    static void Run();
    static void RegisterPreUpdateFunction(const std::function<void()> &func);
    static void RegisterUpdateFunction(const std::function<void()> &func);
    static void RegisterLateUpdateFunction(const std::function<void()> &func);
    static void RegisterFixedUpdateFunction(const std::function<void()> &func);
};


} // namespace UniEngine