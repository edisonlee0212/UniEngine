#include "Application.hpp"

#include <AnimationManager.hpp>
#include <Animator.hpp>
void UniEngine::AnimationManager::PreUpdate()
{
    const std::vector<Entity> *owners = EntityManager::GetPrivateComponentOwnersList<Animator>();
    if (!owners) return;
    for (auto& i : *owners)
    {
        auto &smmc = i.GetPrivateComponent<Animator>();
        if (smmc->m_autoPlay)
        {
            smmc->AutoPlay();
            smmc->Animate();
            return;
        }

        if (smmc->IsEnabled() || !smmc->m_animation)
            return;
        if (Application::IsPlaying() && smmc->m_needUpdate)
        {
            smmc->Animate();
            smmc->m_needUpdate = false;
        }
    }
}
