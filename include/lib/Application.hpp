#pragma once
#include <Core/FileIO.hpp>
#include <EntityManager.hpp>
#include <ISingleton.hpp>
#include <InputManager.hpp>
#include <ProfilerManager.hpp>
#include <RenderManager.hpp>
#include <Scene.hpp>

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
    void SetTimeStep(const double &value);
    [[nodiscard]] double CurrentTime() const;
    [[nodiscard]] double TimeStep() const;
    [[nodiscard]] double FixedDeltaTime() const;
    [[nodiscard]] double DeltaTime() const;
    [[nodiscard]] double LastFrameTime() const;
};

class UNIENGINE_API Application final : ISingleton<Application>
{
    friend class EntityManager;
    friend class EditorManager;

    bool m_initialized;
    bool m_playing;

    std::vector<std::function<void()>> m_externalPreUpdateFunctions;
    std::vector<std::function<void()>> m_externalUpdateFunctions;
    std::vector<std::function<void()>> m_externalFixedUpdateFunctions;
    std::vector<std::function<void()>> m_externalLateUpdateFunctions;

    static void PreUpdateInternal();
    static void UpdateInternal();
    static bool LateUpdateInternal();
    ApplicationTime m_time;
    friend class Scene;
    bool m_needFixedUpdate = false;

  public:
    static ApplicationTime &Time();
    static void SetPlaying(bool value);
    static bool IsPlaying();
    // You are only allowed to create entity after this.
    static bool IsInitialized();
    static void SetTimeStep(float value);
    static void Init(bool fullScreen = false);
    static void End();
    static void Run();
    static void RegisterPreUpdateFunction(const std::function<void()> &func);
    static void RegisterUpdateFunction(const std::function<void()> &func);
    static void RegisterLateUpdateFunction(const std::function<void()> &func);
    static void RegisterFixedUpdateFunction(const std::function<void()> &func);
};


} // namespace UniEngine