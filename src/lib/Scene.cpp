#include <Application.hpp>

#include <EntityManager.hpp>
#include <Scene.hpp>

using namespace UniEngine;

void Scene::Purge()
{
    m_sceneDataStorage.m_entityPrivateComponentStorage = PrivateComponentStorage();
    m_sceneDataStorage.m_entities.clear();
    m_sceneDataStorage.m_entityInfos.clear();
    for (int index = 1; index < m_sceneDataStorage.m_dataComponentStorages.size(); index++)
    {
        auto &i = m_sceneDataStorage.m_dataComponentStorages[index];
        for (auto &chunk : i.m_chunkArray.m_chunks)
            free(chunk.m_data);
    }
    m_sceneDataStorage.m_dataComponentStorages.clear();


    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityInfos.emplace_back();

    if(EntityManager::GetCurrentScene().get() == this){
        for(auto& i : EntityManager::GetInstance().m_entityArchetypeInfos){
            i.m_dataComponentStorageIndex = 0;
        }
        for(auto& i : EntityManager::GetInstance().m_entityQueryInfos){
            i.m_queriedStorage.clear();
        }
    }
}

Bound Scene::GetBound() const
{
    return m_worldBound;
}

void Scene::SetBound(const Bound &value)
{
    m_worldBound = value;
}

Scene::Scene()
{
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityInfos.emplace_back();
    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
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
            ProfilerManager::StartEvent(i.second->GetTypeName());
            i.second->PreUpdate();
            ProfilerManager::EndEvent(i.second->GetTypeName());
        }
    }
}

void Scene::Update()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerManager::StartEvent(i.second->GetTypeName());
            i.second->Update();
            ProfilerManager::EndEvent(i.second->GetTypeName());
        }
    }
}

void Scene::LateUpdate()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled()){
            ProfilerManager::StartEvent(i.second->GetTypeName());
            i.second->LateUpdate();
            ProfilerManager::EndEvent(i.second->GetTypeName());
        }
    }
}
void Scene::FixedUpdate()
{
    for (auto &i : m_systems)
    {
        if (i.second->Enabled()){
            ProfilerManager::StartEvent(i.second->GetTypeName());
            i.second->FixedUpdate();
            ProfilerManager::EndEvent(i.second->GetTypeName());
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