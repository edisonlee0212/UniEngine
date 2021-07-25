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
        i.second->OnGui();
    }
}
void Scene::Serialize(YAML::Emitter &out)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << m_name;
    auto &sceneDataStorage = m_sceneDataStorage;
#pragma region EntityInfo
    out << YAML::Key << "EntityInfos" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_entityInfos.size(); i++)
    {
        SerializeEntityInfo(sceneDataStorage.m_entityInfos[i], out);
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
            dataComponentType.m_typeId = SerializableFactory::GetDataComponentTypeId(dataComponentType.m_name);
            dataComponentStorage.m_dataComponentTypes.push_back(dataComponentType);
        }
        auto inDataChunkArray = inDataComponentStorage["DataComponentChunkArray"];
        if (inDataChunkArray["Entities"].IsDefined())
        {
            YAML::Binary entitiesData = inDataChunkArray["Entities"].as<YAML::Binary>();
            const unsigned char *data = entitiesData.data();
            std::size_t size = entitiesData.size();
            dataComponentStorage.m_chunkArray.m_entities.resize(size / sizeof(Entity));
            std::memcpy(dataComponentStorage.m_chunkArray.m_entities.data(), data, size);
        }
        auto inChunks = inDataChunkArray["Chunks"];
        for (const auto &chunk : inChunks)
        {
            dataComponentStorage.m_chunkArray.m_chunks.emplace_back();
            auto &componentDataChunk = dataComponentStorage.m_chunkArray.m_chunks.back();
            componentDataChunk.m_data =
                static_cast<void *>(calloc(1, EntityManager::GetInstance().m_archetypeChunkSize));
            YAML::Binary chunkData = chunk["Data"].as<YAML::Binary>();
            const unsigned char *data = chunkData.data();
            std::size_t size = chunkData.size();
            assert(size == ARCHETYPE_CHUNK_SIZE);
            std::memcpy(componentDataChunk.m_data, data, size);
        }
    }
#pragma endregion
#pragma region EntityInfo
    auto inEntityInfos = in["EntityInfos"];
    int entityIndex = 1;
    for (const auto &inEntityInfo : inEntityInfos)
    {
        Entity entity;
        entity.m_index = entityIndex;
        sceneDataStorage.m_entityInfos.emplace_back();
        auto &newInfo = sceneDataStorage.m_entityInfos.back();
        newInfo.m_name = inEntityInfo["Name"].as<std::string>();
        entity.m_version = newInfo.m_version = inEntityInfo["Version"].as<unsigned>();
        newInfo.m_static = inEntityInfo["Static"].as<bool>();
        newInfo.m_enabled = inEntityInfo["Enabled"].as<bool>();
        Entity parent;
        parent.m_index = inEntityInfo["Parent.Index"].as<unsigned>();
        parent.m_version = inEntityInfo["Parent.Version"].as<unsigned>();
        newInfo.m_parent = parent;
        if (inEntityInfo["Children"].IsDefined())
        {
            YAML::Binary childrenData = inEntityInfo["Children"].as<YAML::Binary>();
            const unsigned char *data = childrenData.data();
            std::size_t size = childrenData.size();
            newInfo.m_children.resize(size / sizeof(Entity));
            std::memcpy(newInfo.m_children.data(), data, size);
        }
        newInfo.m_dataComponentStorageIndex = inEntityInfo["DataComponentStorageIndex"].as<size_t>();
        newInfo.m_chunkArrayIndex = inEntityInfo["ChunkArrayIndex"].as<size_t>();
        auto inPrivateComponents = inEntityInfo["PrivateComponent"];
        if (inPrivateComponents)
        {
            for (const auto &inPrivateComponent : inPrivateComponents)
            {
                auto name = inPrivateComponent["TypeName"].as<std::string>();
                size_t hashCode;
                auto *ptr = dynamic_cast<IPrivateComponent *>(SerializableFactory::ProduceSerializable(name, hashCode));
                ptr->m_enabled = inPrivateComponent["Enabled"].as<bool>();
                newInfo.m_privateComponentElements.emplace_back(hashCode, ptr, entity);
                m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                ptr->OnCreate();
                ptr->Deserialize(inPrivateComponent);
            }
        }
        entityIndex++;
    }
#pragma endregion
#pragma region Entities
    auto entitiesData = in["Entities"].as<YAML::Binary>();
    const unsigned char *entitiesDataPtr = entitiesData.data();
    std::size_t entitiesSize = entitiesData.size();
    sceneDataStorage.m_entities.resize(entitiesSize / sizeof(Entity));
    std::memcpy(sceneDataStorage.m_entities.data(), entitiesDataPtr, entitiesSize);
#pragma endregion

#pragma region Systems
    auto inSystems = in["Systems"];
    for (const auto &inSystem : inSystems)
    {
        auto name = inSystem["TypeName"].as<std::string>();
        size_t hashCode;
        auto ptr =
            std::shared_ptr<ISystem>(dynamic_cast<ISystem *>(SerializableFactory::ProduceSerializable(name, hashCode)));
        ptr->m_enabled = inSystem["Enabled"].as<bool>();
        ptr->m_rank = inSystem["Rank"].as<float>();
        m_systems.insert({ptr->m_rank, ptr});
        m_indexedSystems.insert({hashCode, ptr});
        ptr->OnCreate();
        ptr->Deserialize(inSystem);
    }
#pragma endregion
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
void Scene::SerializeEntityInfo(const EntityInfo &entityInfo, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "Name" << YAML::Value << entityInfo.m_name;
        out << YAML::Key << "Version" << YAML::Value << entityInfo.m_version;
        out << YAML::Key << "Static" << YAML::Value << entityInfo.m_static;
        out << YAML::Key << "Enabled" << YAML::Value << entityInfo.m_enabled;
        out << YAML::Key << "Parent.Index" << YAML::Value << entityInfo.m_parent.m_index;
        out << YAML::Key << "Parent.Version" << YAML::Value << entityInfo.m_parent.m_version;
        if (!entityInfo.m_children.empty())
        {
            out << YAML::Key << "Children" << YAML::Value
                << YAML::Binary(
                       (const unsigned char *)entityInfo.m_children.data(),
                       entityInfo.m_children.size() * sizeof(Entity));
        }
        out << YAML::Key << "DataComponentStorageIndex" << YAML::Value << entityInfo.m_dataComponentStorageIndex;
        out << YAML::Key << "ChunkArrayIndex" << YAML::Value << entityInfo.m_chunkArrayIndex;
#pragma region Private Components
        out << YAML::Key << "PrivateComponent" << YAML::Value << YAML::BeginSeq;
        for (const auto &element : entityInfo.m_privateComponentElements)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "TypeName" << YAML::Value << element.m_privateComponentData->GetTypeName();
            out << YAML::Key << "Enabled" << YAML::Value << element.m_privateComponentData->m_enabled;
            element.m_privateComponentData->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
#pragma endregion
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

        system->Serialize(out);
    }
    out << YAML::EndMap;
}
