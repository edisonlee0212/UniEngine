#pragma once
#include <ISingleton.hpp>
#include <ThreadPool.hpp>
#include <uniengine_export.h>
namespace UniEngine
{
class UNIENGINE_API JobManager final : ISingleton<JobManager>
{
    ThreadPool m_workers;

  public:
    static void ResizeWorkers(unsigned size);
    static ThreadPool &Workers();
    static void Init();
    static void ParallelFor(unsigned size, const std::function<void(unsigned i)> &func, std::vector<std::shared_future<void>>& results);
};
} // namespace UniEngine
