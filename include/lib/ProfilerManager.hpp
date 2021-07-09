#pragma once
#include <Debug.hpp>
#include <ISingleton.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API ProfilerModule
{
  public:
    virtual void PreUpdate() = 0;
    virtual void LateUpdate() = 0;
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

class UNIENGINE_API EngineProfiler : public ProfilerModule
{
    CPUUsageEvent m_rootEvent = CPUUsageEvent(nullptr, "EngineLoop");
    CPUUsageEvent *m_currentEventPointer = &m_rootEvent;

  public:
    void PreUpdate() override;
    void StartEvent(const std::string &name);
    void EndEvent(const std::string &name);
    void LateUpdate() override;
    void OnGui() override;
};
class UNIENGINE_API ProfilerManager : public ISingleton<ProfilerManager>
{
    EngineProfiler m_engineProfiler;

  public:
    bool m_gui = true;
    static EngineProfiler &GetEngineProfiler();
    static void PreUpdate();
    static void LateUpdate();
};
} // namespace UniEngine