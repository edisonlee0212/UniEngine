#include <Core/Debug.hpp>
#include <EntityManager.hpp>
#include <World.hpp>
using namespace UniEngine;

#pragma region EntityManager

void EntityManager::UnsafeForEachComponent(
    const Entity &entity, const std::function<void(const ComponentDataType &type, void *data)> &func)
{
    if (!entity.IsValid())
        return;
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);
    if (GetInstance().m_entities->at(entity.m_index) == entity)
    {
        EntityArchetypeInfo *chunkInfo =
            GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_archetypeInfo;
        const size_t chunkIndex = info.m_chunkArrayIndex / chunkInfo->m_chunkCapacity;
        const size_t chunkPointer = info.m_chunkArrayIndex % chunkInfo->m_chunkCapacity;
        const ComponentDataChunk chunk =
            GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_chunkArray->Chunks[chunkIndex];
        for (const auto &i : chunkInfo->m_componentTypes)
        {
            func(
                i,
                static_cast<void *>(
                    static_cast<char *>(chunk.m_data) + i.m_offset * chunkInfo->m_chunkCapacity +
                    chunkPointer * i.m_size));
        }
    }
}

void EntityManager::ForEachPrivateComponent(
    const Entity &entity, const std::function<void(PrivateComponentElement &data)> &func)
{
    if (!entity.IsValid())
        return;
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);
    if (GetInstance().m_entities->at(entity.m_index) == entity)
    {
        for (auto &component : info.m_privateComponentElements)
        {
            func(component);
        }
    }
}

void EntityManager::UnsafeForEachEntityStorage(
    const std::function<void(int i, const EntityComponentDataStorage &storage)> &func)
{
    for (size_t i = 1; i < GetInstance().m_entityComponentStorage->size(); i++)
    {
        if (GetInstance().m_entityComponentStorage->at(i).m_archetypeInfo->m_entityAliveCount != 0)
        {
            func(i, GetInstance().m_entityComponentStorage->at(i));
        }
    }
}

size_t EntityManager::GetArchetypeChunkSize()
{
    return GetInstance().m_archetypeChunkSize;
}

void EntityManager::DeleteEntityInternal(const Entity &entity)
{
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);
    Entity actualEntity = GetInstance().m_entities->at(entity.m_index);
    if (actualEntity == entity)
    {
        GetInstance().m_entityPrivateComponentStorage->DeleteEntity(actualEntity);
        info.m_version = actualEntity.m_version + 1;
        info.m_static = false;
        info.m_enabled = true;

        info.m_privateComponentElements.clear();
        // Set to version 0, marks it as deleted.
        actualEntity.m_version = 0;
        EntityComponentDataStorage storage = GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex);
        storage.m_chunkArray->Entities[info.m_chunkArrayIndex] = actualEntity;
        const auto originalIndex = info.m_chunkArrayIndex;
        if (info.m_chunkArrayIndex != storage.m_archetypeInfo->m_entityAliveCount - 1)
        {
            const auto swappedIndex =
                SwapEntity(storage, info.m_chunkArrayIndex, storage.m_archetypeInfo->m_entityAliveCount - 1);
            info.m_chunkArrayIndex = storage.m_archetypeInfo->m_entityAliveCount - 1;
            GetInstance().m_entityInfos->at(swappedIndex).m_chunkArrayIndex = originalIndex;
        }
        storage.m_archetypeInfo->m_entityAliveCount--;
    }
    else
    {
        UNIENGINE_ERROR("Entity already deleted!");
        return;
    }
    GetInstance().m_entities->at(entity.m_index) = actualEntity;
}

void EntityManager::RefreshEntityQueryInfos(const size_t &index)
{
    GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.clear();
    // Select storage with every contained.
    if (!GetInstance().m_entityQueryInfos->at(index).m_allComponentTypes.empty())
    {
        for (auto i : *GetInstance().m_entityComponentStorage)
        {
            if (i.m_archetypeInfo == nullptr)
                continue;
            bool check = true;
            for (const auto &type : GetInstance().m_entityQueryInfos->at(index).m_allComponentTypes)
            {
                if (!i.m_archetypeInfo->HasType(type.m_typeId))
                    check = false;
            }
            if (check)
                GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.push_back(i);
        }
    }
    else
    {
        for (auto i : *GetInstance().m_entityComponentStorage)
        {
            if (i.m_archetypeInfo == nullptr)
                continue;
            GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.push_back(i);
        }
    }
    // Erase with any
    if (!GetInstance().m_entityQueryInfos->at(index).m_anyComponentTypes.empty())
    {
        for (int i = 0; i < GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.size(); i++)
        {
            bool contain = false;
            for (const auto &type : GetInstance().m_entityQueryInfos->at(index).m_anyComponentTypes)
            {
                if (GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.at(i).m_archetypeInfo->HasType(
                        type.m_typeId))
                    contain = true;
                if (contain)
                    break;
            }
            if (!contain)
            {
                GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.erase(
                    GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.begin() + i);
                i--;
            }
        }
    }
    // Erase with none
    if (!GetInstance().m_entityQueryInfos->at(index).m_noneComponentTypes.empty())
    {
        for (int i = 0; i < GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.size(); i++)
        {
            bool contain = false;
            for (const auto &type : GetInstance().m_entityQueryInfos->at(index).m_noneComponentTypes)
            {
                if (GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.at(i).m_archetypeInfo->HasType(
                        type.m_typeId))
                    contain = true;
                if (contain)
                    break;
            }
            if (contain)
            {
                GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.erase(
                    GetInstance().m_entityQueryInfos->at(index).m_queriedStorage.begin() + i);
                i--;
            }
        }
    }
}

void EntityManager::EraseDuplicates(std::vector<ComponentDataType> &types)
{
    std::vector<ComponentDataType> copy;
    copy.insert(copy.begin(), types.begin(), types.end());
    types.clear();
    for (const auto &i : copy)
    {
        bool found = false;
        for (const auto j : types)
        {
            if (i == j)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        types.push_back(i);
    }
}

void EntityManager::GetEntityStorage(const EntityComponentDataStorage &storage, std::vector<Entity> &container)
{
    const size_t amount = storage.m_archetypeInfo->m_entityAliveCount;
    if (amount == 0)
        return;
    container.resize(container.size() + amount);
    const size_t capacity = storage.m_archetypeInfo->m_chunkCapacity;
    memcpy(&container.at(container.size() - amount), storage.m_chunkArray->Entities.data(), amount * sizeof(Entity));
}

size_t EntityManager::SwapEntity(const EntityComponentDataStorage &storage, size_t index1, size_t index2)
{
    if (index1 == index2)
        return -1;
    auto *info = storage.m_archetypeInfo;
    const size_t retVal = storage.m_chunkArray->Entities[index2].m_index;
    const auto other = storage.m_chunkArray->Entities[index2];
    storage.m_chunkArray->Entities[index2] = storage.m_chunkArray->Entities[index1];
    storage.m_chunkArray->Entities[index1] = other;
    const auto capacity = storage.m_archetypeInfo->m_chunkCapacity;
    const auto chunkIndex1 = index1 / capacity;
    const auto chunkIndex2 = index2 / capacity;
    const auto chunkPointer1 = index1 % capacity;
    const auto chunkPointer2 = index2 % capacity;

    for (const auto &i : storage.m_archetypeInfo->m_componentTypes)
    {
        void *temp = static_cast<void *>(malloc(i.m_size));
        void *d1 = static_cast<void *>(
            static_cast<char *>(storage.m_chunkArray->Chunks[chunkIndex1].m_data) + i.m_offset * capacity +
            i.m_size * chunkPointer1);

        void *d2 = static_cast<void *>(
            static_cast<char *>(storage.m_chunkArray->Chunks[chunkIndex2].m_data) + i.m_offset * capacity +
            i.m_size * chunkPointer2);

        memcpy(temp, d1, i.m_size);
        memcpy(d1, d2, i.m_size);
        memcpy(d2, temp, i.m_size);
        free(temp);
    }
    return retVal;
}

void EntityManager::GetAllEntities(std::vector<Entity> &target)
{
    target.insert(target.end(), GetInstance().m_entities->begin() + 1, GetInstance().m_entities->end());
}

void EntityManager::Detach()
{
    GetInstance().m_currentAttachedWorldEntityStorage = nullptr;
    GetInstance().m_entities = nullptr;
    GetInstance().m_entityInfos = nullptr;
    GetInstance().m_entityComponentStorage = nullptr;
    GetInstance().m_entityPrivateComponentStorage = nullptr;
    GetInstance().m_entityQueries = nullptr;
    GetInstance().m_entityQueryInfos = nullptr;
    GetInstance().m_entityQueryPools = nullptr;
}

void EntityManager::ForEachDescendant(
    const Entity &target, const std::function<void(const Entity &entity)> &func, const bool &fromRoot)
{
    Entity realTarget = target;
    if (!realTarget.IsValid())
        return;
    if (fromRoot)
        realTarget = GetRoot(realTarget);
    ForEachDescendantHelper(realTarget, func);
}

std::vector<Entity> *EntityManager::UnsafeGetAllEntities()
{
    return GetInstance().m_entities;
}

void EntityManager::Attach(std::unique_ptr<World> &world)
{
    WorldEntityStorage *targetStorage = &world->m_worldEntityStorage;
    GetInstance().m_currentAttachedWorldEntityStorage = targetStorage;
    GetInstance().m_entities = &targetStorage->m_entities;
    GetInstance().m_entityInfos = &targetStorage->m_entityInfos;
    GetInstance().m_entityComponentStorage = &targetStorage->m_entityComponentStorage;
    GetInstance().m_entityPrivateComponentStorage = &targetStorage->m_entityPrivateComponentStorage;
    GetInstance().m_entityQueries = &targetStorage->m_entityQueries;
    GetInstance().m_entityQueryInfos = &targetStorage->m_entityQueryInfos;
    GetInstance().m_entityQueryPools = &targetStorage->m_entityQueryPools;
    GetInstance().m_basicArchetype = CreateEntityArchetype("Basic", Transform(), GlobalTransform());
}

Entity EntityManager::CreateEntity(const std::string &name)
{
    if (!GetInstance().m_currentAttachedWorldEntityStorage)
    {
        UNIENGINE_ERROR("EntityManager not attached to any world!");
        return Entity();
    }
    return CreateEntity(GetInstance().m_basicArchetype, name);
}

Entity EntityManager::CreateEntity(const EntityArchetype &archetype, const std::string &name)
{
    if (!GetInstance().m_currentAttachedWorldEntityStorage)
    {
        UNIENGINE_ERROR("EntityManager not attached to any world!");
        return Entity();
    }
    if (archetype.IsValid())
        return Entity();
    Entity retVal;
    EntityComponentDataStorage storage = GetInstance().m_entityComponentStorage->at(archetype.m_index);
    EntityArchetypeInfo *info = storage.m_archetypeInfo;
    if (info->m_entityCount == info->m_entityAliveCount)
    {
        const size_t chunkIndex = info->m_entityCount / info->m_chunkCapacity + 1;
        if (storage.m_chunkArray->Chunks.size() <= chunkIndex)
        {
            // Allocate new chunk;
            ComponentDataChunk chunk;
            chunk.m_data = static_cast<void *>(calloc(1, GetInstance().m_archetypeChunkSize));
            storage.m_chunkArray->Chunks.push_back(chunk);
        }
        retVal.m_index = GetInstance().m_entities->size();
        // If the version is 0 in chunk means it's deleted.
        retVal.m_version = 1;
        EntityInfo entityInfo;
        entityInfo.m_name = name;
        entityInfo.m_archetypeInfoIndex = archetype.m_index;
        entityInfo.m_chunkArrayIndex = info->m_entityCount;
        storage.m_chunkArray->Entities.push_back(retVal);
        GetInstance().m_entityInfos->push_back(std::move(entityInfo));
        GetInstance().m_entities->push_back(retVal);
        info->m_entityCount++;
        info->m_entityAliveCount++;
    }
    else
    {
        retVal = storage.m_chunkArray->Entities.at(info->m_entityAliveCount);
        EntityInfo &entityInfo = GetInstance().m_entityInfos->at(retVal.m_index);
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        retVal.m_version = entityInfo.m_version;
        storage.m_chunkArray->Entities[entityInfo.m_chunkArrayIndex] = retVal;
        GetInstance().m_entities->at(retVal.m_index) = retVal;
        storage.m_archetypeInfo->m_entityAliveCount++;
        // Reset all component data
        const auto chunkIndex = entityInfo.m_chunkArrayIndex / info->m_chunkCapacity;
        const auto chunkPointer = entityInfo.m_chunkArrayIndex % info->m_chunkCapacity;
        const auto chunk = GetInstance()
                               .m_entityComponentStorage->at(entityInfo.m_archetypeInfoIndex)
                               .m_chunkArray->Chunks[chunkIndex];
        for (const auto &i : info->m_componentTypes)
        {
            const auto offset = i.m_offset * info->m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
    }
    retVal.SetComponentData(Transform());
    retVal.SetComponentData(GlobalTransform());
    return retVal;
}

std::vector<Entity> EntityManager::CreateEntities(
    const EntityArchetype &archetype, const size_t &amount, const std::string &name)
{
    if (!GetInstance().m_currentAttachedWorldEntityStorage)
    {
        UNIENGINE_ERROR("EntityManager not attached to any world!");
        return std::vector<Entity>();
    }
    if (archetype.IsValid())
        return std::vector<Entity>();
    std::vector<Entity> retVal;
    EntityComponentDataStorage storage = GetInstance().m_entityComponentStorage->at(archetype.m_index);
    EntityArchetypeInfo *info = storage.m_archetypeInfo;
    auto remainAmount = amount;
    const Transform transform;
    const GlobalTransform globalTransform;
    while (remainAmount > 0 && info->m_entityAliveCount != info->m_entityCount)
    {
        remainAmount--;
        Entity entity = storage.m_chunkArray->Entities.at(info->m_entityAliveCount);
        EntityInfo &entityInfo = GetInstance().m_entityInfos->at(entity.m_index);
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        entity.m_version = entityInfo.m_version;
        storage.m_chunkArray->Entities[entityInfo.m_chunkArrayIndex] = entity;
        GetInstance().m_entities->at(entity.m_index) = entity;
        info->m_entityAliveCount++;
        // Reset all component data
        const size_t chunkIndex = entityInfo.m_chunkArrayIndex / info->m_chunkCapacity;
        const size_t chunkPointer = entityInfo.m_chunkArrayIndex % info->m_chunkCapacity;
        const ComponentDataChunk chunk = GetInstance()
                                             .m_entityComponentStorage->at(entityInfo.m_archetypeInfoIndex)
                                             .m_chunkArray->Chunks[chunkIndex];
        for (const auto &i : info->m_componentTypes)
        {
            const size_t offset = i.m_offset * info->m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
        retVal.push_back(entity);
        entity.SetComponentData(transform);
        entity.SetComponentData(globalTransform);
    }
    if (remainAmount == 0)
        return retVal;
    info->m_entityCount += remainAmount;
    info->m_entityAliveCount += remainAmount;
    const size_t chunkIndex = info->m_entityCount / info->m_chunkCapacity + 1;
    while (storage.m_chunkArray->Chunks.size() <= chunkIndex)
    {
        // Allocate new chunk;
        ComponentDataChunk chunk;
        chunk.m_data = static_cast<void *>(calloc(1, GetInstance().m_archetypeChunkSize));
        storage.m_chunkArray->Chunks.push_back(chunk);
    }
    const size_t originalSize = GetInstance().m_entities->size();
    GetInstance().m_entities->resize(originalSize + remainAmount);
    GetInstance().m_entityInfos->resize(originalSize + remainAmount);

    for (int i = 0; i < remainAmount; i++)
    {
        auto &entity = GetInstance().m_entities->at(originalSize + i);
        entity.m_index = originalSize + i;
        entity.m_version = 1;

        auto &entityInfo = GetInstance().m_entityInfos->at(originalSize + i);
        entityInfo = EntityInfo();
        entityInfo.m_name = name;
        entityInfo.m_archetypeInfoIndex = archetype.m_index;
        entityInfo.m_chunkArrayIndex = info->m_entityAliveCount - remainAmount + i;
    }

    storage.m_chunkArray->Entities.insert(
        storage.m_chunkArray->Entities.end(),
        GetInstance().m_entities->begin() + originalSize,
        GetInstance().m_entities->end());
    const int threadSize = JobManager::PrimaryWorkers().Size();
    int perThreadAmount = remainAmount / threadSize;
    if (perThreadAmount > 0)
    {
        std::vector<std::shared_future<void>> results;
        for (int i = 0; i < threadSize; i++)
        {
            results.push_back(JobManager::PrimaryWorkers()
                                  .Push([i, perThreadAmount, originalSize](int id) {
                                      const Transform transform;
                                      const GlobalTransform globalTransform;
                                      for (int index = originalSize + i * perThreadAmount;
                                           index < originalSize + (i + 1) * perThreadAmount;
                                           index++)
                                      {
                                          auto &entity = GetInstance().m_entities->at(index);
                                          entity.SetComponentData(transform);
                                          entity.SetComponentData(globalTransform);
                                      }
                                  })
                                  .share());
        }
        results.push_back(JobManager::PrimaryWorkers()
                              .Push([perThreadAmount, originalSize, &remainAmount, threadSize](int id) {
                                  const Transform transform;
                                  const GlobalTransform globalTransform;
                                  for (int index = originalSize + perThreadAmount * threadSize;
                                       index < originalSize + remainAmount;
                                       index++)
                                  {
                                      auto &entity = GetInstance().m_entities->at(index);
                                      entity.SetComponentData(transform);
                                      entity.SetComponentData(globalTransform);
                                  }
                              })
                              .share());
        for (const auto &i : results)
            i.wait();
    }

    retVal.insert(retVal.end(), GetInstance().m_entities->begin() + originalSize, GetInstance().m_entities->end());
    return retVal;
}

std::vector<Entity> EntityManager::CreateEntities(const size_t &amount, const std::string &name)
{
    auto &manager = GetInstance();
    if (!manager.m_currentAttachedWorldEntityStorage)
    {
        UNIENGINE_ERROR("EntityManager not attached to any world!");
        return std::vector<Entity>();
    }
    const auto archetype = manager.m_basicArchetype;
    if (archetype.IsValid())
        return std::vector<Entity>();
    std::vector<Entity> retVal;
    EntityComponentDataStorage storage = manager.m_entityComponentStorage->at(archetype.m_index);
    EntityArchetypeInfo *info = storage.m_archetypeInfo;
    auto remainAmount = amount;
    const Transform transform;
    const GlobalTransform globalTransform;
    while (remainAmount > 0 && info->m_entityAliveCount != info->m_entityCount)
    {
        remainAmount--;
        Entity entity = storage.m_chunkArray->Entities.at(info->m_entityAliveCount);
        EntityInfo &entityInfo = manager.m_entityInfos->at(entity.m_index);
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        entity.m_version = entityInfo.m_version;
        storage.m_chunkArray->Entities[entityInfo.m_chunkArrayIndex] = entity;
        manager.m_entities->at(entity.m_index) = entity;
        info->m_entityAliveCount++;
        // Reset all component data
        const size_t chunkIndex = entityInfo.m_chunkArrayIndex / info->m_chunkCapacity;
        const size_t chunkPointer = entityInfo.m_chunkArrayIndex % info->m_chunkCapacity;
        const ComponentDataChunk chunk =
            manager.m_entityComponentStorage->at(entityInfo.m_archetypeInfoIndex).m_chunkArray->Chunks[chunkIndex];
        for (const auto &i : info->m_componentTypes)
        {
            const size_t offset = i.m_offset * info->m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
        retVal.push_back(entity);
        entity.SetComponentData(transform);
        entity.SetComponentData(globalTransform);
    }
    if (remainAmount == 0)
        return retVal;
    info->m_entityCount += remainAmount;
    info->m_entityAliveCount += remainAmount;
    const size_t chunkIndex = info->m_entityCount / info->m_chunkCapacity + 1;
    while (storage.m_chunkArray->Chunks.size() <= chunkIndex)
    {
        // Allocate new chunk;
        ComponentDataChunk chunk;
        chunk.m_data = static_cast<void *>(calloc(1, manager.m_archetypeChunkSize));
        storage.m_chunkArray->Chunks.push_back(chunk);
    }
    const size_t originalSize = manager.m_entities->size();
    manager.m_entities->resize(originalSize + remainAmount);
    manager.m_entityInfos->resize(originalSize + remainAmount);

    for (int i = 0; i < remainAmount; i++)
    {
        auto &entity = manager.m_entities->at(originalSize + i);
        entity.m_index = originalSize + i;
        entity.m_version = 1;

        auto &entityInfo = manager.m_entityInfos->at(originalSize + i);
        entityInfo = EntityInfo();
        entityInfo.m_name = name;
        entityInfo.m_archetypeInfoIndex = archetype.m_index;
        entityInfo.m_chunkArrayIndex = info->m_entityAliveCount - remainAmount + i;
        entity.SetComponentData(transform);
        entity.SetComponentData(globalTransform);
    }

    storage.m_chunkArray->Entities.insert(
        storage.m_chunkArray->Entities.end(), manager.m_entities->begin() + originalSize, manager.m_entities->end());
    const int threadSize = JobManager::PrimaryWorkers().Size();
    int perThreadAmount = remainAmount / threadSize;
    if (perThreadAmount > 0)
    {
        std::vector<std::shared_future<void>> results;
        for (int i = 0; i < threadSize; i++)
        {
            results.push_back(JobManager::PrimaryWorkers()
                                  .Push([i, perThreadAmount, originalSize](int id) {
                                      const Transform transform;
                                      const GlobalTransform globalTransform;
                                      for (int index = originalSize + i * perThreadAmount;
                                           index < originalSize + (i + 1) * perThreadAmount;
                                           index++)
                                      {
                                          auto &entity = GetInstance().m_entities->at(index);
                                          entity.SetComponentData(transform);
                                          entity.SetComponentData(globalTransform);
                                      }
                                  })
                                  .share());
        }
        results.push_back(JobManager::PrimaryWorkers()
                              .Push([perThreadAmount, originalSize, &remainAmount, threadSize](int id) {
                                  const Transform transform;
                                  const GlobalTransform globalTransform;
                                  for (int index = originalSize + perThreadAmount * threadSize;
                                       index < originalSize + remainAmount;
                                       index++)
                                  {
                                      auto &entity = GetInstance().m_entities->at(index);
                                      entity.SetComponentData(transform);
                                      entity.SetComponentData(globalTransform);
                                  }
                              })
                              .share());
        for (const auto &i : results)
            i.wait();
    }

    retVal.insert(retVal.end(), manager.m_entities->begin() + originalSize, manager.m_entities->end());
    return retVal;
}

void EntityManager::DeleteEntity(const Entity &entity)
{
    /*
    if (!Application::m_initialized)
    {
        UNIENGINE_ERROR("DeleteEntity: Initialize Engine first!");
        return;
    }
    */
    if (!entity.IsValid())
        return;
    const size_t entityIndex = entity.m_index;
    if (entity != GetInstance().m_entities->at(entityIndex))
    {
        UNIENGINE_ERROR("Entity out of date!");
    }
    // DO NOT CHANGE CODE HERE!
    auto children = GetInstance().m_entityInfos->at(entityIndex).m_children;
    for (const auto &child : children)
    {
        DeleteEntity(child);
    }
    if (GetInstance().m_entityInfos->at(entityIndex).m_parent.m_index != 0)
        RemoveChild(entity, GetInstance().m_entityInfos->at(entityIndex).m_parent);
    DeleteEntityInternal(entity);
    GetInstance().m_currentAttachedWorldEntityStorage->m_parentHierarchyVersion++;
}

std::string EntityManager::GetEntityName(const Entity &entity)
{
    if (!entity.IsValid())
        return "";
    const size_t index = entity.m_index;

    if (entity != GetInstance().m_entities->at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return "";
    }
    return GetInstance().m_entityInfos->at(index).m_name;
}

void EntityManager::SetEntityName(const Entity &entity, const std::string &name)
{
    if (!entity.IsValid())
        return;
    const size_t index = entity.m_index;

    if (entity != GetInstance().m_entities->at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return;
    }
    if (name.length() != 0)
    {
        GetInstance().m_entityInfos->at(index).m_name = name;
        return;
    }
    GetInstance().m_entityInfos->at(index).m_name = "Unnamed";
}

void EntityManager::SetParent(const Entity &entity, const Entity &parent, const bool &recalculateTransform)
{
    if (!entity.IsValid() || !parent.IsValid())
        return;
    // Check self-contain.
    bool contained = false;
    ForEachDescendant(parent, [&](const Entity &iterator) {
        if (!contained && iterator == entity)
            contained = true;
    });
    if (contained)
    {
        UNIENGINE_WARNING("Set parent failed");
        return;
    }
    GetInstance().m_currentAttachedWorldEntityStorage->m_parentHierarchyVersion++;
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    if (GetInstance().m_entityInfos->at(childIndex).m_parent.m_index != 0)
    {
        RemoveChild(entity, GetInstance().m_entities->at(GetInstance().m_entityInfos->at(childIndex).m_parent.m_index));
    }
    if (recalculateTransform)
    {
        const auto childGlobalTransform = entity.GetComponentData<GlobalTransform>();
        const auto parentGlobalTransform = parent.GetComponentData<GlobalTransform>();
        Transform childTransform;
        childTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * childGlobalTransform.m_value;
        entity.SetComponentData(childTransform);
    }
    GetInstance().m_entityInfos->at(childIndex).m_parent = parent;
    GetInstance().m_entityInfos->at(parentIndex).m_children.push_back(entity);
}

Entity EntityManager::GetParent(const Entity &entity)
{
    if (!entity.IsValid())
        return Entity();
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_parent;
}

std::vector<Entity> EntityManager::GetChildren(const Entity &entity)
{
    if (!entity.IsValid())
        return std::vector<Entity>();
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_children;
}

size_t EntityManager::GetChildrenAmount(const Entity &entity)
{
    if (!entity.IsValid())
        return 0;
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_children.size();
}

inline void EntityManager::ForEachChild(const Entity &entity, const std::function<void(Entity child)> &func)
{
    if (!entity.IsValid())
        return;
    auto children = GetInstance().m_entityInfos->at(entity.m_index).m_children;
    for (auto i : children)
    {
        if (!i.IsDeleted())
            func(i);
    }
}

void EntityManager::RemoveChild(const Entity &entity, const Entity &parent)
{
    if (!entity.IsValid() || !parent.IsValid())
        return;
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    if (GetInstance().m_entityInfos->at(childIndex).m_parent.m_index == 0)
    {
        UNIENGINE_ERROR("No child by the parent!");
    }
    GetInstance().m_currentAttachedWorldEntityStorage->m_parentHierarchyVersion++;
    GetInstance().m_entityInfos->at(childIndex).m_parent = Entity();
    const size_t childrenCount = GetInstance().m_entityInfos->at(parentIndex).m_children.size();

    for (int i = 0; i < childrenCount; i++)
    {
        if (GetInstance().m_entityInfos->at(parentIndex).m_children[i].m_index == childIndex)
        {
            GetInstance().m_entityInfos->at(parentIndex).m_children[i] =
                GetInstance().m_entityInfos->at(parentIndex).m_children.back();
            GetInstance().m_entityInfos->at(parentIndex).m_children.pop_back();
            break;
        }
    }
    const auto childGlobalTransform = entity.GetComponentData<GlobalTransform>();
    Transform childTransform;
    childTransform.m_value = childGlobalTransform.m_value;
    entity.SetComponentData(childTransform);
}

size_t EntityManager::GetParentHierarchyVersion()
{
    return GetInstance().m_currentAttachedWorldEntityStorage->m_parentHierarchyVersion;
}

void EntityManager::RemoveComponentData(const Entity &entity, const size_t &typeID)
{
    if (!entity.IsValid())
        return;
    if (typeID == typeid(Transform).hash_code())
    {
        return;
    }
    if (typeID == typeid(GlobalTransform).hash_code())
    {
        return;
    }
    EntityInfo &entityInfo = GetInstance().m_entityInfos->at(entity.m_index);
    EntityArchetypeInfo *archetypeInfo =
        GetInstance().m_entityComponentStorage->at(entityInfo.m_archetypeInfoIndex).m_archetypeInfo;
    if (archetypeInfo->m_componentTypes.size() <= 1)
    {
        UNIENGINE_ERROR("Remove Component Data failed: Entity must have at least 1 data component!");
        return;
    }
#pragma region Create new archetype
    auto *newArchetypeInfo = new EntityArchetypeInfo();
    newArchetypeInfo->m_name = "New archetype";
    newArchetypeInfo->m_entityCount = 0;
    newArchetypeInfo->m_componentTypes = archetypeInfo->m_componentTypes;
    bool found = false;
    for (int i = 0; i < newArchetypeInfo->m_componentTypes.size(); i++)
    {
        if (newArchetypeInfo->m_componentTypes[i].m_typeId == typeID)
        {
            newArchetypeInfo->m_componentTypes.erase(newArchetypeInfo->m_componentTypes.begin() + i);
            found = true;
            break;
        }
    }
    if (!found)
    {
        delete newArchetypeInfo;
        UNIENGINE_ERROR("Failed to remove component data: Component not found");
        return;
    }
#pragma region Sort types and check duplicate
    size_t offset = 0;
    ComponentDataType prev = newArchetypeInfo->m_componentTypes[0];
    for (auto &i : newArchetypeInfo->m_componentTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }

    newArchetypeInfo->m_entitySize =
        newArchetypeInfo->m_componentTypes.back().m_offset + newArchetypeInfo->m_componentTypes.back().m_size;
    newArchetypeInfo->m_chunkCapacity = GetInstance().m_archetypeChunkSize / newArchetypeInfo->m_entitySize;
    auto duplicateIndex = -1;
    for (size_t i = 1; i < GetInstance().m_entityComponentStorage->size(); i++)
    {
        EntityArchetypeInfo *compareInfo = GetInstance().m_entityComponentStorage->at(i).m_archetypeInfo;
        if (newArchetypeInfo->m_chunkCapacity != compareInfo->m_chunkCapacity)
            continue;
        if (newArchetypeInfo->m_entitySize != compareInfo->m_entitySize)
            continue;
        bool typeCheck = true;
        for (const auto &j : newArchetypeInfo->m_componentTypes)
        {
            if (!compareInfo->HasType(j.m_typeId))
                typeCheck = false;
        }
        if (typeCheck)
        {
            duplicateIndex = compareInfo->m_index;
            delete newArchetypeInfo;
            newArchetypeInfo = compareInfo;
            break;
        }
    }
#pragma endregion
    EntityArchetype archetype;
    if (duplicateIndex == -1)
    {
        archetype.m_index = GetInstance().m_entityComponentStorage->size();
        newArchetypeInfo->m_index = archetype.m_index;
        GetInstance().m_entityComponentStorage->push_back(
            EntityComponentDataStorage(newArchetypeInfo, new ComponentDataChunkArray()));
    }
    else
    {
        archetype.m_index = duplicateIndex;
    }
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(archetype);
    // Transfer component data
    for (const auto &type : newArchetypeInfo->m_componentTypes)
    {
        SetComponentData(newEntity, type.m_typeId, type.m_size, GetComponentDataPointer(entity, type.m_typeId));
    }
    // 5. Swap entity.
    EntityInfo &newEntityInfo = GetInstance().m_entityInfos->at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_archetypeInfoIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_archetypeInfoIndex = entityInfo.m_archetypeInfoIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_archetypeInfoIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    GetInstance()
        .m_entityComponentStorage->at(entityInfo.m_archetypeInfoIndex)
        .m_chunkArray->Entities[entityInfo.m_chunkArrayIndex] = entity;
    GetInstance()
        .m_entityComponentStorage->at(newEntityInfo.m_archetypeInfoIndex)
        .m_chunkArray->Entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
    for (size_t i = 0; i < GetInstance().m_entityQueryInfos->size(); i++)
    {
        RefreshEntityQueryInfos(i);
    }
}

void EntityManager::SetComponentData(const Entity &entity, size_t id, size_t size, ComponentDataBase *data)
{
    if (!entity.IsValid())
        return;
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);

    auto *chunkInfo = GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_archetypeInfo;
    const auto chunkIndex = info.m_chunkArrayIndex / chunkInfo->m_chunkCapacity;
    const auto chunkPointer = info.m_chunkArrayIndex % chunkInfo->m_chunkCapacity;
    const auto chunk =
        GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_chunkArray->Chunks[chunkIndex];
    if (id == typeid(Transform).hash_code())
    {
        const auto &type = chunkInfo->m_componentTypes[0];
        chunk.SetData(
            static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size), size, data);
        return;
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        const auto &type = chunkInfo->m_componentTypes[1];
        chunk.SetData(
            static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size), size, data);
        return;
    }
    for (const auto &type : chunkInfo->m_componentTypes)
    {

        if (type.m_typeId == id)
        {
            chunk.SetData(
                static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size),
                size,
                data);
            return;
        }
    }
    UNIENGINE_LOG("ComponentData doesn't exist");
}

ComponentDataBase *EntityManager::GetComponentDataPointer(const Entity &entity, const size_t &id)
{
    if (!entity.IsValid())
        return nullptr;
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);

    EntityArchetypeInfo *chunkInfo =
        GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_archetypeInfo;
    const size_t chunkIndex = info.m_chunkArrayIndex / chunkInfo->m_chunkCapacity;
    const size_t chunkPointer = info.m_chunkArrayIndex % chunkInfo->m_chunkCapacity;
    const ComponentDataChunk chunk =
        GetInstance().m_entityComponentStorage->at(info.m_archetypeInfoIndex).m_chunkArray->Chunks[chunkIndex];
    if (id == typeid(Transform).hash_code())
    {
        const auto &type = chunkInfo->m_componentTypes[0];
        return chunk.GetDataPointer(
            static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size));
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        const auto &type = chunkInfo->m_componentTypes[1];
        return chunk.GetDataPointer(
            static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size));
    }
    for (const auto &type : chunkInfo->m_componentTypes)
    {
        if (type.m_typeId == id)
        {
            return chunk.GetDataPointer(
                static_cast<size_t>(type.m_offset * chunkInfo->m_chunkCapacity + chunkPointer * type.m_size));
        }
    }
    UNIENGINE_LOG("ComponentData doesn't exist");
    return nullptr;
}

EntityArchetype EntityManager::CreateEntityArchetype(
    const std::string &name, const std::vector<ComponentDataType> &types)
{
    auto *info = new EntityArchetypeInfo();
    info->m_name = name;
    info->m_entityCount = 0;
    std::vector<ComponentDataType> actualTypes;
    actualTypes.push_back(Typeof<Transform>());
    actualTypes.push_back(Typeof<GlobalTransform>());
    actualTypes.insert(actualTypes.end(), types.begin(), types.end());
    std::sort(actualTypes.begin() + 2, actualTypes.end(), ComponentTypeComparator);
    size_t offset = 0;
    ComponentDataType prev = actualTypes[0];
    // Erase duplicates
    EraseDuplicates(actualTypes);
    for (auto &i : actualTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    info->m_componentTypes = actualTypes;
    info->m_entitySize = info->m_componentTypes.back().m_offset + info->m_componentTypes.back().m_size;
    info->m_chunkCapacity = GetInstance().m_archetypeChunkSize / info->m_entitySize;
    int duplicateIndex = -1;
    for (size_t i = 1; i < GetInstance().m_entityComponentStorage->size(); i++)
    {
        EntityArchetypeInfo *compareInfo = GetInstance().m_entityComponentStorage->at(i).m_archetypeInfo;
        if (info->m_chunkCapacity != compareInfo->m_chunkCapacity)
            continue;
        if (info->m_entitySize != compareInfo->m_entitySize)
            continue;
        bool typeCheck = true;
        for (const auto &j : info->m_componentTypes)
        {
            if (!compareInfo->HasType(j.m_typeId))
                typeCheck = false;
        }
        if (typeCheck)
        {
            duplicateIndex = compareInfo->m_index;
            delete info;
            info = compareInfo;
            break;
        }
    }
    EntityArchetype retVal;
    if (duplicateIndex == -1)
    {
        retVal.m_index = GetInstance().m_entityComponentStorage->size();
        info->m_index = retVal.m_index;
        GetInstance().m_entityComponentStorage->push_back(
            EntityComponentDataStorage(info, new ComponentDataChunkArray()));
    }
    else
    {
        retVal.m_index = duplicateIndex;
    }
    for (size_t i = 0; i < GetInstance().m_entityQueryInfos->size(); i++)
    {
        RefreshEntityQueryInfos(i);
    }
    return retVal;
}

void EntityManager::SetPrivateComponent(
    const Entity &entity, const std::string &name, const size_t &id, PrivateComponentBase *ptr, const bool &enabled)
{
    if (!ptr || !entity.IsValid())
        return;
    ptr->m_enabled = true;
    bool found = false;
    size_t i = 0;
    for (auto &element : GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements)
    {
        if (id == element.m_typeId)
        {
            found = true;
            element.m_privateComponentData = std::unique_ptr<PrivateComponentBase>(ptr);
            element.ResetOwner(entity);
            element.m_privateComponentData->Init();
        }
        i++;
    }
    if (!found)
    {
        GetInstance().m_entityPrivateComponentStorage->SetPrivateComponent(entity, id);
        GetInstance()
            .m_entityInfos->at(entity.m_index)
            .m_privateComponentElements.emplace_back(name, id, std::unique_ptr<PrivateComponentBase>(ptr), entity);
    }
}

bool EntityManager::IsEntityArchetypeValid(const EntityArchetype &archetype)
{
    return archetype.IsNull() && GetInstance().m_entityComponentStorage->size() > archetype.m_index;
}

void EntityManager::ForEachDescendantHelper(
    const Entity &target, const std::function<void(const Entity &entity)> &func)
{
    func(target);
    ForEachChild(target, [&](Entity child) { ForEachDescendantHelper(child, func); });
}

EntityArchetype EntityManager::GetDefaultEntityArchetype()
{
    return GetInstance().m_basicArchetype;
}

EntityArchetypeInfo EntityManager::GetArchetypeInfo(const EntityArchetype &entityArchetype)
{
    return *GetInstance().m_entityComponentStorage->at(entityArchetype.m_index).m_archetypeInfo;
}

Entity EntityManager::GetRoot(const Entity &entity)
{
    Entity retVal = entity;
    auto parent = GetParent(retVal);
    while (!parent.IsNull())
    {
        retVal = parent;
        parent = GetParent(retVal);
    }
    return retVal;
}

Entity EntityManager::GetEntity(const size_t &index)
{
    if (index > 0 && index < GetInstance().m_entities->size())
        return GetInstance().m_entities->at(index);
    return Entity();
}

void EntityManager::RemovePrivateComponent(const Entity &entity, const size_t &typeId)
{
    if (!entity.IsValid())
        return;
    for (auto i = 0; i < GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements.size(); i++)
    {
        if (GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements[i].m_typeId == typeId)
        {
            GetInstance()
                .m_entityInfos->at(entity.m_index)
                .m_privateComponentElements.erase(
                    GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements.begin() + i);
        }
    }
    GetInstance().m_entityPrivateComponentStorage->RemovePrivateComponent(entity, typeId);
}

EntityArchetype EntityManager::GetEntityArchetype(const Entity &entity)
{
    EntityArchetype retVal = EntityArchetype();
    if (!entity.IsValid())
        return retVal;
    EntityInfo &info = GetInstance().m_entityInfos->at(entity.m_index);
    retVal.m_index = info.m_archetypeInfoIndex;
    return retVal;
}

void EntityManager::SetEnable(const Entity &entity, const bool &value)
{
    if (!entity.IsValid())
        return;
    if (GetInstance().m_entityInfos->at(entity.m_index).m_enabled != value)
    {
        for (auto &i : GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements)
        {
            if (value)
            {
                i.m_privateComponentData->OnEntityEnable();
            }
            else
            {
                i.m_privateComponentData->OnEntityDisable();
            }
        }
    }
    GetInstance().m_entityInfos->at(entity.m_index).m_enabled = value;

    for (const auto &i : GetInstance().m_entityInfos->at(entity.m_index).m_children)
    {
        SetEnable(i, value);
    }
}

void EntityManager::SetStatic(const Entity &entity, const bool &value)
{
    if (!entity.IsValid())
        return;
    ForEachDescendant(
        entity, [=](const Entity iterator) { GetInstance().m_entityInfos->at(iterator.m_index).m_static = value; });
}

void EntityManager::SetEnableSingle(const Entity &entity, const bool &value)
{
    if (!entity.IsValid())
        return;
    if (GetInstance().m_entityInfos->at(entity.m_index).m_enabled != value)
    {
        for (auto &i : GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements)
        {
            if (value)
            {
                i.m_privateComponentData->OnEntityEnable();
            }
            else
            {
                i.m_privateComponentData->OnEntityDisable();
            }
        }
    }
    GetInstance().m_entityInfos->at(entity.m_index).m_enabled = value;
}

bool EntityManager::IsEntityEnabled(const Entity &entity)
{
    if (!entity.IsValid())
        return false;
    return GetInstance().m_entityInfos->at(entity.m_index).m_enabled;
}

bool EntityManager::IsEntityStatic(const Entity &entity)
{
    if (!entity.IsValid())
        return false;
    return GetInstance().m_entityInfos->at(entity.m_index).m_static;
}

bool EntityManager::IsEntityDeleted(const size_t &index)
{
    return GetInstance().m_entities->at(index).m_version == 0;
}

bool EntityManager::IsEntityValid(const Entity &entity)
{
    return GetInstance().m_entities->at(entity.m_index).m_version == entity.m_version;
}

EntityQuery EntityManager::CreateEntityQuery()
{
    EntityQuery retVal;
    retVal.m_index = GetInstance().m_entityQueries->size();
    GetInstance().m_entityQueries->push_back(retVal);

    const EntityQueryInfo info;
    GetInstance().m_entityQueryInfos->push_back(info);
    RefreshEntityQueryInfos(retVal.m_index);

    return retVal;
}

std::vector<EntityComponentDataStorage> EntityManager::UnsafeGetComponentDataStorage(const EntityQuery &entityQuery)
{
    if (entityQuery.IsNull())
        return std::vector<EntityComponentDataStorage>();
    const size_t index = entityQuery.m_index;
    if (GetInstance().m_entityQueries->at(index) != entityQuery)
    {
        UNIENGINE_ERROR("EntityQuery out of date!");
        return std::vector<EntityComponentDataStorage>();
    }
    return GetInstance().m_entityQueryInfos->at(index).m_queriedStorage;
}

void EntityManager::ForAllEntities(const std::function<void(int i, Entity entity)> &func)
{
    for (int index = 0; index < GetInstance().m_entities->size(); index++)
    {
        if (GetInstance().m_entities->at(index).m_version != 0)
        {
            func(index, GetInstance().m_entities->at(index));
        }
    }
}

std::string EntityManager::GetEntityArchetypeName(const EntityArchetype &entityArchetype)
{
    return GetInstance()
        .m_currentAttachedWorldEntityStorage->m_entityComponentStorage[entityArchetype.m_index]
        .m_archetypeInfo->m_name;
}

void EntityManager::GetEntityArray(const EntityQuery &entityQuery, std::vector<Entity> &container)
{
    if (entityQuery.IsNull())
        return;
    const size_t index = entityQuery.m_index;
    if (GetInstance().m_entityQueries->at(index) != entityQuery)
    {
        UNIENGINE_ERROR("EntityQuery out of date!");
        return;
    }
    for (auto i : GetInstance().m_entityQueryInfos->at(index).m_queriedStorage)
    {
        GetEntityStorage(i, container);
    }
}

void EntityQuery::ToEntityArray(std::vector<Entity> &container) const
{
    EntityManager::GetEntityArray(*this, container);
}

EntityComponentDataStorage::EntityComponentDataStorage(EntityArchetypeInfo *info, ComponentDataChunkArray *array)
{
    m_archetypeInfo = info;
    m_chunkArray = array;
}

size_t EntityManager::GetEntityAmount(EntityQuery entityQuery)
{
    if (entityQuery.IsNull())
        return 0;
    const size_t index = entityQuery.m_index;
    if (GetInstance().m_entityQueries->at(index) != entityQuery)
    {
        UNIENGINE_ERROR("EntityQuery out of date!");
        return 0;
    }
    size_t retVal = 0;
    for (auto i : GetInstance().m_entityQueryInfos->at(index).m_queriedStorage)
    {
        retVal += i.m_archetypeInfo->m_entityAliveCount;
    }
    return retVal;
}
void EntityManager::SetEntityArchetypeName(const EntityArchetype &entityArchetype, const std::string& name)
{
    GetInstance()
        .m_currentAttachedWorldEntityStorage->m_entityComponentStorage[entityArchetype.m_index]
        .m_archetypeInfo->m_name = name;
}
std::vector<Entity> EntityManager::GetDescendants(const Entity &entity)
{
    std::vector<Entity> retVal;
    GetDescendantsHelper(entity, retVal);
    return retVal;
}
void EntityManager::GetDescendantsHelper(const Entity &target, std::vector<Entity>& results)
{
    auto& children = GetInstance().m_entityInfos->at(target.m_index).m_children;
    if(!children.empty())results.insert(results.end(), children.begin(), children.end());
    for(const auto& i : children)
        GetDescendantsHelper(i, results);
}

size_t EntityQuery::GetEntityAmount() const
{
    return EntityManager::GetEntityAmount(*this);
}
#pragma endregion
