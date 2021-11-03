#include "Application.hpp"

#include <AnimationManager.hpp>
#include <Animator.hpp>
void UniEngine::AnimationManager::PreUpdate()
{
    ProfilerLayer::StartEvent("AnimationManager");
    const std::vector<Entity> *owners = EntityManager::UnsafeGetPrivateComponentOwnersList<Animator>(EntityManager::GetCurrentScene());
    if (!owners)
    {
        ProfilerLayer::EndEvent("AnimationManager");
        return;
    }
    auto &workers = JobManager::PrimaryWorkers();
    std::vector<std::shared_future<void>> results;
    auto threadSize = workers.Size();
    auto threadLoad = owners->size() / threadSize;
    auto loadReminder = owners->size() % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(workers
                              .Push([=](int id) {
                                  for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                  {
                                      auto animator = owners->at(i).GetOrSetPrivateComponent<Animator>().lock();
                                      animator->Animate();
                                      if (!Application::IsPlaying() && animator->m_autoPlay)
                                      {
                                          animator->AutoPlay();
                                      }
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto animator = owners->at(i).GetOrSetPrivateComponent<Animator>().lock();
                                      animator->Animate();
                                      if (!Application::IsPlaying() && animator->m_autoPlay)
                                      {
                                          animator->AutoPlay();
                                      }
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    results.clear();

    owners = EntityManager::UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>(EntityManager::GetCurrentScene());
    if (!owners)
    {
        ProfilerLayer::EndEvent("AnimationManager");
        return;
    }
    threadSize = workers.Size();
    threadLoad = owners->size() / threadSize;
    loadReminder = owners->size() % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(workers
                              .Push([=](int id) {
                                  for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                  {
                                      auto smmc = owners->at(i).GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
                                      smmc->GetBoneMatrices();
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto smmc = owners->at(i).GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
                                      smmc->GetBoneMatrices();
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    ProfilerLayer::EndEvent("AnimationManager");
}