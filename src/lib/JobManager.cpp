#include <JobManager.hpp>
using namespace UniEngine;

void JobManager::ResizeWorkers(unsigned size)
{
    GetInstance().m_workers.FinishAll(true);
    GetInstance().m_workers.Resize(size);
}

ThreadPool &JobManager::Workers()
{
    return GetInstance().m_workers;
}

void JobManager::Init()
{
    Workers().Resize(std::thread::hardware_concurrency() - 1);
}
void JobManager::ParallelFor(
    unsigned size, const std::function<void(unsigned i)> &func, std::vector<std::shared_future<void>> &results)
{
    auto &workers = GetInstance().m_workers;
    const auto threadSize = workers.Size();
    const auto threadLoad = size / threadSize;
    const auto loadReminder = size % threadSize;
    results.reserve(results.size() + threadSize);
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(workers
                              .Push([=](int id) {
                                  for (unsigned i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                  {
                                      func(i);
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const unsigned i = threadIndex + threadSize * threadLoad;
                                      func(i);
                                  }
                              })
                              .share());
    }
}
