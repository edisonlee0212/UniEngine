#include <Application.hpp>

#include <EntityManager.hpp>
#include <World.hpp>

#include <PhysicsManager.hpp>

using namespace UniEngine;

void World::Purge()
{
    m_worldEntityStorage.m_entityPrivateComponentStorage = PrivateComponentStorage();
    m_worldEntityStorage.m_entities.clear();
    m_worldEntityStorage.m_entityInfos.clear();
    m_worldEntityStorage.m_parentHierarchyVersion = 0;
    for (int index = 1; index < m_worldEntityStorage.m_entityComponentStorage.size(); index++)
    {
        auto &i = m_worldEntityStorage.m_entityComponentStorage[index];
        for (auto &chunk : i.m_chunkArray->Chunks)
            free(chunk.m_data);
        i.m_chunkArray->Chunks.clear();
        i.m_chunkArray->Entities.clear();
        i.m_archetypeInfo->m_entityAliveCount = 0;
        i.m_archetypeInfo->m_entityCount = 0;
    }

    m_worldEntityStorage.m_entities.emplace_back();
    m_worldEntityStorage.m_entityInfos.emplace_back();
}

Bound World::GetBound() const
{
    return m_worldBound;
}

void World::SetBound(const Bound &value)
{
    m_worldBound = value;
}

size_t World::GetIndex() const
{
    return m_index;
}

World::World(size_t index)
{
    m_index = index;
    m_worldEntityStorage = WorldEntityStorage();
    m_worldEntityStorage.m_entities.emplace_back();
    m_worldEntityStorage.m_entityInfos.emplace_back();
    m_worldEntityStorage.m_entityComponentStorage.emplace_back(nullptr, nullptr);
    m_worldEntityStorage.m_entityQueries.emplace_back();
    m_worldEntityStorage.m_entityQueryInfos.emplace_back();

    CreateSystem<PhysicsSystem>("PhysicsSystem", SystemGroup::SimulationSystemGroup);
}

World::~World()
{
    Purge();
    for (auto &i : m_systems)
    {
        i.second->OnDestroy();
    }
    if (EntityManager::GetInstance().m_currentAttachedWorldEntityStorage == &m_worldEntityStorage)
    {
        EntityManager::Detach();
    };
}

void World::PreUpdate()
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

void World::Update()
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

void World::LateUpdate()
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
void World::FixedUpdate()
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

void World::OnGui()
{
    for (auto &i : m_systems)
    {
        i.second->OnGui();
    }
}