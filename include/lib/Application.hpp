#pragma once
#include <ClassRegistry.hpp>
#include <EntityManager.hpp>
#include <ILayer.hpp>
#include <ISingleton.hpp>
#include <InputManager.hpp>
#include <ProfilerLayer.hpp>
#include <RenderManager.hpp>
#include <Scene.hpp>
#include <Utilities.hpp>
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
    Uninitialized,
    Initialized,
    OnDestroy
};
enum class UNIENGINE_API GameStatus{
    Stop,
    Pause,
    Step,
    Playing
};
class UNIENGINE_API Application final : public ISingleton<Application>
{
    friend class EntityManager;
    friend class WindowManager;
    friend class EditorManager;
    friend class ProjectManager;
    friend class EditorLayer;
    ApplicationConfigs m_applicationConfigs;
    ApplicationStatus m_applicationStatus = ApplicationStatus::Uninitialized;
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
    static void OnInspect();
    bool m_enableSettingsMenu = false;

    std::shared_ptr<Scene> m_scene;

    std::vector<std::shared_ptr<ILayer>> m_layers;
  public:
    template <typename T>
    static std::shared_ptr<T> PushLayer();
    template <typename T>
    static std::shared_ptr<T> GetLayer();
    template <typename T>
    static void PopLayer();
    static bool IsPlaying();
    static void Reset();
    static ApplicationTime &Time();
    static void Play();
    static void Step();
    static void Stop();
    static void Pause();
    static GameStatus GetGameStatus();
    // You are only allowed to create entity after this.
    static bool IsInitialized();
    static void Create(const ApplicationConfigs& applicationConfigs);
    static void End();
    static void Start();
    static void RegisterPreUpdateFunction(const std::function<void()> &func);
    static void RegisterUpdateFunction(const std::function<void()> &func);
    static void RegisterLateUpdateFunction(const std::function<void()> &func);
    static void RegisterFixedUpdateFunction(const std::function<void()> &func);
};
template <typename T> std::shared_ptr<T> Application::PushLayer()
{
    auto& application = GetInstance();
    if(application.m_applicationStatus != ApplicationStatus::Uninitialized){
        UNIENGINE_ERROR("Unable to push layer! Application already started!");
        return nullptr;
    }
    auto test = GetLayer<T>();
    if(!test){
        test = std::make_shared<T>();
        if(!std::dynamic_pointer_cast<ILayer>(test)){
            UNIENGINE_ERROR("Not a layer!");
            return nullptr;
        }
        auto& application = GetInstance();
        application.m_layers.push_back(std::dynamic_pointer_cast<ILayer>(test));
    }
    return test;
}
template <typename T> std::shared_ptr<T> Application::GetLayer()
{
    auto& application = GetInstance();
    for(auto& i : application.m_layers){
        auto test = std::dynamic_pointer_cast<T>(i);
        if(test) return test;
    }
    return nullptr;
}
template <typename T> void Application::PopLayer()
{
    auto& application = GetInstance();
    int index = 0;
    for(auto& i : application.m_layers){
        auto test = std::dynamic_pointer_cast<T>(i);
        if(test) {
            std::dynamic_pointer_cast<ILayer>(i)->OnDestroy();
            application.m_layers.erase(application.m_layers.begin() + index);
        }
    }
}
} // namespace UniEngine