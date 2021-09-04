//
// Created by lllll on 8/23/2021.
//

#include "UnknownPrivateComponent.hpp"

void UniEngine::UnknownPrivateComponent::OnInspect()
{
}
void UniEngine::UnknownPrivateComponent::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<UnknownPrivateComponent>(target);
}
