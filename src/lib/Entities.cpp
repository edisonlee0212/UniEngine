#include "Application.hpp"
#include <AssetManager.hpp>
#include "Engine/Utilities/Console.hpp"
#include <Entity.hpp>
#include "Engine/ECS/Entities.hpp"
#include <PhysicsLayer.hpp>
#include <Scene.hpp>
using namespace UniEngine;

#pragma region EntityManager

void Entities::UnsafeForEachDataComponent(
    const std::shared_ptr<Scene> &scene,
    const Entity &entity,
    const std::function<void(const DataComponentType &type, void *data)> &func)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index);
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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

void Entities::ForEachPrivateComponent(
    const std::shared_ptr<Scene> &scene,
    const Entity &entity,
    const std::function<void(PrivateComponentElement &data)> &func)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto elements = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements;
    for (auto &component : elements)
    {
        func(component);
    }
}

void Entities::UnsafeForEachEntityStorage(
    const std::shared_ptr<Scene> &scene,
    const std::function<void(int i, const std::string &name, const DataComponentStorage &storage)> &func)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }

    auto &archetypeInfos = entityManager.m_entityArchetypeInfos;
    for (int i = 0; i < archetypeInfos.size(); i++)
    {
        auto dcs = GetDataComponentStorage(scene, i);
        if (!dcs.has_value())
            continue;
        func(i, archetypeInfos[i].m_name, dcs->first.get());
    }
}

size_t Entities::GetArchetypeChunkSize()
{
    auto &entityManager = GetInstance();
    return entityManager.m_archetypeChunkSize;
}

void Entities::DeleteEntityInternal(const std::shared_ptr<Scene> &scene, unsigned entityIndex)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entityIndex);
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    Entity actualEntity = scene->m_sceneDataStorage.m_entities.at(entityIndex);

    scene->m_sceneDataStorage.m_entityPrivateComponentStorage.DeleteEntity(actualEntity);
    entityInfo.m_version = actualEntity.m_version + 1;
    entityInfo.m_enabled = true;

    scene->m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = Entity();
    entityInfo.m_handle = Handle(0);

    for (auto &i : entityInfo.m_privateComponentElements)
    {
        i.m_privateComponentData->OnDestroy();
    }
    entityInfo.m_privateComponentElements.clear();
    // Set to version 0, marks it as deleted.
    actualEntity.m_version = 0;
    dataComponentStorage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = actualEntity;
    const auto originalIndex = entityInfo.m_chunkArrayIndex;
    if (entityInfo.m_chunkArrayIndex != dataComponentStorage.m_entityAliveCount - 1)
    {
        const auto swappedIndex = SwapEntity(
            scene, dataComponentStorage, entityInfo.m_chunkArrayIndex, dataComponentStorage.m_entityAliveCount - 1);
        entityInfo.m_chunkArrayIndex = dataComponentStorage.m_entityAliveCount - 1;
        scene->m_sceneDataStorage.m_entityInfos.at(swappedIndex).m_chunkArrayIndex = originalIndex;
    }
    dataComponentStorage.m_entityAliveCount--;

    scene->m_sceneDataStorage.m_entities.at(entityIndex) = actualEntity;
}

std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> Entities::GetDataComponentStorage(
    const std::shared_ptr<Scene> &scene, unsigned entityArchetypeIndex)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return std::nullopt;
    }
    auto &archetypeInfo = entityManager.m_entityArchetypeInfos.at(entityArchetypeIndex);
    int targetIndex = 0;
    for (auto &i : scene->m_sceneDataStorage.m_dataComponentStorages)
    {
        if (i.m_dataComponentTypes.size() != archetypeInfo.m_dataComponentTypes.size())
        {
            targetIndex++;
            continue;
        }
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
            return {{std::ref(i), targetIndex}};
        }
        targetIndex++;
    }
    // If we didn't find the target storage, then we need to create a new one.
    scene->m_sceneDataStorage.m_dataComponentStorages.emplace_back(archetypeInfo);
    return {
        {std::ref(scene->m_sceneDataStorage.m_dataComponentStorages.back()),
         scene->m_sceneDataStorage.m_dataComponentStorages.size() - 1}};
}

std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> Entities::GetDataComponentStorage(
    const std::shared_ptr<Scene> &scene, const EntityArchetype &entityArchetype)
{
    return GetDataComponentStorage(scene, entityArchetype.m_index);
}

std::vector<std::reference_wrapper<DataComponentStorage>> Entities::QueryDataComponentStorages(
    const std::shared_ptr<Scene> &scene, const EntityQuery &entityQuery)
{
    return QueryDataComponentStorages(scene, entityQuery.m_index);
}

void Entities::EraseDuplicates(std::vector<DataComponentType> &types)
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

void Entities::GetEntityStorage(
    const std::shared_ptr<Scene> &scene,
    const DataComponentStorage &storage,
    std::vector<Entity> &container,
    bool checkEnable)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    const size_t amount = storage.m_entityAliveCount;
    if (amount == 0)
        return;
    if (checkEnable)
    {
        auto &workers = JobManager::Workers();
        const auto capacity = storage.m_chunkCapacity;
        const auto &chunkArray = storage.m_chunkArray;
        const auto &entities = &chunkArray.m_entities;
        std::vector<std::shared_future<void>> results;
        const auto threadSize = workers.Size();
        const auto threadLoad = amount / threadSize;
        const auto loadReminder = amount % threadSize;
        std::vector<std::vector<Entity>> tempStorage;
        tempStorage.resize(threadSize);
        for (int threadIndex = 0; threadIndex < threadSize; threadIndex++)
        {
            results.push_back(workers
                                  .Push([=, &chunkArray, &entities, &tempStorage](int id) {
                                      for (int i = threadIndex * threadLoad; i < (threadIndex + 1) * threadLoad; i++)
                                      {
                                          const auto chunkIndex = i / capacity;
                                          const auto remainder = i % capacity;
                                          const auto entity = entities->at(i);
                                          if (!scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_enabled)
                                              continue;
                                          tempStorage[threadIndex].push_back(entity);
                                      }
                                      if (threadIndex < loadReminder)
                                      {
                                          const int i = threadIndex + threadSize * threadLoad;
                                          const auto chunkIndex = i / capacity;
                                          const auto remainder = i % capacity;
                                          const auto entity = entities->at(i);
                                          if (!scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_enabled)
                                              return;
                                          tempStorage[threadIndex].push_back(entity);
                                      }
                                  })
                                  .share());
        }
        for (const auto &i : results)
            i.wait();
        for (auto &i : tempStorage)
        {
            container.insert(container.end(), i.begin(), i.end());
        }
    }
    else
    {
        container.resize(container.size() + amount);
        const size_t capacity = storage.m_chunkCapacity;
        memcpy(
            &container.at(container.size() - amount), storage.m_chunkArray.m_entities.data(), amount * sizeof(Entity));
    }
}

size_t Entities::SwapEntity(
    const std::shared_ptr<Scene> &scene, DataComponentStorage &storage, size_t index1, size_t index2)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return -1;
    }
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

void Entities::GetAllEntities(const std::shared_ptr<Scene> &scene, std::vector<Entity> &target)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    target.insert(
        target.end(), scene->m_sceneDataStorage.m_entities.begin() + 1, scene->m_sceneDataStorage.m_entities.end());
}

void Entities::ForEachDescendant(
    const std::shared_ptr<Scene> &scene,
    const Entity &target,
    const std::function<void(const std::shared_ptr<Scene> &scene, const Entity &entity)> &func,
    const bool &fromRoot)
{
    Entity realTarget = target;
    if (!realTarget.IsValid())
        return;
    if (fromRoot)
        realTarget = GetRoot(scene, realTarget);
    ForEachDescendantHelper(scene, realTarget, func);
}

std::vector<Entity> *Entities::UnsafeGetAllEntities(const std::shared_ptr<Scene> &scene)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return nullptr;
    }
    return &scene->m_sceneDataStorage.m_entities;
}

void Entities::Attach(const std::shared_ptr<Scene> &scene)
{
    if (Application::IsPlaying())
    {
        UNIENGINE_ERROR("Stop Application to attach scene");
    }
    auto &entityManager = GetInstance();
    entityManager.m_scene = scene;
    for (auto &func : Application::GetInstance().m_postAttachSceneFunctions)
    {
        func(scene);
    }
}

Entity Entities::CreateEntity(const std::shared_ptr<Scene> &scene, const std::string &name)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return Entity();
    }
    return CreateEntity(scene, entityManager.m_basicArchetype, name);
}

Entity Entities::CreateEntity(
    const std::shared_ptr<Scene> &scene,
    const EntityArchetype &archetype,
    const std::string &name,
    const Handle &handle)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return Entity();
    }
    assert(archetype.IsValid());
    entityManager.m_scene->m_saved = false;
    Entity retVal;
    auto search = GetDataComponentStorage(scene, archetype);
    DataComponentStorage &storage = search->first;
    if (storage.m_entityCount == storage.m_entityAliveCount)
    {
        const size_t chunkIndex = storage.m_entityCount / storage.m_chunkCapacity + 1;
        if (storage.m_chunkArray.m_chunks.size() <= chunkIndex)
        {
            // Allocate new chunk;
            ComponentDataChunk chunk;
            chunk.m_data = static_cast<void *>(calloc(1, entityManager.m_archetypeChunkSize));
            storage.m_chunkArray.m_chunks.push_back(chunk);
        }
        retVal.m_index = scene->m_sceneDataStorage.m_entities.size();
        // If the version is 0 in chunk means it's deleted.
        retVal.m_version = 1;
        EntityMetadata entityInfo;
        entityInfo.m_root = retVal;
        entityInfo.m_static = false;
        entityInfo.m_name = name;
        entityInfo.m_handle = handle;
        entityInfo.m_dataComponentStorageIndex = search->second;
        entityInfo.m_chunkArrayIndex = storage.m_entityCount;
        storage.m_chunkArray.m_entities.push_back(retVal);

        scene->m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = retVal;
        scene->m_sceneDataStorage.m_entityInfos.push_back(std::move(entityInfo));
        scene->m_sceneDataStorage.m_entities.push_back(retVal);
        storage.m_entityCount++;
        storage.m_entityAliveCount++;
    }
    else
    {
        retVal = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(retVal.m_index);
        entityInfo.m_root = retVal;
        entityInfo.m_static = false;
        entityInfo.m_handle = handle;
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        retVal.m_version = entityInfo.m_version;

        scene->m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = retVal;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = retVal;
        scene->m_sceneDataStorage.m_entities.at(retVal.m_index) = retVal;
        storage.m_entityAliveCount++;
        // Reset all component data
        const auto chunkIndex = entityInfo.m_chunkArrayIndex / storage.m_chunkCapacity;
        const auto chunkPointer = entityInfo.m_chunkArrayIndex % storage.m_chunkCapacity;
        const auto chunk = storage.m_chunkArray.m_chunks[chunkIndex];
        for (const auto &i : storage.m_dataComponentTypes)
        {
            const auto offset = i.m_offset * storage.m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
    }
    SetDataComponent(scene, retVal, Transform());
    SetDataComponent(scene, retVal, GlobalTransform());
    SetDataComponent(scene, retVal, GlobalTransformUpdateFlag());
    return retVal;
}

std::vector<Entity> Entities::CreateEntities(
    const std::shared_ptr<Scene> &scene,
    const EntityArchetype &archetype,
    const size_t &amount,
    const std::string &name)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    assert(archetype.IsValid());
    std::vector<Entity> retVal;
    entityManager.m_scene->m_saved = false;
    auto search = GetDataComponentStorage(scene, archetype);
    DataComponentStorage &storage = search->first;
    auto remainAmount = amount;
    const Transform transform;
    const GlobalTransform globalTransform;
    const GlobalTransformUpdateFlag transformStatus;
    while (remainAmount > 0 && storage.m_entityAliveCount != storage.m_entityCount)
    {
        remainAmount--;
        Entity entity = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index);
        entityInfo.m_root = entity;
        entityInfo.m_static = false;
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        entity.m_version = entityInfo.m_version;
        entityInfo.m_handle = Handle();
        scene->m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = entity;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
        scene->m_sceneDataStorage.m_entities.at(entity.m_index) = entity;
        storage.m_entityAliveCount++;
        // Reset all component data
        const size_t chunkIndex = entityInfo.m_chunkArrayIndex / storage.m_chunkCapacity;
        const size_t chunkPointer = entityInfo.m_chunkArrayIndex % storage.m_chunkCapacity;
        const ComponentDataChunk &chunk = storage.m_chunkArray.m_chunks[chunkIndex];
        for (const auto &i : storage.m_dataComponentTypes)
        {
            const size_t offset = i.m_offset * storage.m_chunkCapacity + chunkPointer * i.m_size;
            chunk.ClearData(offset, i.m_size);
        }
        retVal.push_back(entity);
        SetDataComponent(scene, entity, transform);
        SetDataComponent(scene, entity, globalTransform);
        SetDataComponent(scene, entity, GlobalTransformUpdateFlag());
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
        chunk.m_data = static_cast<void *>(calloc(1, entityManager.m_archetypeChunkSize));
        storage.m_chunkArray.m_chunks.push_back(chunk);
    }
    const size_t originalSize = scene->m_sceneDataStorage.m_entities.size();
    scene->m_sceneDataStorage.m_entities.resize(originalSize + remainAmount);
    scene->m_sceneDataStorage.m_entityInfos.resize(originalSize + remainAmount);

    for (int i = 0; i < remainAmount; i++)
    {
        auto &entity = scene->m_sceneDataStorage.m_entities.at(originalSize + i);
        entity.m_index = originalSize + i;
        entity.m_version = 1;

        auto &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(originalSize + i);
        entityInfo = EntityMetadata();
        entityInfo.m_root = entity;
        entityInfo.m_static = false;
        entityInfo.m_name = name;
        entityInfo.m_dataComponentStorageIndex = search->second;
        entityInfo.m_chunkArrayIndex = storage.m_entityAliveCount - remainAmount + i;

        entityInfo.m_handle = Handle();

        scene->m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = entity;
    }

    storage.m_chunkArray.m_entities.insert(
        storage.m_chunkArray.m_entities.end(),
        scene->m_sceneDataStorage.m_entities.begin() + originalSize,
        scene->m_sceneDataStorage.m_entities.end());
    const int threadSize = JobManager::Workers().Size();
    int perThreadAmount = remainAmount / threadSize;
    if (perThreadAmount > 0)
    {
        std::vector<std::shared_future<void>> results;
        for (int i = 0; i < threadSize; i++)
        {
            results.push_back(JobManager::Workers()
                                  .Push([i, perThreadAmount, originalSize, scene](int id) {
                                      const Transform transform;
                                      const GlobalTransform globalTransform;
                                      for (int index = originalSize + i * perThreadAmount;
                                           index < originalSize + (i + 1) * perThreadAmount;
                                           index++)
                                      {
                                          auto &entity = scene->m_sceneDataStorage.m_entities.at(index);
                                          SetDataComponent(scene, entity, transform);
                                          SetDataComponent(scene, entity, globalTransform);
                                          SetDataComponent(scene, entity, GlobalTransformUpdateFlag());
                                      }
                                  })
                                  .share());
        }
        results.push_back(JobManager::Workers()
                              .Push([perThreadAmount, originalSize, &remainAmount, threadSize, scene](int id) {
                                  const Transform transform;
                                  const GlobalTransform globalTransform;
                                  for (int index = originalSize + perThreadAmount * threadSize;
                                       index < originalSize + remainAmount;
                                       index++)
                                  {
                                      auto &entity = scene->m_sceneDataStorage.m_entities.at(index);
                                      SetDataComponent(scene, entity, transform);
                                      SetDataComponent(scene, entity, globalTransform);
                                      SetDataComponent(scene, entity, GlobalTransformUpdateFlag());
                                  }
                              })
                              .share());
        for (const auto &i : results)
            i.wait();
    }

    retVal.insert(
        retVal.end(),
        scene->m_sceneDataStorage.m_entities.begin() + originalSize,
        scene->m_sceneDataStorage.m_entities.end());
    return retVal;
}

std::vector<Entity> Entities::CreateEntities(
    const std::shared_ptr<Scene> &scene, const size_t &amount, const std::string &name)
{
    auto &entityManager = GetInstance();
    return CreateEntities(scene, entityManager.m_basicArchetype, amount, name);
}

void Entities::DeleteEntity(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    if (!entity.IsValid())
    {
        return;
    }
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    entityManager.m_scene->m_saved = false;
    const size_t entityIndex = entity.m_index;
    auto children = scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_children;
    for (const auto &child : children)
    {
        DeleteEntity(scene, child);
    }
    if (scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_parent.m_index != 0)
        RemoveChild(scene, entity, scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_parent);
    DeleteEntityInternal(scene, entity.m_index);
}

std::string Entities::GetEntityName(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(entity.IsValid());
    const size_t index = entity.m_index;
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return "";
    }
    if (entity != scene->m_sceneDataStorage.m_entities.at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return "";
    }
    return scene->m_sceneDataStorage.m_entityInfos.at(index).m_name;
}

void Entities::SetEntityName(const std::shared_ptr<Scene> &scene, const Entity &entity, const std::string &name)
{
    assert(entity.IsValid());
    const size_t index = entity.m_index;
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    if (entity != scene->m_sceneDataStorage.m_entities.at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return;
    }
    entityManager.m_scene->m_saved = false;
    if (name.length() != 0)
    {
        scene->m_sceneDataStorage.m_entityInfos.at(index).m_name = name;
        return;
    }
    scene->m_sceneDataStorage.m_entityInfos.at(index).m_name = "Unnamed";
}
void Entities::SetEntityStatic(const std::shared_ptr<Scene> &scene, const Entity &entity, bool value)
{
    assert(entity.IsValid());
    const size_t childIndex = entity.m_index;
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.GetRoot().m_index);
    entityInfo.m_static = value;
}
void Entities::SetParent(
    const std::shared_ptr<Scene> &scene, const Entity &entity, const Entity &parent, const bool &recalculateTransform)
{
    assert(entity.IsValid() && parent.IsValid());
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &parentEntityInfo = scene->m_sceneDataStorage.m_entityInfos.at(parentIndex);
    for (const auto &i : parentEntityInfo.m_children)
    {
        if (i == entity)
            return;
    }
    entityManager.m_scene->m_saved = false;
    auto &childEntityInfo = scene->m_sceneDataStorage.m_entityInfos.at(childIndex);
    if (!childEntityInfo.m_parent.IsNull())
    {
        RemoveChild(scene, entity, childEntityInfo.m_parent);
    }
    if (recalculateTransform)
    {
        const auto childGlobalTransform = GetDataComponent<GlobalTransform>(scene, entity);
        const auto parentGlobalTransform = GetDataComponent<GlobalTransform>(scene, parent);
        Transform childTransform;
        childTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * childGlobalTransform.m_value;
        SetDataComponent(scene, entity, childTransform);
    }
    childEntityInfo.m_parent = parent;
    childEntityInfo.m_root = parentEntityInfo.m_root;
    childEntityInfo.m_static = false;
    parentEntityInfo.m_children.push_back(entity);
}

Entity Entities::GetParent(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return Entity();
    }
    const size_t entityIndex = entity.m_index;
    return scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_parent;
}

std::vector<Entity> Entities::GetChildren(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    const size_t entityIndex = entity.m_index;
    return scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_children;
}

Entity Entities::GetChild(const std::shared_ptr<Scene> &scene, const Entity &entity, int index)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return Entity();
    }
    const size_t entityIndex = entity.m_index;
    auto &children = scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_children;
    if (children.size() > index)
        return children[index];
    return Entity();
}

size_t Entities::GetChildrenAmount(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return 0;
    }
    const size_t entityIndex = entity.m_index;
    return scene->m_sceneDataStorage.m_entityInfos.at(entityIndex).m_children.size();
}

inline void Entities::ForEachChild(
    const std::shared_ptr<Scene> &scene,
    const Entity &entity,
    const std::function<void(const std::shared_ptr<Scene> &scene, Entity child)> &func)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto children = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_children;
    for (auto i : children)
    {
        if (i.IsValid())
            func(scene, i);
    }
}

void Entities::RemoveChild(const std::shared_ptr<Scene> &scene, const Entity &entity, const Entity &parent)
{
    assert(entity.IsValid() && parent.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    auto &childEntityMetadata = scene->m_sceneDataStorage.m_entityInfos.at(childIndex);
    auto &parentEntityMetadata = scene->m_sceneDataStorage.m_entityInfos.at(parentIndex);
    if (childEntityMetadata.m_parent.m_index == 0)
    {
        UNIENGINE_ERROR("No child by the parent!");
    }
    entityManager.m_scene->m_saved = false;
    childEntityMetadata.m_parent = Entity();
    childEntityMetadata.m_root = entity;
    const size_t childrenCount = parentEntityMetadata.m_children.size();

    for (int i = 0; i < childrenCount; i++)
    {
        if (parentEntityMetadata.m_children[i].m_index == childIndex)
        {
            parentEntityMetadata.m_children[i] = parentEntityMetadata.m_children.back();
            parentEntityMetadata.m_children.pop_back();
            break;
        }
    }
    const auto childGlobalTransform = GetDataComponent<GlobalTransform>(scene, entity);
    Transform childTransform;
    childTransform.m_value = childGlobalTransform.m_value;
    SetDataComponent(scene, entity, childTransform);
}

void Entities::RemoveDataComponent(const std::shared_ptr<Scene> &scene, const Entity &entity, const size_t &typeID)
{
    assert(entity.IsValid());
    if (typeID == typeid(Transform).hash_code() || typeID == typeid(GlobalTransform).hash_code() ||
        typeID == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return;
    }
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index);
    auto &entityArchetypeInfos = entityManager.m_entityArchetypeInfos;
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
    newArchetypeInfo.m_chunkCapacity = entityManager.m_archetypeChunkSize / newArchetypeInfo.m_entitySize;
    auto archetype = CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(scene, archetype);
    // Transfer component data
    for (const auto &type : newArchetypeInfo.m_dataComponentTypes)
    {
        SetDataComponent(
            scene,
            newEntity.m_index,
            type.m_typeId,
            type.m_size,
            GetDataComponentPointer(scene, entity, type.m_typeId));
    }
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = scene->m_sceneDataStorage.m_entityInfos.at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = entityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    scene->m_sceneDataStorage.m_dataComponentStorages.at(entityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
    scene->m_sceneDataStorage.m_dataComponentStorages.at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(scene, newEntity);
#pragma endregion
    entityManager.m_scene->m_saved = false;
}

void Entities::SetDataComponent(
    const std::shared_ptr<Scene> &scene, const unsigned &entityIndex, size_t id, size_t size, IDataComponent *data)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entityIndex);
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
                    static_cast<size_t>(
                        type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * type.m_size),
                    size,
                    data);
                return;
            }
        }
        UNIENGINE_LOG("ComponentData doesn't exist");
    }
}
IDataComponent *Entities::GetDataComponentPointer(
    const std::shared_ptr<Scene> &scene, unsigned entityIndex, const size_t &id)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return nullptr;
    }
    EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entityIndex);
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
IDataComponent *Entities::GetDataComponentPointer(
    const std::shared_ptr<Scene> &scene, const Entity &entity, const size_t &id)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return nullptr;
    }
    EntityMetadata &entityInfo = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index);
    auto &dataComponentStorage =
        scene->m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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

EntityArchetype Entities::CreateEntityArchetype(
    const std::string &name, const std::vector<DataComponentType> &types)
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
    EraseDuplicates(actualTypes);
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

void Entities::SetPrivateComponent(
    const std::shared_ptr<Scene> &scene, const Entity &entity, std::shared_ptr<IPrivateComponent> ptr)
{
    assert(ptr && entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto typeName = ptr->GetTypeName();
    bool found = false;
    size_t i = 0;
    auto &elements = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (typeName == element.m_privateComponentData->GetTypeName())
        {
            found = true;
            if (element.m_privateComponentData)
            {
                element.m_privateComponentData->OnDestroy();
            }
            element.m_privateComponentData = ptr;
            element.ResetOwner(entity, scene);
            element.m_privateComponentData->OnCreate();
        }
        i++;
    }
    if (!found)
    {
        auto id = SerializationManager::GetSerializableTypeId(typeName);
        scene->m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, id);
        elements.emplace_back(id, ptr, entity, scene);
    }
    entityManager.m_scene->m_saved = false;
}

void Entities::ForEachDescendantHelper(
    const std::shared_ptr<Scene> &scene,
    const Entity &target,
    const std::function<void(const std::shared_ptr<Scene> &scene, const Entity &entity)> &func)
{
    func(scene, target);
    ForEachChild(scene, target, [&](const std::shared_ptr<Scene> &scene, Entity child) {
        ForEachDescendantHelper(scene, child, func);
    });
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

Entity Entities::GetRoot(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    Entity retVal = entity;
    auto parent = GetParent(scene, retVal);
    while (!parent.IsNull())
    {
        retVal = parent;
        parent = GetParent(scene, retVal);
    }
    return retVal;
}

Entity Entities::GetEntity(const std::shared_ptr<Scene> &scene, const size_t &index)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    if (index > 0 && index < scene->m_sceneDataStorage.m_entities.size())
        return scene->m_sceneDataStorage.m_entities.at(index);
    return {};
}

void Entities::RemovePrivateComponent(const std::shared_ptr<Scene> &scene, const Entity &entity, size_t typeId)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &privateComponentElements =
        scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements;
    for (auto i = 0; i < privateComponentElements.size(); i++)
    {
        if (privateComponentElements[i].m_typeId == typeId)
        {
            privateComponentElements[i].m_privateComponentData->OnDestroy();
            privateComponentElements.erase(privateComponentElements.begin() + i);
            break;
        }
    }
    entityManager.m_scene->m_saved = false;
    scene->m_sceneDataStorage.m_entityPrivateComponentStorage.RemovePrivateComponent(entity, typeId);
}

void Entities::SetEnable(const std::shared_ptr<Scene> &scene, const Entity &entity, const bool &value)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    if (scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_enabled != value)
    {
        for (auto &i : scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements)
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
    scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_enabled = value;

    for (const auto &i : scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_children)
    {
        SetEnable(scene, i, value);
    }
    entityManager.m_scene->m_saved = false;
}

void Entities::SetEnableSingle(const std::shared_ptr<Scene> &scene, const Entity &entity, const bool &value)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &entityMetadata = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index);
    if (entityMetadata.m_enabled != value)
    {
        for (auto &i : entityMetadata.m_privateComponentElements)
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
        entityMetadata.m_enabled = value;
        entityManager.m_scene->m_saved = false;
    }
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

void Entities::ForAllEntities(
    const std::shared_ptr<Scene> &scene, const std::function<void(int i, Entity entity)> &func)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    for (int index = 0; index < scene->m_sceneDataStorage.m_entities.size(); index++)
    {
        if (scene->m_sceneDataStorage.m_entities.at(index).m_version != 0)
        {
            func(index, scene->m_sceneDataStorage.m_entities.at(index));
        }
    }
}

std::string Entities::GetEntityArchetypeName(const EntityArchetype &entityArchetype)
{
    auto &entityManager = GetInstance();
    return entityManager.m_entityArchetypeInfos[entityArchetype.m_index].m_name;
}

void Entities::GetEntityArray(
    const std::shared_ptr<Scene> &scene,
    const EntityQuery &entityQuery,
    std::vector<Entity> &container,
    bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(scene, entityQuery);
    for (const auto i : queriedStorages)
    {
        GetEntityStorage(scene, i.get(), container, checkEnable);
    }
}

void EntityQuery::ToEntityArray(
    const std::shared_ptr<Scene> &scene, std::vector<Entity> &container, bool checkEnable) const
{
    Entities::GetEntityArray(scene, *this, container, checkEnable);
}

size_t Entities::GetEntityAmount(const std::shared_ptr<Scene> &scene, EntityQuery entityQuery, bool checkEnable)
{
    assert(entityQuery.IsValid());
    size_t retVal = 0;
    auto &entityManager = GetInstance();
    if (checkEnable)
    {
        auto queriedStorages = QueryDataComponentStorages(scene, entityQuery);
        for (const auto i : queriedStorages)
        {
            for (int index = 0; index < i.get().m_entityAliveCount; index++)
            {
                if (i.get().m_chunkArray.m_entities[index].IsEnabled())
                    retVal++;
            }
        }
    }
    else
    {
        auto queriedStorages = QueryDataComponentStorages(scene, entityQuery);
        for (const auto i : queriedStorages)
        {
            retVal += i.get().m_entityAliveCount;
        }
    }
    return retVal;
}
void Entities::SetEntityArchetypeName(const EntityArchetype &entityArchetype, const std::string &name)
{
    auto &entityManager = GetInstance();
    entityManager.m_entityArchetypeInfos[entityArchetype.m_index].m_name = name;
}
std::vector<Entity> Entities::GetDescendants(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    std::vector<Entity> retVal;
    GetDescendantsHelper(scene, entity, retVal);
    return retVal;
}
void Entities::GetDescendantsHelper(
    const std::shared_ptr<Scene> &scene, const Entity &target, std::vector<Entity> &results)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return;
    }
    auto &children = scene->m_sceneDataStorage.m_entityInfos.at(target.m_index).m_children;
    if (!children.empty())
        results.insert(results.end(), children.begin(), children.end());
    for (const auto &i : children)
        GetDescendantsHelper(scene, i, results);
}
template <typename T>
std::vector<Entity> Entities::GetPrivateComponentOwnersList(const std::shared_ptr<Scene> &scene)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    return scene->m_sceneDataStorage.m_entityPrivateComponentStorage.GetOwnersList<T>();
}
void Entities::Init()
{
    auto &entityManager = GetInstance();
    entityManager.m_entityArchetypeInfos.emplace_back();
    entityManager.m_entityQueryInfos.emplace_back();

    entityManager.m_basicArchetype =
        CreateEntityArchetype("Basic", Transform(), GlobalTransform(), GlobalTransformUpdateFlag());
}
std::shared_ptr<Scene> Entities::GetCurrentScene()
{
    auto &entityManager = GetInstance();
    return entityManager.m_scene;
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

std::weak_ptr<IPrivateComponent> Entities::GetPrivateComponent(
    const std::shared_ptr<Scene> &scene, const Entity &entity, const std::string &typeName)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        throw 0;
    }
    size_t i = 0;
    auto &elements = scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (typeName == element.m_privateComponentData->GetTypeName())
        {
            return element.m_privateComponentData;
        }
        i++;
    }
    throw 0;
}

Entity Entities::GetEntity(const std::shared_ptr<Scene> &scene, const Handle &handle)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    auto search = scene->m_sceneDataStorage.m_entityMap.find(handle);
    if (search != scene->m_sceneDataStorage.m_entityMap.end())
    {
        return search->second;
    }
    return {};
}
bool Entities::HasPrivateComponent(
    const std::shared_ptr<Scene> &scene, const Entity &entity, const std::string &typeName)
{
    assert(entity.IsValid());
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return false;
    }
    for (auto &element : scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_privateComponentElements)
    {
        if (element.m_privateComponentData->m_typeName == typeName)
        {
            return true;
        }
    }
    return false;
}
std::vector<std::reference_wrapper<DataComponentStorage>> Entities::QueryDataComponentStorages(
    const std::shared_ptr<Scene> &scene, unsigned int entityQueryIndex)
{
    auto &entityManager = GetInstance();
    if (!scene)
    {
        return {};
    }
    auto &queryInfos = entityManager.m_entityQueryInfos.at(entityQueryIndex);
    auto &entityComponentStorage = scene->m_sceneDataStorage.m_dataComponentStorages;
    std::vector<std::reference_wrapper<DataComponentStorage>> queriedStorage;
    // Select storage with every contained.
    if (!queryInfos.m_allDataComponentTypes.empty())
    {
        for (int i = 0; i < entityComponentStorage.size(); i++)
        {
            auto &dataStorage = entityComponentStorage.at(i);
            bool check = true;
            for (const auto &type : queryInfos.m_allDataComponentTypes)
            {
                if (!dataStorage.HasType(type.m_typeId))
                    check = false;
            }
            if (check)
                queriedStorage.push_back(std::ref(dataStorage));
        }
    }
    else
    {
        for (int i = 0; i < entityComponentStorage.size(); i++)
        {
            auto &dataStorage = entityComponentStorage.at(i);
            queriedStorage.push_back(std::ref(dataStorage));
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
                if (queriedStorage.at(i).get().HasType(type.m_typeId))
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
                if (queriedStorage.at(i).get().HasType(type.m_typeId))
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
    return queriedStorage;
}
bool Entities::IsEntityValid(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    auto &storage = scene->m_sceneDataStorage.m_entities;
    return entity.m_index != 0 && entity.m_version != 0 && entity.m_index < storage.size() &&
           storage.at(entity.m_index).m_version == entity.m_version;
}
bool Entities::IsEntityEnabled(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(IsEntityValid(scene, entity));
    return scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_enabled;
}
bool Entities::IsEntityRoot(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(IsEntityValid(scene, entity));
    return scene->m_sceneDataStorage.m_entityInfos.at(entity.m_index).m_root == entity;
}
bool Entities::IsEntityStatic(const std::shared_ptr<Scene> &scene, const Entity &entity)
{
    assert(IsEntityValid(scene, entity));
    return scene->m_sceneDataStorage.m_entityInfos.at(entity.GetRoot().m_index).m_static;
}
size_t EntityQuery::GetEntityAmount(const std::shared_ptr<Scene> &scene, bool checkEnabled) const
{
    return Entities::GetEntityAmount(scene, *this, checkEnabled);
}
#pragma endregion
