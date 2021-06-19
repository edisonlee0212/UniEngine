#pragma once
#include <EntityManager.hpp>
#include <FileIO.hpp>
#include <ISingleton.hpp>
#include <RenderManager.hpp>
#include <World.hpp>
namespace UniEngine
{
class UNIENGINE_API Application final : ISingleton<Application>
{
    friend class EntityManager;
    friend class EditorManager;
    std::unique_ptr<World> m_world;
    bool m_initialized;
    float m_timeStep;
    bool m_innerLooping;
    bool m_playing;

    std::vector<std::function<void()>> m_externalPreUpdateFunctions;
    std::vector<std::function<void()>> m_externalUpdateFunctions;
    std::vector<std::function<void()>> m_externalLateUpdateFunctions;

    static void PreUpdateInternal();
    static void UpdateInternal();
    static bool LateUpdateInternal();

  public:
    static double EngineTime();
    static void SetPlaying(bool value);
    static bool IsPlaying();
    // You are only allowed to create entity after this.
    static bool IsInitialized();
    static void SetTimeStep(float value);
    static void Init(bool fullScreen = false);
    static void End();
    static void Run();
    static std::unique_ptr<World> &GetCurrentWorld();

    static void RegisterPreUpdateFunction(const std::function<void()> &func);
    static void RegisterUpdateFunction(const std::function<void()> &func);
    static void RegisterLateUpdateFunction(const std::function<void()> &func);
};
} // namespace UniEngine