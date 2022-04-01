//
// Created by lllll on 8/13/2021.
//
#include "IPrivateComponent.hpp"
#include <ProjectManager.hpp>
#include "Engine/ECS/Entities.hpp"
#include "Serialization.hpp"

using namespace UniEngine;

PrivateComponentElement::PrivateComponentElement(
        size_t id, const std::shared_ptr<IPrivateComponent> &data, const Entity &owner,
        const std::shared_ptr<Scene> &scene) {
    m_typeId = id;
    m_privateComponentData = data;
    m_privateComponentData->m_owner = owner;
    m_privateComponentData->m_scene = scene;
    m_privateComponentData->OnCreate();
}

void PrivateComponentElement::ResetOwner(const Entity &newOwner, const std::shared_ptr<Scene> &scene) const {
    m_privateComponentData->m_owner = newOwner;
    m_privateComponentData->m_scene = scene;
}

std::shared_ptr<Scene> IPrivateComponent::GetScene() const {
    return m_scene.lock();
}

bool IPrivateComponent::Started() {
    return m_started;
}

bool IPrivateComponent::IsEnabled() const {
    return m_enabled;
}

size_t IPrivateComponent::GetVersion() const {
    return m_version;
}

Entity IPrivateComponent::GetOwner() const {
    return m_owner;
}

void IPrivateComponent::SetEnabled(const bool &value) {
    if (m_enabled != value) {
        if (value) {
            OnEnable();
        } else {
            OnDisable();
        }
        m_enabled = value;
    }
}
