#pragma once
#include <ISingleton.hpp>
#include <ThreadPool.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class JobManager final : ISingleton<JobManager>
{
    ThreadPool m_primaryWorkers;
    ThreadPool m_secondaryWorkers;

  public:
    UNIENGINE_API static void ResizePrimaryWorkers(int size);
    UNIENGINE_API static void ResizeSecondaryWorkers(int size);
    UNIENGINE_API static ThreadPool &PrimaryWorkers();
    UNIENGINE_API static ThreadPool &SecondaryWorkers();
};
} // namespace UniEngine
