#include "Application.hpp"

#include <AnimationManager.hpp>
#include <Animator.hpp>
void UniEngine::AnimationManager::PreUpdate()
{
    const std::vector<Entity> *owners = EntityManager::GetPrivateComponentOwnersList<Animator>();
    if (!owners)
        return;
    
    auto &workers = JobManager::PrimaryWorkers();
    std::vector<std::shared_future<void>> results;
    const auto threadSize = workers.Size();
    const auto threadLoad = owners->size() / threadSize;
    const auto loadReminder = owners->size() % threadSize;
    for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
    {
        results.push_back(workers
                              .Push([=](int id) {
                                  for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                  {
                                      auto &smmc = owners->at(i).GetPrivateComponent<Animator>();
                                      if (smmc->m_autoPlay)
                                      {
                                          smmc->AutoPlay();
                                          smmc->Animate();
                                      }else if (
                                          Application::IsPlaying() && 
                                          smmc->IsEnabled() &&
                                          smmc->m_animation && smmc->m_needUpdate)
                                      {
                                          smmc->Animate();
                                          smmc->m_needUpdate = false;
                                      }
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto &smmc = owners->at(i).GetPrivateComponent<Animator>();
                                      if (smmc->m_autoPlay)
                                      {
                                          smmc->AutoPlay();
                                          smmc->Animate();
                                      }
                                      else if (
                                          Application::IsPlaying() && smmc->IsEnabled() && smmc->m_animation &&
                                          smmc->m_needUpdate)
                                      {
                                          smmc->Animate();
                                          smmc->m_needUpdate = false;
                                      }
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    /*
    for (auto &i : *owners)
    {
        auto &smmc = i.GetPrivateComponent<Animator>();
        if (smmc->m_autoPlay)
        {
            smmc->AutoPlay();
            smmc->Animate();
            continue;
        }

        if (smmc->IsEnabled() || !smmc->m_animation)
            continue;
        if (Application::IsPlaying() && smmc->m_needUpdate)
        {
            smmc->Animate();
            smmc->m_needUpdate = false;
        }
    }
    */
}
