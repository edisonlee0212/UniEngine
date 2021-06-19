#include <EntityManager.hpp>
#include <World.hpp>
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

void World::RegisterFixedUpdateFunction(const std::function<void()> &func)
{
    m_externalFixedUpdateFunctions.push_back(func);
}

Bound World::GetBound() const
{
    return m_worldBound;
}

void World::SetBound(const Bound &value)
{
    m_worldBound = value;
}

void World::SetFrameStartTime(double time) const
{
    m_time->m_frameStartTime = time;
}

void World::SetTimeStep(float timeStep) const
{
    m_time->m_timeStep = timeStep;
}

size_t World::GetIndex() const
{
    return m_index;
}

World::World(size_t index)
{
    m_index = index;
    m_time = new WorldTime();
    m_worldEntityStorage = WorldEntityStorage();
    m_worldEntityStorage.m_entities.emplace_back();
    m_worldEntityStorage.m_entityInfos.emplace_back();
    m_worldEntityStorage.m_entityComponentStorage.emplace_back(nullptr, nullptr);
    m_worldEntityStorage.m_entityQueries.emplace_back();
    m_worldEntityStorage.m_entityQueryInfos.emplace_back();
}

void World::ResetTime() const
{
    m_time->m_deltaTime = 0;
    m_time->m_lastFrameTime = 0;
    m_time->m_fixedDeltaTime = 0;
}

World::~World()
{
    Purge();
    for (auto i : m_preparationSystems)
    {
        i->OnDestroy();
        delete i;
    }
    for (auto i : m_simulationSystems)
    {
        i->OnDestroy();
        delete i;
    }
    for (auto i : m_presentationSystems)
    {
        i->OnDestroy();
        delete i;
    }
    delete m_time;
    if (EntityManager::GetInstance().m_currentAttachedWorldEntityStorage == &m_worldEntityStorage)
    {
        EntityManager::Detach();
    }
}

void World::PreUpdate()
{
    m_needFixedUpdate = false;
    if (m_time->m_fixedDeltaTime >= m_time->m_timeStep)
    {
        m_needFixedUpdate = true;
    }

    if (m_needFixedUpdate)
    {
        for (const auto &i : m_externalFixedUpdateFunctions)
            i();
        for (auto i : m_preparationSystems)
        {
            if (i->Enabled())
                i->FixedUpdate();
        }
        for (auto i : m_simulationSystems)
        {
            if (i->Enabled())
                i->FixedUpdate();
        }
        for (auto i : m_presentationSystems)
        {
            if (i->Enabled())
                i->FixedUpdate();
        }
    }

    for (auto i : m_preparationSystems)
    {
        if (i->Enabled())
            i->PreUpdate();
    }

    for (auto i : m_simulationSystems)
    {
        if (i->Enabled())
            i->PreUpdate();
    }

    for (auto i : m_presentationSystems)
    {
        if (i->Enabled())
            i->PreUpdate();
    }
}

void World::Update()
{

    for (auto i : m_preparationSystems)
    {
        if (i->Enabled())
            i->Update();
    }
    for (auto i : m_simulationSystems)
    {
        if (i->Enabled())
            i->Update();
    }
    for (auto i : m_presentationSystems)
    {
        if (i->Enabled())
            i->Update();
    }
}

void World::LateUpdate()
{
    for (auto i : m_preparationSystems)
    {
        if (i->Enabled())
            i->LateUpdate();
    }

    for (auto i : m_simulationSystems)
    {
        if (i->Enabled())
            i->LateUpdate();
    }

    for (auto i : m_presentationSystems)
    {
        if (i->Enabled())
            i->LateUpdate();
    }
    if (m_needFixedUpdate)
    {
        m_time->m_fixedDeltaTime = 0;
    }
}

WorldTime *World::Time() const
{
    return m_time;
}
