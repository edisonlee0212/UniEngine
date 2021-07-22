#include <Application.hpp>

#include <EntityManager.hpp>
#include <Scene.hpp>

#include <PhysicsManager.hpp>

using namespace UniEngine;

void Scene::Purge()
{
    m_sceneDataStorage.m_entityPrivateComponentStorage = PrivateComponentStorage();
    m_sceneDataStorage.m_entities.clear();
    m_sceneDataStorage.m_entityInfos.clear();
    m_sceneDataStorage.m_parentHierarchyVersion = 0;
    for (int index = 1; index < m_sceneDataStorage.m_entityComponentStorage.size(); index++)
    {
        auto &i = m_sceneDataStorage.m_entityComponentStorage[index];
        for (auto &chunk : i.m_chunkArray.Chunks)
            free(chunk.m_data);
        i.m_chunkArray.Chunks.clear();
        i.m_chunkArray.Entities.clear();
        i.m_archetypeInfo.m_entityAliveCount = 0;
        i.m_archetypeInfo.m_entityCount = 0;
    }

    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityInfos.emplace_back();
}

Bound Scene::GetBound() const
{
    return m_worldBound;
}

void Scene::SetBound(const Bound &value)
{
    m_worldBound = value;
}

size_t Scene::GetIndex() const
{
    return m_index;
}

Scene::Scene(size_t index)
{
    m_index = index;
    m_sceneDataStorage = SceneDataStorage();
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityInfos.emplace_back();
    m_sceneDataStorage.m_entityComponentStorage.emplace_back();
    m_sceneDataStorage.m_entityQueries.emplace_back();
    m_sceneDataStorage.m_entityQueryInfos.emplace_back();
}

Scene::~Scene()
{
    Purge();
    for (auto &i : m_systems)
    {
        i.second->OnDestroy();
    }
    if (EntityManager::GetInstance().m_currentAttachedWorldEntityStorage == &m_sceneDataStorage)
    {
        EntityManager::Detach();
    };
}

void Scene::PreUpdate()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerManager::StartEvent(i.second->m_name);
            i.second->PreUpdate();
            ProfilerManager::EndEvent(i.second->m_name);
        }
    }
}

void Scene::Update()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerManager::StartEvent(i.second->m_name);
            i.second->Update();
            ProfilerManager::EndEvent(i.second->m_name);
        }
    }
}

void Scene::LateUpdate()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled()){
            ProfilerManager::StartEvent(i.second->m_name);
            i.second->LateUpdate();
            ProfilerManager::EndEvent(i.second->m_name);
        }
    }
}
void Scene::FixedUpdate()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled()){
            ProfilerManager::StartEvent(i.second->m_name);
            i.second->FixedUpdate();
            ProfilerManager::EndEvent(i.second->m_name);
        }
    }
}

void Scene::OnGui()
{
    for (auto &i : m_systems)
    {
        i.second->OnGui();
    }
}