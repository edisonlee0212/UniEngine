#include "Engine/ECS/Entities.hpp"
#include "Application.hpp"
#include "Engine/Utilities/Console.hpp"
#include <ProjectManager.hpp>
#include <Entity.hpp>
#include <PhysicsLayer.hpp>
#include <Scene.hpp>
using namespace UniEngine;

#pragma region EntityManager



size_t Entities::GetArchetypeChunkSize()
{
    auto &entityManager = GetInstance();
    return entityManager.m_archetypeChunkSize;
}

EntityArchetype Entities::CreateEntityArchetype(const std::string &name, const std::vector<DataComponentType> &types)
{
    auto &entityManager = GetInstance();
    EntityArchetypeInfo entityArchetypeInfo;
    entityArchetypeInfo.m_name = name;
    std::vector<DataComponentType> actualTypes;
    actualTypes.push_back(Typeof<Transform>());
    actualTypes.push_back(Typeof<GlobalTransform>());
    actualTypes.push_back(Typeof<GlobalTransformUpdateFlag>());
    actualTypes.insert(actualTypes.end(), types.begin(), types.end());
    std::sort(actualTypes.begin() + 3, actualTypes.end(), ComponentTypeComparator);
    size_t offset = 0;
    DataComponentType prev = actualTypes[0];
    // Erase duplicates
    std::vector<DataComponentType> copy;
    copy.insert(copy.begin(), actualTypes.begin(), actualTypes.end());
    actualTypes.clear();
    for (const auto &i : copy)
    {
        bool found = false;
        for (const auto j : actualTypes)
        {
            if (i == j)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        actualTypes.push_back(i);
    }

    for (auto &i : actualTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    entityArchetypeInfo.m_dataComponentTypes = actualTypes;
    entityArchetypeInfo.m_entitySize = entityArchetypeInfo.m_dataComponentTypes.back().m_offset +
                                       entityArchetypeInfo.m_dataComponentTypes.back().m_size;
    entityArchetypeInfo.m_chunkCapacity = entityManager.m_archetypeChunkSize / entityArchetypeInfo.m_entitySize;
    return CreateEntityArchetypeHelper(entityArchetypeInfo);
}

EntityArchetype Entities::GetDefaultEntityArchetype()
{
    auto &entityManager = GetInstance();
    return entityManager.m_basicArchetype;
}

EntityArchetypeInfo Entities::GetArchetypeInfo(const EntityArchetype &entityArchetype)
{
    auto &entityManager = GetInstance();
    return entityManager.m_entityArchetypeInfos[entityArchetype.m_index];
}


EntityQuery Entities::CreateEntityQuery()
{
    EntityQuery retVal;
    auto &entityManager = GetInstance();
    retVal.m_index = entityManager.m_entityQueryInfos.size();
    EntityQueryInfo info;
    info.m_index = retVal.m_index;
    entityManager.m_entityQueryInfos.resize(entityManager.m_entityQueryInfos.size() + 1);
    entityManager.m_entityQueryInfos[info.m_index] = info;
    return retVal;
}


std::string Entities::GetEntityArchetypeName(const EntityArchetype &entityArchetype)
{
    auto &entityManager = GetInstance();
    return entityManager.m_entityArchetypeInfos[entityArchetype.m_index].m_name;
}

void Entities::SetEntityArchetypeName(const EntityArchetype &entityArchetype, const std::string &name)
{
    auto &entityManager = GetInstance();
    entityManager.m_entityArchetypeInfos[entityArchetype.m_index].m_name = name;
}

void Entities::Init()
{
    auto &entityManager = GetInstance();
    entityManager.m_entityArchetypeInfos.emplace_back();
    entityManager.m_entityQueryInfos.emplace_back();

    entityManager.m_basicArchetype =
        CreateEntityArchetype("Basic", Transform(), GlobalTransform(), GlobalTransformUpdateFlag());
}

EntityArchetype Entities::CreateEntityArchetypeHelper(const EntityArchetypeInfo &info)
{
    EntityArchetype retVal = EntityArchetype();
    auto &entityManager = GetInstance();
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;
    int duplicateIndex = -1;
    for (size_t i = 1; i < entityArchetypeInfos.size(); i++)
    {
        EntityArchetypeInfo &compareInfo = entityArchetypeInfos[i];
        if (info.m_chunkCapacity != compareInfo.m_chunkCapacity)
            continue;
        if (info.m_entitySize != compareInfo.m_entitySize)
            continue;
        bool typeCheck = true;

        for (auto &componentType : info.m_dataComponentTypes)
        {
            if (!compareInfo.HasType(componentType.m_typeId))
                typeCheck = false;
        }
        if (typeCheck)
        {
            duplicateIndex = i;
            break;
        }
    }
    if (duplicateIndex == -1)
    {
        retVal.m_index = entityArchetypeInfos.size();
        entityArchetypeInfos.push_back(info);
    }
    else
    {
        retVal.m_index = duplicateIndex;
    }
    return retVal;
}

#pragma endregion
