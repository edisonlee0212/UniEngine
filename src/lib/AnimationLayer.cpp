#include "Application.hpp"
#include "Scene.hpp"
#include "AnimationLayer.hpp"
#include "Animator.hpp"
#include "SkinnedMeshRenderer.hpp"
using namespace UniEngine;
void AnimationLayer::PreUpdate()
{
    ProfilerLayer::StartEvent("AnimationManager");
    auto scene = GetScene();
    auto *owners =
        scene->UnsafeGetPrivateComponentOwnersList<Animator>();
    if (!owners)
    {
        ProfilerLayer::EndEvent("AnimationManager");
        return;
    }
    auto &workers = Jobs::Workers();
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
                                       auto animator = scene->GetOrSetPrivateComponent<Animator>(owners->at(i)).lock();
                                      if (animator->m_animatedCurrentFrame)
                                      {
                                          animator->m_animatedCurrentFrame = false;
                                      }
                                      if (!Application::IsPlaying() && animator->m_autoPlay)
                                      {
                                          animator->AutoPlay();
                                      }
                                      animator->Apply();
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto animator = scene->GetOrSetPrivateComponent<Animator>(owners->at(i)).lock();
                                      if (animator->m_animatedCurrentFrame)
                                      {
                                          animator->m_animatedCurrentFrame = false;
                                      }
                                      if (!Application::IsPlaying() && animator->m_autoPlay)
                                      {
                                          animator->AutoPlay();
                                      }
                                      animator->Apply();
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    results.clear();

    owners = scene->UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>();
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
                                      auto smmc = scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(owners->at(i)).lock();
                                      smmc->GetBoneMatrices();
                                  }
                                  if (threadIndex < loadReminder)
                                  {
                                      const int i = threadIndex + threadSize * threadLoad;
                                      auto smmc = scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(owners->at(i)).lock();
                                      smmc->GetBoneMatrices();
                                  }
                              })
                              .share());
    }
    for (const auto &i : results)
        i.wait();
    ProfilerLayer::EndEvent("AnimationManager");
}