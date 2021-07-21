#pragma once
#include <Core/Debug.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API IProfiler
{
    friend class ProfilerManager;
  protected:
    std::string m_name;
    virtual void PreUpdate() = 0;
    virtual void LateUpdate() = 0;
    virtual void StartEvent(const std::string &name) = 0;
    virtual void EndEvent(const std::string &name) = 0;
    virtual void OnGui() = 0;
};
struct CPUUsageEvent
{
    std::string m_name = "";
    double m_timeStart = 0;
    double m_timeEnd = 0;
    std::vector<CPUUsageEvent> m_children;
    CPUUsageEvent *m_parent;
    CPUUsageEvent(CPUUsageEvent *parent, const std::string &name);
    void OnGui(const float &parentTotalTime) const;
};

class UNIENGINE_API CPUTimeProfiler : public IProfiler
{
    CPUUsageEvent m_rootEvent = CPUUsageEvent(nullptr, "Main Loop");
    CPUUsageEvent *m_currentEventPointer = &m_rootEvent;
    friend class ProfilerManager;

  protected:
    void PreUpdate() override;
    void StartEvent(const std::string &name) override;
    void EndEvent(const std::string &name) override;
    void LateUpdate() override;
    void OnGui() override;
};
class UNIENGINE_API ProfilerManager : public ISingleton<ProfilerManager>
{
    std::map<size_t, std::shared_ptr<IProfiler>> m_profilers;
    bool m_record = false;
  public:
    bool m_gui = true;
    template <class T = IProfiler> static std::shared_ptr<T> GetOrCreateProfiler(const std::string &name);
    template <class T = IProfiler> static std::shared_ptr<T> GetProfiler();
    static void PreUpdate();
    static void LateUpdate();
    static void StartEvent(const std::string &name);
    static void EndEvent(const std::string &name);
    static void OnGui();
};
template <class T> std::shared_ptr<T> UniEngine::ProfilerManager::GetProfiler()
{
    auto &profilerManager = GetInstance();
    const auto search = profilerManager.m_profilers.find(typeid(T).hash_code());
    if (search != profilerManager.m_profilers.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
template <class T> std::shared_ptr<T> UniEngine::ProfilerManager::GetOrCreateProfiler(const std::string &name)
{
    auto profiler = GetProfiler<T>();
    if (profiler != nullptr)
        return profiler;
    auto &profilerManager = GetInstance();
    profiler = std::make_shared<T>();
    profiler->m_name = name;
    profilerManager.m_profilers.insert({typeid(T).hash_code(), profiler});
    return profiler;
}

} // namespace UniEngine