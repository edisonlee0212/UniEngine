#include <Core/Debug.hpp>
#include <EntityManager.hpp>
#include <PhysicsManager.hpp>
#include <Scene.hpp>
using namespace UniEngine;

#pragma region EntityManager

void EntityManager::UnsafeForEachDataComponent(
    const Entity &entity, const std::function<void(const DataComponentType &type, void *data)> &func)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    EntityInfo &entityInfo = entityManager.m_entityInfos->at(entity.m_index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    const size_t chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const size_t chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    const ComponentDataChunk &chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    for (const auto &i : dataComponentStorage.m_dataComponentTypes)
    {
        func(
            i,
            static_cast<void *>(
                static_cast<char *>(chunk.m_data) + i.m_offset * dataComponentStorage.m_chunkCapacity +
                chunkPointer * i.m_size));
    }
}

void EntityManager::ForEachPrivateComponent(
    const Entity &entity, const std::function<void(PrivateComponentElement &data)> &func)
{
    assert(entity.IsValid());
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
    const std::function<void(int i, const std::string &name, const DataComponentStorage &storage)> &func)
{
    auto &entityManager = GetInstance();
    auto &archetypeInfos = entityManager.m_entityArchetypeInfos;
    for (int i = 0; i < archetypeInfos.size(); i++)
    {
        func(
            i,
            archetypeInfos[i].m_name,
            entityManager.m_entityDataComponentStorage->at(archetypeInfos[i].m_dataComponentStorageIndex));
    }
}

size_t EntityManager::GetArchetypeChunkSize()
{
    return GetInstance().m_archetypeChunkSize;
}

void EntityManager::DeleteEntityInternal(const Entity &entity)
{
    auto &entityManager = GetInstance();
    EntityInfo &entityInfo = entityManager.m_entityInfos->at(entity.m_index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    Entity actualEntity = entityManager.m_entities->at(entity.m_index);
    if (actualEntity == entity)
    {
        entityManager.m_entityPrivateComponentStorage->DeleteEntity(actualEntity);
        entityInfo.m_version = actualEntity.m_version + 1;
        entityInfo.m_static = false;
        entityInfo.m_enabled = true;
        while (!entityInfo.m_privateComponentElements.empty())
        {
            entityInfo.m_privateComponentElements.back().m_privateComponentData->OnDestroy();
            delete entityInfo.m_privateComponentElements.back().m_privateComponentData;
            entityInfo.m_privateComponentElements.pop_back();
        }
        // Set to version 0, marks it as deleted.
        actualEntity.m_version = 0;
        dataComponentStorage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = actualEntity;
        const auto originalIndex = entityInfo.m_chunkArrayIndex;
        if (entityInfo.m_chunkArrayIndex != dataComponentStorage.m_entityAliveCount - 1)
        {
            const auto swappedIndex = SwapEntity(
                dataComponentStorage, entityInfo.m_chunkArrayIndex, dataComponentStorage.m_entityAliveCount - 1);
            entityInfo.m_chunkArrayIndex = dataComponentStorage.m_entityAliveCount - 1;
            GetInstance().m_entityInfos->at(swappedIndex).m_chunkArrayIndex = originalIndex;
        }
        dataComponentStorage.m_entityAliveCount--;
    }
    else
    {
        UNIENGINE_ERROR("Entity already deleted!");
        return;
    }
    GetInstance().m_entities->at(entity.m_index) = actualEntity;
}

void EntityManager::RefreshEntityArchetypeInfo(const size_t &index)
{
    auto &entityManager = GetInstance();
    auto &archetypeInfo = entityManager.m_entityArchetypeInfos.at(index);
    int targetIndex = 0;
    for (const auto &i : *entityManager.m_entityDataComponentStorage)
    {
        if (i.m_dataComponentTypes.size() != archetypeInfo.m_dataComponentTypes.size())
            continue;
        bool check = true;
        for (int j = 0; j < i.m_dataComponentTypes.size(); j++)
        {
            if (i.m_dataComponentTypes[j].m_name != archetypeInfo.m_dataComponentTypes[j].m_name)
            {
                check = false;
                break;
            }
        }
        if (check)
        {
            archetypeInfo.m_dataComponentStorageIndex = targetIndex;
            break;
        }
        targetIndex++;
    }
}

void EntityManager::RefreshEntityQueryInfo(const size_t &index)
{
    auto &entityManager = GetInstance();
    auto &queryInfos = entityManager.m_entityQueryInfos.at(index);
    auto &entityComponentStorage = GetInstance().m_entityDataComponentStorage;
    auto &queriedStorage = entityManager.m_entityQueryInfos.at(index).m_queriedStorage;
    queriedStorage.clear();
    // Select storage with every contained.
    if (!queryInfos.m_allDataComponentTypes.empty())
    {
        for (int i = 0; i < entityComponentStorage->size(); i++)
        {
            auto &dataStorage = entityComponentStorage->at(i);
            bool check = true;
            for (const auto &type : queryInfos.m_allDataComponentTypes)
            {
                if (!dataStorage.HasType(type.m_typeId))
                    check = false;
            }
            if (check)
                queriedStorage.push_back(&dataStorage);
        }
    }
    else
    {
        for (int i = 0; i < entityComponentStorage->size(); i++)
        {
            auto &dataStorage = entityComponentStorage->at(i);
            queriedStorage.push_back(&dataStorage);
        }
    }
    // Erase with any
    if (!queryInfos.m_anyDataComponentTypes.empty())
    {
        for (int i = 0; i < queriedStorage.size(); i++)
        {
            bool contain = false;
            for (const auto &type : queryInfos.m_anyDataComponentTypes)
            {
                if (queriedStorage.at(i)->HasType(type.m_typeId))
                    contain = true;
                if (contain)
                    break;
            }
            if (!contain)
            {
                queriedStorage.erase(queriedStorage.begin() + i);
                i--;
            }
        }
    }
    // Erase with none
    if (!queryInfos.m_noneDataComponentTypes.empty())
    {
        for (int i = 0; i < queriedStorage.size(); i++)
        {
            bool contain = false;
            for (const auto &type : queryInfos.m_noneDataComponentTypes)
            {
                if (queriedStorage.at(i)->HasType(type.m_typeId))
                    contain = true;
                if (contain)
                    break;
            }
            if (contain)
            {
                queriedStorage.erase(queriedStorage.begin() + i);
                i--;
            }
        }
    }
}

void EntityManager::EraseDuplicates(std::vector<DataComponentType> &types)
{
    std::vector<DataComponentType> copy;
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

void EntityManager::GetEntityStorage(const DataComponentStorage &storage, std::vector<Entity> &container)
{
    const size_t amount = storage.m_entityAliveCount;
    if (amount == 0)
        return;
    container.resize(container.size() + amount);
    const size_t capacity = storage.m_chunkCapacity;
    memcpy(&container.at(container.size() - amount), storage.m_chunkArray.m_entities.data(), amount * sizeof(Entity));
}

size_t EntityManager::SwapEntity(DataComponentStorage &storage, size_t index1, size_t index2)
{
    if (index1 == index2)
        return -1;
    const size_t retVal = storage.m_chunkArray.m_entities[index2].m_index;
    const auto other = storage.m_chunkArray.m_entities[index2];
    storage.m_chunkArray.m_entities[index2] = storage.m_chunkArray.m_entities[index1];
    storage.m_chunkArray.m_entities[index1] = other;
    const auto capacity = storage.m_chunkCapacity;
    const auto chunkIndex1 = index1 / capacity;
    const auto chunkIndex2 = index2 / capacity;
    const auto chunkPointer1 = index1 % capacity;
    const auto chunkPointer2 = index2 % capacity;
    for (const auto &i : storage.m_dataComponentTypes)
    {
        void *temp = static_cast<void *>(malloc(i.m_size));
        void *d1 = static_cast<void *>(
            static_cast<char *>(storage.m_chunkArray.m_chunks[chunkIndex1].m_data) + i.m_offset * capacity +
            i.m_size * chunkPointer1);

        void *d2 = static_cast<void *>(
            static_cast<char *>(storage.m_chunkArray.m_chunks[chunkIndex2].m_data) + i.m_offset * capacity +
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
    auto &entityManager = GetInstance();
    entityManager.m_scene = nullptr;
    entityManager.m_currentAttachedWorldEntityStorage = nullptr;
    entityManager.m_entities = nullptr;
    entityManager.m_entityInfos = nullptr;
    entityManager.m_entityDataComponentStorage = nullptr;
    entityManager.m_entityPrivateComponentStorage = nullptr;

    for (auto &i : entityManager.m_entityArchetypeInfos)
    {
        i.m_dataComponentStorageIndex = 0;
    }
    for (auto &i : entityManager.m_entityQueryInfos)
    {
        i.m_queriedStorage.clear();
    }
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

void EntityManager::Attach(const std::shared_ptr<Scene> &scene)
{
    if (scene == nullptr)
        return;
    auto &entityManager = GetInstance();
    auto &targetStorage = scene->m_sceneDataStorage;
    entityManager.m_currentAttachedWorldEntityStorage = &targetStorage;
    entityManager.m_entities = &targetStorage.m_entities;
    entityManager.m_entityInfos = &targetStorage.m_entityInfos;
    entityManager.m_entityDataComponentStorage = &targetStorage.m_dataComponentStorages;
    entityManager.m_entityPrivateComponentStorage = &targetStorage.m_entityPrivateComponentStorage;
    entityManager.m_scene = scene;

    GetOrCreateSystem<PhysicsSystem>(SystemGroup::SimulationSystemGroup);

    for (auto &i : entityManager.m_entityArchetypeInfos)
    {
        i.m_dataComponentStorageIndex = 0;
    }
    for (auto &i : entityManager.m_entityQueryInfos)
    {
        i.m_queriedStorage.clear();
    }

    RefreshAllEntityArchetypeInfos();
    RefreshAllEntityQueryInfos();
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
    auto &entityManager = GetInstance();
    assert(archetype.IsValid());
    Entity retVal;


    EntityArchetypeInfo &archetypeInfo = entityManager.m_entityArchetypeInfos[archetype.m_index];
    if (archetypeInfo.m_dataComponentStorageIndex == 0)
    {
        archetypeInfo.m_dataComponentStorageIndex = entityManager.m_entityDataComponentStorage->size();
        entityManager.m_entityDataComponentStorage->emplace_back(archetypeInfo);
        for (size_t i = 0; i < GetInstance().m_entityQueryInfos.size(); i++)
        {
            RefreshEntityQueryInfo(i);
        }
    }
    DataComponentStorage &storage =
        entityManager.m_entityDataComponentStorage->at(archetypeInfo.m_dataComponentStorageIndex);
    if (storage.m_entityCount == storage.m_entityAliveCount)
    {
        const size_t chunkIndex = storage.m_entityCount / storage.m_chunkCapacity + 1;
        if (storage.m_chunkArray.m_chunks.size() <= chunkIndex)
        {
            // Allocate new chunk;
            ComponentDataChunk chunk;
            chunk.m_data = static_cast<void *>(calloc(1, GetInstance().m_archetypeChunkSize));
            storage.m_chunkArray.m_chunks.push_back(chunk);
        }
        retVal.m_index = GetInstance().m_entities->size();
        // If the version is 0 in chunk means it's deleted.
        retVal.m_version = 1;
        EntityInfo entityInfo;
        entityInfo.m_name = name;
        entityInfo.m_dataComponentStorageIndex = archetypeInfo.m_dataComponentStorageIndex;
        entityInfo.m_chunkArrayIndex = storage.m_entityCount;
        storage.m_chunkArray.m_entities.push_back(retVal);
        GetInstance().m_entityInfos->push_back(std::move(entityInfo));
        GetInstance().m_entities->push_back(retVal);
        storage.m_entityCount++;
        storage.m_entityAliveCount++;
    }
    else
    {
        retVal = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityInfo &entityInfo = GetInstance().m_entityInfos->at(retVal.m_index);
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        retVal.m_version = entityInfo.m_version;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = retVal;
        GetInstance().m_entities->at(retVal.m_index) = retVal;
        storage.m_entityAliveCount++;
        // Reset all component data
        const auto chunkIndex = entityInfo.m_chunkArrayIndex / storage.m_chunkCapacity;
        const auto chunkPointer = entityInfo.m_chunkArrayIndex % storage.m_chunkCapacity;
        const auto chunk = GetInstance()
                               .m_entityDataComponentStorage->at(archetypeInfo.m_dataComponentStorageIndex)
                               .m_chunkArray.m_chunks[chunkIndex];
        for (const auto &i : storage.m_dataComponentTypes)
        {
            const auto offset = i.m_offset * storage.m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
    }
    retVal.SetDataComponent(Transform());
    retVal.SetDataComponent(GlobalTransform());
    retVal.SetDataComponent(GlobalTransformUpdateFlag());
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
    assert(archetype.IsValid());
    std::vector<Entity> retVal;
    auto &entityManager = GetInstance();
    EntityArchetypeInfo &archetypeInfo = entityManager.m_entityArchetypeInfos[archetype.m_index];
    if (archetypeInfo.m_dataComponentStorageIndex == 0)
    {
        archetypeInfo.m_dataComponentStorageIndex = entityManager.m_entityDataComponentStorage->size();
        entityManager.m_entityDataComponentStorage->emplace_back(archetypeInfo);
        RefreshAllEntityQueryInfos();
    }
    DataComponentStorage &storage =
        entityManager.m_entityDataComponentStorage->at(archetypeInfo.m_dataComponentStorageIndex);
    auto remainAmount = amount;
    const Transform transform;
    const GlobalTransform globalTransform;
    const GlobalTransformUpdateFlag transformStatus;
    while (remainAmount > 0 && storage.m_entityAliveCount != storage.m_entityCount)
    {
        remainAmount--;
        Entity entity = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityInfo &entityInfo = GetInstance().m_entityInfos->at(entity.m_index);
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        entity.m_version = entityInfo.m_version;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
        GetInstance().m_entities->at(entity.m_index) = entity;
        storage.m_entityAliveCount++;
        // Reset all component data
        const size_t chunkIndex = entityInfo.m_chunkArrayIndex / storage.m_chunkCapacity;
        const size_t chunkPointer = entityInfo.m_chunkArrayIndex % storage.m_chunkCapacity;
        const ComponentDataChunk &chunk =
            GetInstance()
                .m_entityDataComponentStorage->at(archetypeInfo.m_dataComponentStorageIndex)
                .m_chunkArray.m_chunks[chunkIndex];
        for (const auto &i : storage.m_dataComponentTypes)
        {
            const size_t offset = i.m_offset * storage.m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
        retVal.push_back(entity);
        entity.SetDataComponent(transform);
        entity.SetDataComponent(globalTransform);
        entity.SetDataComponent(GlobalTransformUpdateFlag());
    }
    if (remainAmount == 0)
        return retVal;
    storage.m_entityCount += remainAmount;
    storage.m_entityAliveCount += remainAmount;
    const size_t chunkIndex = storage.m_entityCount / storage.m_chunkCapacity + 1;
    while (storage.m_chunkArray.m_chunks.size() <= chunkIndex)
    {
        // Allocate new chunk;
        ComponentDataChunk chunk;
        chunk.m_data = static_cast<void *>(calloc(1, GetInstance().m_archetypeChunkSize));
        storage.m_chunkArray.m_chunks.push_back(chunk);
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
        entityInfo.m_dataComponentStorageIndex = archetypeInfo.m_dataComponentStorageIndex;
        entityInfo.m_chunkArrayIndex = storage.m_entityAliveCount - remainAmount + i;
    }

    storage.m_chunkArray.m_entities.insert(
        storage.m_chunkArray.m_entities.end(),
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
                                          entity.SetDataComponent(transform);
                                          entity.SetDataComponent(globalTransform);
                                          entity.SetDataComponent(GlobalTransformUpdateFlag());
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
                                      entity.SetDataComponent(transform);
                                      entity.SetDataComponent(globalTransform);
                                      entity.SetDataComponent(GlobalTransformUpdateFlag());
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
    return CreateEntities(GetInstance().m_basicArchetype, amount, name);
}

void EntityManager::DeleteEntity(const Entity &entity)
{
    assert(entity.IsValid());
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
}

std::string EntityManager::GetEntityName(const Entity &entity)
{
    assert(entity.IsValid());
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
    assert(entity.IsValid());
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
    assert(entity.IsValid() && parent.IsValid());
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    auto& entityManager = GetInstance();
    auto& parentEntityInfo = entityManager.m_entityInfos->at(parentIndex);
    for(const auto& i : parentEntityInfo.m_children){
        if(i == entity) return;
    }
    auto& childEntityInfo = entityManager.m_entityInfos->at(childIndex);
    if (!childEntityInfo.m_parent.IsNull())
    {
        RemoveChild(entity, childEntityInfo.m_parent);
    }
    if (recalculateTransform)
    {
        const auto childGlobalTransform = entity.GetDataComponent<GlobalTransform>();
        const auto parentGlobalTransform = parent.GetDataComponent<GlobalTransform>();
        Transform childTransform;
        childTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * childGlobalTransform.m_value;
        entity.SetDataComponent(childTransform);
    }
    childEntityInfo.m_parent = parent;
    parentEntityInfo.m_children.push_back(entity);
}

Entity EntityManager::GetParent(const Entity &entity)
{
    assert(entity.IsValid());
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_parent;
}

std::vector<Entity> EntityManager::GetChildren(const Entity &entity)
{
    assert(entity.IsValid());
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_children;
}

size_t EntityManager::GetChildrenAmount(const Entity &entity)
{
    assert(entity.IsValid());
    const size_t entityIndex = entity.m_index;
    return GetInstance().m_entityInfos->at(entityIndex).m_children.size();
}

inline void EntityManager::ForEachChild(const Entity &entity, const std::function<void(Entity child)> &func)
{
    assert(entity.IsValid());
    auto children = GetInstance().m_entityInfos->at(entity.m_index).m_children;
    for (auto i : children)
    {
        if (!i.IsDeleted())
            func(i);
    }
}

void EntityManager::RemoveChild(const Entity &entity, const Entity &parent)
{
    assert(entity.IsValid() && parent.IsValid());
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    if (GetInstance().m_entityInfos->at(childIndex).m_parent.m_index == 0)
    {
        UNIENGINE_ERROR("No child by the parent!");
    }
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
    const auto childGlobalTransform = entity.GetDataComponent<GlobalTransform>();
    Transform childTransform;
    childTransform.m_value = childGlobalTransform.m_value;
    entity.SetDataComponent(childTransform);
}

void EntityManager::RemoveDataComponent(const Entity &entity, const size_t &typeID)
{
    assert(entity.IsValid());
    if (typeID == typeid(Transform).hash_code() || typeID == typeid(GlobalTransform).hash_code() ||
        typeID == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return;
    }
    auto &entityManager = GetInstance();
    EntityInfo &entityInfo = entityManager.m_entityInfos->at(entity.m_index);
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    if (dataComponentStorage.m_dataComponentTypes.size() <= 3)
    {
        UNIENGINE_ERROR("Remove Component Data failed: Entity must have at least 1 data component besides 3 basic data "
                        "components!");
        return;
    }
#pragma region Create new archetype
    EntityArchetypeInfo newArchetypeInfo;
    newArchetypeInfo.m_name = "New archetype";
    newArchetypeInfo.m_dataComponentTypes = dataComponentStorage.m_dataComponentTypes;
    bool found = false;
    for (int i = 0; i < newArchetypeInfo.m_dataComponentTypes.size(); i++)
    {
        if (newArchetypeInfo.m_dataComponentTypes[i].m_typeId == typeID)
        {
            newArchetypeInfo.m_dataComponentTypes.erase(newArchetypeInfo.m_dataComponentTypes.begin() + i);
            found = true;
            break;
        }
    }
    if (!found)
    {
        UNIENGINE_ERROR("Failed to remove component data: Component not found");
        return;
    }
    size_t offset = 0;
    DataComponentType prev = newArchetypeInfo.m_dataComponentTypes[0];
    for (auto &i : newArchetypeInfo.m_dataComponentTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    newArchetypeInfo.m_entitySize =
        newArchetypeInfo.m_dataComponentTypes.back().m_offset + newArchetypeInfo.m_dataComponentTypes.back().m_size;
    newArchetypeInfo.m_chunkCapacity = GetInstance().m_archetypeChunkSize / newArchetypeInfo.m_entitySize;
    auto archetype = CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(archetype);
    // Transfer component data
    for (const auto &type : newArchetypeInfo.m_dataComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    // 5. Swap entity.
    EntityInfo &newEntityInfo = GetInstance().m_entityInfos->at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = entityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    entityManager.m_entityDataComponentStorage->at(entityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
    entityManager.m_entityDataComponentStorage->at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
}

void EntityManager::SetDataComponent(const unsigned &entityIndex, size_t id, size_t size, IDataComponent *data)
{
    auto &entityManager = GetInstance();
    auto &entityInfo = entityManager.m_entityInfos->at(entityIndex);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    const auto chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const auto chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    const auto chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    if (id == typeid(Transform).hash_code())
    {
        chunk.SetData(static_cast<size_t>(chunkPointer * sizeof(Transform)), sizeof(Transform), data);
    }
    else if (id == typeid(GlobalTransform).hash_code())
    {
        chunk.SetData(
            static_cast<size_t>(
                sizeof(Transform) * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(GlobalTransform)),
            sizeof(GlobalTransform),
            data);
        static_cast<GlobalTransformUpdateFlag *>(
            chunk.GetDataPointer(static_cast<size_t>(
                (sizeof(Transform) + sizeof(GlobalTransform)) * dataComponentStorage.m_chunkCapacity +
                chunkPointer * sizeof(GlobalTransformUpdateFlag))))
            ->m_value = true;
    }
    else if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        chunk.SetData(
            static_cast<size_t>(
                (sizeof(Transform) + sizeof(GlobalTransform)) * dataComponentStorage.m_chunkCapacity +
                chunkPointer * sizeof(GlobalTransformUpdateFlag)),
            sizeof(GlobalTransformUpdateFlag),
            data);
    }
    else
    {
        for (const auto &type : dataComponentStorage.m_dataComponentTypes)
        {
            if (type.m_typeId == id)
            {
                chunk.SetData(
                    static_cast<size_t>(type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * type.m_size),
                    size,
                    data);
                return;
            }
        }
        UNIENGINE_LOG("ComponentData doesn't exist");
    }
}

IDataComponent *EntityManager::GetDataComponentPointer(const Entity &entity, const size_t &id)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    EntityInfo &entityInfo = entityManager.m_entityInfos->at(entity.m_index);
    auto &dataComponentStorage = (*entityManager.m_entityDataComponentStorage)[entityInfo.m_dataComponentStorageIndex];
    const auto chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
    const auto chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
    const auto chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];
    if (id == typeid(Transform).hash_code())
    {
        return chunk.GetDataPointer(static_cast<size_t>(chunkPointer * sizeof(Transform)));
    }
    if (id == typeid(GlobalTransform).hash_code())
    {
        return chunk.GetDataPointer(static_cast<size_t>(
            sizeof(Transform) * dataComponentStorage.m_chunkCapacity + chunkPointer * sizeof(GlobalTransform)));
    }
    if (id == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return chunk.GetDataPointer(static_cast<size_t>(
            (sizeof(Transform) + sizeof(GlobalTransform)) * dataComponentStorage.m_chunkCapacity +
            chunkPointer * sizeof(GlobalTransformUpdateFlag)));
    }
    for (const auto &type : dataComponentStorage.m_dataComponentTypes)
    {
        if (type.m_typeId == id)
        {
            return chunk.GetDataPointer(
                static_cast<size_t>(type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * type.m_size));
        }
    }
    UNIENGINE_LOG("ComponentData doesn't exist");
    return nullptr;
}

EntityArchetype EntityManager::CreateEntityArchetype(
    const std::string &name, const std::vector<DataComponentType> &types)
{
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
    EraseDuplicates(actualTypes);
    for (auto &i : actualTypes)
    {
        i.m_offset = offset;
        offset += i.m_size;
    }
    entityArchetypeInfo.m_dataComponentTypes = actualTypes;
    entityArchetypeInfo.m_entitySize =
        entityArchetypeInfo.m_dataComponentTypes.back().m_offset + entityArchetypeInfo.m_dataComponentTypes.back().m_size;
    entityArchetypeInfo.m_chunkCapacity = GetInstance().m_archetypeChunkSize / entityArchetypeInfo.m_entitySize;
    return CreateEntityArchetypeHelper(entityArchetypeInfo);
}

void EntityManager::SetPrivateComponent(
    const Entity &entity, const size_t &id, IPrivateComponent *ptr, const bool &enabled)
{
    if (!ptr || !entity.IsValid())
        return;
    ptr->m_enabled = true;
    bool found = false;
    size_t i = 0;
    auto &elements = GetInstance().m_entityInfos->at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (id == element.m_typeId)
        {
            found = true;
            if (element.m_privateComponentData)
            {
                element.m_privateComponentData->OnDestroy();
                delete element.m_privateComponentData;
            }
            element.m_privateComponentData = ptr;
            element.ResetOwner(entity);
            element.m_privateComponentData->OnCreate();
        }
        i++;
    }
    if (!found)
    {
        GetInstance().m_entityPrivateComponentStorage->SetPrivateComponent(entity, id);
        elements.emplace_back(id, ptr, entity);
    }
}

void EntityManager::ForEachDescendantHelper(const Entity &target, const std::function<void(const Entity &entity)> &func)
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
    return GetInstance().m_entityArchetypeInfos[entityArchetype.m_index];
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

void EntityManager::RemovePrivateComponent(const Entity &entity, size_t typeId)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    auto &privateComponentElements = entityManager.m_entityInfos->at(entity.m_index).m_privateComponentElements;
    for (auto i = 0; i < privateComponentElements.size(); i++)
    {
        if (privateComponentElements[i].m_typeId == typeId)
        {
            privateComponentElements[i].m_privateComponentData->OnDestroy();
            delete privateComponentElements[i].m_privateComponentData;
            privateComponentElements.erase(privateComponentElements.begin() + i);
            break;
        }
    }
    entityManager.m_entityPrivateComponentStorage->RemovePrivateComponent(entity, typeId);
}


void EntityManager::SetEnable(const Entity &entity, const bool &value)
{
    assert(entity.IsValid());
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
    assert(entity.IsValid());
    ForEachDescendant(
        entity, [=](const Entity iterator) { GetInstance().m_entityInfos->at(iterator.m_index).m_static = value; });
}

void EntityManager::SetEnableSingle(const Entity &entity, const bool &value)
{
    assert(entity.IsValid());
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

EntityQuery EntityManager::CreateEntityQuery()
{
    EntityQuery retVal;
    retVal.m_index = GetInstance().m_entityQueryInfos.size();
    const EntityQueryInfo info;
    GetInstance().m_entityQueryInfos.push_back(info);
    RefreshEntityQueryInfo(retVal.m_index);
    return retVal;
}

std::vector<DataComponentStorage *> EntityManager::UnsafeGetDataComponentStorage(const EntityQuery &entityQuery)
{
    assert(entityQuery.IsValid());
    return GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage;
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
    return GetInstance().m_entityArchetypeInfos[entityArchetype.m_index].m_name;
}

void EntityManager::GetEntityArray(const EntityQuery &entityQuery, std::vector<Entity> &container)
{
    assert(entityQuery.IsValid());
    for (const auto *i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        GetEntityStorage(*i, container);
    }
}

void EntityQuery::ToEntityArray(std::vector<Entity> &container) const
{
    EntityManager::GetEntityArray(*this, container);
}

size_t EntityManager::GetEntityAmount(EntityQuery entityQuery)
{
    assert(entityQuery.IsValid());
    size_t retVal = 0;
    for (auto i : GetInstance().m_entityQueryInfos[entityQuery.m_index].m_queriedStorage)
    {
        retVal += i->m_entityAliveCount;
    }
    return retVal;
}
void EntityManager::SetEntityArchetypeName(const EntityArchetype &entityArchetype, const std::string &name)
{
    GetInstance().m_entityArchetypeInfos[entityArchetype.m_index].m_name = name;
}
std::vector<Entity> EntityManager::GetDescendants(const Entity &entity)
{
    std::vector<Entity> retVal;
    GetDescendantsHelper(entity, retVal);
    return retVal;
}
void EntityManager::GetDescendantsHelper(const Entity &target, std::vector<Entity> &results)
{
    auto &children = GetInstance().m_entityInfos->at(target.m_index).m_children;
    if (!children.empty())
        results.insert(results.end(), children.begin(), children.end());
    for (const auto &i : children)
        GetDescendantsHelper(i, results);
}
template <typename T> const std::vector<Entity> EntityManager::GetPrivateComponentOwnersList()
{
    return GetInstance().m_entityPrivateComponentStorage->GetOwnersList<T>();
}
void EntityManager::Init()
{
    auto &entityManager = GetInstance();
    auto scene = std::make_shared<Scene>();
    EntityManager::Attach(scene);

    entityManager.m_entityArchetypeInfos.emplace_back();
    entityManager.m_entityQueryInfos.emplace_back();

    GetInstance().m_basicArchetype =
        CreateEntityArchetype("Basic", Transform(), GlobalTransform(), GlobalTransformUpdateFlag());
}
std::shared_ptr<Scene> EntityManager::GetCurrentScene()
{
    return GetInstance().m_scene;
}
EntityArchetype EntityManager::CreateEntityArchetypeHelper(const EntityArchetypeInfo &info)
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
void EntityManager::RefreshAllEntityQueryInfos()
{
    for (size_t i = 1; i < GetInstance().m_entityQueryInfos.size(); i++)
    {
        RefreshEntityQueryInfo(i);
    }
}

void EntityManager::RefreshAllEntityArchetypeInfos()
{
    for (size_t i = 1; i < GetInstance().m_entityArchetypeInfos.size(); i++)
    {
        RefreshEntityArchetypeInfo(i);
    }
}

size_t EntityQuery::GetEntityAmount() const
{
    return EntityManager::GetEntityAmount(*this);
}
#pragma endregion
