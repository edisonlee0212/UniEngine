#include "Application.hpp"

#include <AnimationManager.hpp>
#include <Animator.hpp>
void UniEngine::AnimationManager::PreUpdate()
{
    ProfilerManager::StartEvent("AnimationManager");
    const std::vector<Entity> *owners = EntityManager::UnsafeGetPrivateComponentOwnersList<Animator>();
    if (!owners)
    {
        ProfilerManager::EndEvent("AnimationManager");
        return;
    }
    auto &workers = JobManager::PrimaryWorkers();
    std::vector<std::shared_future<void>> results;
    auto threadSize = workers.Size();
    auto threadLoad = owners->size() / threadSize;
    auto loadReminder = owners->size() % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(
            workers
                .Push([=](int id) {
                    for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                    {
                        auto &animator = owners->at(i).GetPrivateComponent<Animator>();
                        if (animator.m_autoPlay)
                        {
                            animator.AutoPlay();
                            animator.Animate();
                        }
                        else if (Application::IsPlaying() && animator.IsEnabled() && animator.m_animation)
                        {
                            animator.Animate();
                        }
                    }
                    if (threadIndex < loadReminder)
                    {
                        const int i = threadIndex + threadSize * threadLoad;
                        auto& smmc = owners->at(i).GetPrivateComponent<Animator>();
                        if (smmc.m_autoPlay)
                        {
                            smmc.AutoPlay();
                            smmc.Animate();
                        }
                        else if (Application::IsPlaying() && smmc.IsEnabled() && smmc.m_animation)
                        {
                            smmc.Animate();
                        }
                    }
                })
                .share());
    }
    for (const auto &i : results)
        i.wait();
    results.clear();

    owners = EntityManager::UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>();
    if (!owners)
    {
        ProfilerManager::EndEvent("AnimationManager");
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
                                      auto &smmc = owners->at(i).GetPrivateComponent<SkinnedMeshRenderer>();
                                      smmc.GetBoneMatrices();
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto &smmc = owners->at(i).GetPrivateComponent<SkinnedMeshRenderer>();
                                      smmc.GetBoneMatrices();
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    ProfilerManager::EndEvent("AnimationManager");
}
