#pragma once
#include "Console.hpp"
#include "ISingleton.hpp"
#include <uniengine_export.h>
#include "ILayer.hpp"
namespace UniEngine
{
class UNIENGINE_API IProfiler
{
    friend class ProfilerLayer;
  protected:
    std::string m_name;
    virtual void PreUpdate() = 0;
    virtual void LateUpdate() = 0;
    virtual void StartEvent(const std::string &name) = 0;
    virtual void EndEvent(const std::string &name) = 0;
    virtual void OnInspect() = 0;
};
struct CPUUsageEvent
{
    std::string m_name = "";
    double m_timeStart = 0;
    double m_timeEnd = 0;
    std::vector<CPUUsageEvent> m_children;
    CPUUsageEvent *m_parent;
    CPUUsageEvent(CPUUsageEvent *parent, const std::string &name);
    void OnInspect(const float &parentTotalTime) const;
};

class UNIENGINE_API CPUTimeProfiler : public IProfiler
{
    CPUUsageEvent m_rootEvent = CPUUsageEvent(nullptr, "Main Loop");
    CPUUsageEvent *m_currentEventPointer = &m_rootEvent;
    friend class ProfilerLayer;

  protected:
    void PreUpdate() override;
    void StartEvent(const std::string &name) override;
    void EndEvent(const std::string &name) override;
    void LateUpdate() override;
    void OnInspect() override;
};
class UNIENGINE_API ProfilerLayer : public ILayer
{
    std::map<size_t, std::shared_ptr<IProfiler>> m_profilers;
    bool m_record = false;
    void PreUpdate() override;
    void LateUpdate() override;
    void OnInspect() override;
    void OnCreate() override;
  public:
    bool m_gui = false;
    template <class T = IProfiler> std::shared_ptr<T> GetOrCreateProfiler(const std::string &name);
    template <class T = IProfiler> std::shared_ptr<T> GetProfiler();
    static void StartEvent(const std::string &name);
    static void EndEvent(const std::string &name);

};
template <class T> std::shared_ptr<T> UniEngine::ProfilerLayer::GetProfiler()
{
    const auto search = m_profilers.find(typeid(T).hash_code());
    if (search != m_profilers.end())
        return std::dynamic_pointer_cast<T>(search->second);
    return nullptr;
}
template <class T> std::shared_ptr<T> UniEngine::ProfilerLayer::GetOrCreateProfiler(const std::string &name)
{
    auto profiler = GetProfiler<T>();
    if (profiler != nullptr)
        return profiler;
    profiler = std::make_shared<T>();
    profiler->m_name = name;
    m_profilers.insert({typeid(T).hash_code(), profiler});
    return profiler;
}


} // namespace UniEngine