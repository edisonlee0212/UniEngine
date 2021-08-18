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

    if (EntityManager::GetCurrentScene().get() == this)
    {
        for (auto &i : EntityManager::GetInstance().m_entityArchetypeInfos)
        {
            i.m_dataComponentStorageIndex = 0;
        }
        for (auto &i : EntityManager::GetInstance().m_entityQueryInfos)
        {
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
    for (auto &entityInfo : m_sceneDataStorage.m_entityInfos)
    {
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            if (!privateComponentElement.m_privateComponentData->m_started)
            {
                privateComponentElement.m_privateComponentData->Start();
                privateComponentElement.m_privateComponentData->m_started = true;
            }
            privateComponentElement.m_privateComponentData->PreUpdate();
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerManager::StartEvent(i.second->GetTypeName());
            if (!i.second->m_started)
            {
                i.second->Start();
                i.second->m_started = true;
            }
            i.second->PreUpdate();
            ProfilerManager::EndEvent(i.second->GetTypeName());
        }
    }
}

void Scene::Update()
{
    for (auto &entityInfo : m_sceneDataStorage.m_entityInfos)
    {
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->Update();
        }
    }

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
    for (auto &entityInfo : m_sceneDataStorage.m_entityInfos)
    {
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->LateUpdate();
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerManager::StartEvent(i.second->GetTypeName());
            i.second->LateUpdate();
            ProfilerManager::EndEvent(i.second->GetTypeName());
        }
    }
}
void Scene::FixedUpdate()
{

    for (auto &entityInfo : m_sceneDataStorage.m_entityInfos)
    {
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->FixedUpdate();
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
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
        if(i.second->m_started) i.second->OnGui();
    }
}
void Scene::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "Scene" << YAML::Value << m_name;

    std::unordered_map<Handle, std::shared_ptr<IAsset>> assetMap;
    std::vector<AssetRef> list;
    auto &sceneDataStorage = m_sceneDataStorage;
#pragma region EntityInfo
    out << YAML::Key << "EntityInfos" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_entityInfos.size(); i++)
    {
        sceneDataStorage.m_entityInfos[i].Serialize(out);
        for (const auto &i : sceneDataStorage.m_entityInfos[i].m_privateComponentElements)
        {
            i.m_privateComponentData->CollectAssetRef(list);
        }
    }
    out << YAML::EndSeq;
#pragma endregion

#pragma region Systems
    out << YAML::Key << "Systems" << YAML::Value << YAML::BeginSeq;
    for (const auto &i : m_systems)
    {
        SerializeSystem(i.second, out);
    }
    out << YAML::EndSeq;
#pragma endregion

#pragma region DataComponentStorage
    out << YAML::Key << "DataComponentStorages" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_dataComponentStorages.size(); i++)
    {
        SerializeDataComponentStorage(sceneDataStorage.m_dataComponentStorages[i], out);
    }
    out << YAML::EndSeq;
#pragma endregion
#pragma region Entities
    out << YAML::Key << "Entities" << YAML::Value
        << YAML::Binary(
               (const unsigned char *)sceneDataStorage.m_entities.data(),
               sceneDataStorage.m_entities.size() * sizeof(Entity));
#pragma endregion

#pragma region Assets
    for (auto &i : list)
    {
        auto asset = i.Get<IAsset>();
        if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle() && asset->GetPath().empty())
        {
            assetMap[asset->GetHandle()] = asset;
        }
    }
    bool listCheck = true;
    while (listCheck)
    {
        size_t currentSize = assetMap.size();
        list.clear();
        for (auto &i : assetMap)
        {
            i.second->CollectAssetRef(list);
        }
        for (auto &i : list)
        {
            auto asset = i.Get<IAsset>();
            if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle() && asset->GetPath().empty())
            {
                assetMap[asset->GetHandle()] = asset;
            }
        }
        if (assetMap.size() == currentSize)
            listCheck = false;
    }
    if (!assetMap.empty())
    {
        out << YAML::Key << "LocalAssets" << YAML::Value << YAML::BeginSeq;
        for (auto &i : assetMap)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "TypeName" << YAML::Value << i.second->GetTypeName();
            out << YAML::Key << "Handle" << YAML::Value << i.first.GetValue();
            out << YAML::Key << "Name" << YAML::Value << i.second->m_name;
            i.second->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
#pragma endregion
    out << YAML::EndMap;
}
void Scene::Deserialize(const YAML::Node &in)
{
    if (!in["Scene"])
    {
        UNIENGINE_ERROR("Scene load failed!");
    }
    UNIENGINE_LOG("Loading scene...");
    auto &sceneDataStorage = m_sceneDataStorage;
    m_name = in["Scene"].as<std::string>();

    // Must attach current scene to entitymanager before loading!
    // assert(EntityManager::GetCurrentScene().get() == this);

#pragma region Assets
    std::vector<std::shared_ptr<IAsset>> localAssets;
    auto inLocalAssets = in["LocalAssets"];
    if (inLocalAssets)
    {
        for (const auto &i : inLocalAssets)
        {
            Handle handle = i["Handle"].as<uint64_t>();
            localAssets.push_back(
                AssetManager::CreateAsset(i["TypeName"].as<std::string>(), handle, i["Name"].as<std::string>()));
        }
        int index = 0;
        for (const auto &i : inLocalAssets)
        {
            localAssets[index++]->Deserialize(i);
        }
    }

#pragma endregion
#pragma region DataComponentStorage
    auto inDataComponentStorages = in["DataComponentStorages"];
    for (const auto &inDataComponentStorage : inDataComponentStorages)
    {
        sceneDataStorage.m_dataComponentStorages.emplace_back();
        auto &dataComponentStorage = sceneDataStorage.m_dataComponentStorages.back();
        dataComponentStorage.m_entitySize = inDataComponentStorage["EntitySize"].as<size_t>();
        dataComponentStorage.m_chunkCapacity = inDataComponentStorage["ChunkCapacity"].as<size_t>();
        dataComponentStorage.m_entityCount = inDataComponentStorage["EntityCount"].as<size_t>();
        dataComponentStorage.m_entityAliveCount = inDataComponentStorage["EntityAliveCount"].as<size_t>();
        auto inDataComponentTypes = inDataComponentStorage["DataComponentTypes"];
        for (const auto &inDataComponentType : inDataComponentTypes)
        {
            DataComponentType dataComponentType;
            dataComponentType.m_name = inDataComponentType["Name"].as<std::string>();
            dataComponentType.m_size = inDataComponentType["Size"].as<size_t>();
            dataComponentType.m_offset = inDataComponentType["Offset"].as<size_t>();
            dataComponentType.m_typeId = SerializationManager::GetDataComponentTypeId(dataComponentType.m_name);
            dataComponentStorage.m_dataComponentTypes.push_back(dataComponentType);
        }
        auto inDataChunkArray = inDataComponentStorage["DataComponentChunkArray"];
        if (inDataChunkArray["Entities"].IsDefined())
        {
            YAML::Binary entitiesData = inDataChunkArray["Entities"].as<YAML::Binary>();
            dataComponentStorage.m_chunkArray.m_entities.resize(entitiesData.size() / sizeof(Entity));
            std::memcpy(dataComponentStorage.m_chunkArray.m_entities.data(), entitiesData.data(), entitiesData.size());
        }
        auto inChunks = inDataChunkArray["Chunks"];
        for (const auto &chunk : inChunks)
        {
            dataComponentStorage.m_chunkArray.m_chunks.emplace_back();
            auto &componentDataChunk = dataComponentStorage.m_chunkArray.m_chunks.back();
            componentDataChunk.m_data =
                static_cast<void *>(calloc(1, EntityManager::GetInstance().m_archetypeChunkSize));
            YAML::Binary chunkData = chunk["Data"].as<YAML::Binary>();
            assert(chunkData.size() == ARCHETYPE_CHUNK_SIZE);
            std::memcpy(componentDataChunk.m_data, chunkData.data(), chunkData.size());
        }
    }
#pragma endregion
#pragma region EntityInfo
    auto inEntityInfos = in["EntityInfos"];
    for (const auto &inEntityInfo : inEntityInfos)
    {
        sceneDataStorage.m_entityInfos.emplace_back();
        auto &newInfo = sceneDataStorage.m_entityInfos.back();
        newInfo.Deserialize(inEntityInfo);
    }
#pragma endregion
#pragma region Entities
    auto entitiesData = in["Entities"].as<YAML::Binary>();
    sceneDataStorage.m_entities.resize(entitiesData.size() / sizeof(Entity));
    std::memcpy(sceneDataStorage.m_entities.data(), entitiesData.data(), entitiesData.size());
#pragma endregion
    unsigned entityIndex = 1;
    for (const auto &inEntityInfo : inEntityInfos)
    {
        auto &entityInfo = sceneDataStorage.m_entityInfos.at(entityIndex);
        Entity entity;
        entity.m_index = entityIndex;
        entity.m_version = entityInfo.m_version;
        sceneDataStorage.m_entityMap[entityInfo.GetHandle()] = entity;
        entityIndex++;
    }
    entityIndex = 1;
    for (const auto &inEntityInfo : inEntityInfos)
    {
        auto &entityInfo = sceneDataStorage.m_entityInfos.at(entityIndex);
        Entity entity;
        entity.m_index = entityIndex;
        entity.m_version = entityInfo.m_version;
        auto inPrivateComponents = inEntityInfo["PrivateComponent"];
        if (inPrivateComponents)
        {
            for (const auto &inPrivateComponent : inPrivateComponents)
            {
                auto name = inPrivateComponent["TypeName"].as<std::string>();
                size_t hashCode;
                auto ptr = std::static_pointer_cast<IPrivateComponent>(
                    SerializationManager::ProduceSerializable(name, hashCode));
                ptr->m_enabled = inPrivateComponent["Enabled"].as<bool>();
                ptr->m_started = false;
                m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                entityInfo.m_privateComponentElements.emplace_back(hashCode, ptr, entity);
            }
        }
        entityIndex++;
    }

#pragma region Systems
    auto inSystems = in["Systems"];
    std::vector<std::shared_ptr<ISystem>> systems;
    for (const auto &inSystem : inSystems)
    {
        auto name = inSystem["TypeName"].as<std::string>();
        size_t hashCode;
        auto ptr = std::static_pointer_cast<ISystem>(SerializationManager::ProduceSerializable(name, hashCode));
        ptr->m_handle = Handle(inSystem["Handle"].as<uint64_t>());
        ptr->m_enabled = inSystem["Enabled"].as<bool>();
        ptr->m_rank = inSystem["Rank"].as<float>();
        ptr->m_started = false;
        m_systems.insert({ptr->m_rank, ptr});
        m_indexedSystems.insert({hashCode, ptr});
        m_mappedSystems[ptr->m_handle] = ptr;
        systems.push_back(ptr);
        ptr->OnCreate();
    }
#pragma endregion

    entityIndex = 1;
    for (const auto &inEntityInfo : inEntityInfos)
    {
        auto &entityInfo = sceneDataStorage.m_entityInfos.at(entityIndex);
        ;
        auto inPrivateComponents = inEntityInfo["PrivateComponent"];
        int componentIndex = 0;
        if (inPrivateComponents)
        {
            for (const auto &inPrivateComponent : inPrivateComponents)
            {
                auto name = inPrivateComponent["TypeName"].as<std::string>();
                auto ptr = entityInfo.m_privateComponentElements[componentIndex].m_privateComponentData;
                ptr->Deserialize(inPrivateComponent);
                componentIndex++;
            }
        }
        entityIndex++;
    }

    int systemIndex = 0;
    for (const auto &inSystem : inSystems)
    {
        auto name = inSystem["TypeName"].as<std::string>();
        systems[systemIndex]->Deserialize(inSystem);
        systemIndex++;
    }
}
void Scene::SerializeDataComponentStorage(const DataComponentStorage &storage, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "EntitySize" << YAML::Value << storage.m_entitySize;
        out << YAML::Key << "ChunkCapacity" << YAML::Value << storage.m_chunkCapacity;
        out << YAML::Key << "EntityCount" << YAML::Value << storage.m_entityCount;
        out << YAML::Key << "EntityAliveCount" << YAML::Value << storage.m_entityAliveCount;
        out << YAML::Key << "DataComponentTypes" << YAML::Value << YAML::BeginSeq;
        for (const auto &i : storage.m_dataComponentTypes)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << i.m_name;
            out << YAML::Key << "Size" << YAML::Value << i.m_size;
            out << YAML::Key << "Offset" << YAML::Value << i.m_offset;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::Key << "DataComponentChunkArray" << YAML::Value << YAML::BeginMap;
        {
            auto &array = storage.m_chunkArray;
            out << YAML::Key << "Entities" << YAML::Value
                << YAML::Binary(
                       (const unsigned char *)array.m_entities.data(), array.m_entities.size() * sizeof(Entity));
            out << YAML::Key << "Chunks" << YAML::Value << YAML::BeginSeq;
            for (const auto &i : array.m_chunks)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Data" << YAML::Value
                    << YAML::Binary((const unsigned char *)i.m_data, ARCHETYPE_CHUNK_SIZE);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;
    }
    out << YAML::EndMap;
}
void Scene::SerializeSystem(const std::shared_ptr<ISystem> &system, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "TypeName" << YAML::Value << system->GetTypeName();
        out << YAML::Key << "Enabled" << YAML::Value << system->m_enabled;
        out << YAML::Key << "Rank" << YAML::Value << system->m_rank;
        out << YAML::Key << "Handle" << YAML::Value << system->GetHandle();
        system->Serialize(out);
    }
    out << YAML::EndMap;
}
void Scene::OnCreate()
{
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityInfos.emplace_back();
    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
}
