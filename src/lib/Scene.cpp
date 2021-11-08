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
}

void Scene::Start()
{
    auto entities = m_sceneDataStorage.m_entities;
    for (const auto &entity : entities)
    {
        if (entity.m_version == 0)
            continue;
        auto entityInfo = m_sceneDataStorage.m_entityInfos[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            if (!privateComponentElement.m_privateComponentData->m_started)
            {
                privateComponentElement.m_privateComponentData->Start();
                if (entityInfo.m_version != entityInfo.m_version)
                    break;
                privateComponentElement.m_privateComponentData->m_started = true;
            }
            if (entityInfo.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerLayer::StartEvent(i.second->GetTypeName());
            if (!i.second->m_started)
            {
                i.second->Start();
                i.second->m_started = true;
            }
            ProfilerLayer::EndEvent(i.second->GetTypeName());
        }
    }
}

void Scene::Update()
{
    auto entities = m_sceneDataStorage.m_entities;
    for (const auto &entity : entities)
    {
        if (entity.m_version == 0)
            continue;
        auto entityInfo = m_sceneDataStorage.m_entityInfos[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->Update();
            if (entityInfo.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerLayer::StartEvent(i.second->GetTypeName());
            i.second->Update();
            ProfilerLayer::EndEvent(i.second->GetTypeName());
        }
    }
}

void Scene::LateUpdate()
{
    auto entities = m_sceneDataStorage.m_entities;
    for (const auto &entity : entities)
    {
        if (entity.m_version == 0)
            continue;
        auto entityInfo = m_sceneDataStorage.m_entityInfos[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->LateUpdate();
            if (entityInfo.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerLayer::StartEvent(i.second->GetTypeName());
            i.second->LateUpdate();
            ProfilerLayer::EndEvent(i.second->GetTypeName());
        }
    }
}
void Scene::FixedUpdate()
{
    auto entities = m_sceneDataStorage.m_entities;
    for (const auto &entity : entities)
    {
        if (entity.m_version == 0)
            continue;
        auto entityInfo = m_sceneDataStorage.m_entityInfos[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            privateComponentElement.m_privateComponentData->FixedUpdate();
            if (entityInfo.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled())
        {
            ProfilerLayer::StartEvent(i.second->GetTypeName());
            i.second->FixedUpdate();
            ProfilerLayer::EndEvent(i.second->GetTypeName());
        }
    }
}
static const char *EnvironmentTypes[]{"Environmental Map", "Color"};
void Scene::OnInspect()
{
    if (this == EntityManager::GetCurrentScene().get())
        EditorManager::DragAndDropButton<Camera>(m_mainCamera, "Main Camera", true);
    if (ImGui::CollapsingHeader("Environment Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static int type = (int)m_environmentSettings.m_environmentType;
        if (ImGui::Combo("Environment type", &type, EnvironmentTypes, IM_ARRAYSIZE(EnvironmentTypes)))
        {
            m_environmentSettings.m_environmentType = (EnvironmentType)type;
        }
        switch (m_environmentSettings.m_environmentType)
        {
        case EnvironmentType::EnvironmentalMap: {
            EditorManager::DragAndDropButton<EnvironmentalMap>(
                m_environmentSettings.m_environmentalMap, "Environmental Map");
        }
        break;
        case EnvironmentType::Color: {
            ImGui::ColorEdit3("Background Color", &m_environmentSettings.m_backgroundColor.x);
        }
        break;
        }
        ImGui::DragFloat(
            "Environmental light intensity", &m_environmentSettings.m_ambientLightIntensity, 0.01f, 0.0f, 2.0f);
        ImGui::DragFloat("Environmental light gamma", &m_environmentSettings.m_environmentGamma, 0.01f, 0.0f, 2.0f);
    }
}

std::shared_ptr<ISystem> Scene::GetOrCreateSystem(const std::string &systemName, float order)
{
    size_t typeId;
    auto ptr = SerializationManager::ProduceSerializable(systemName, typeId);
    auto system = std::dynamic_pointer_cast<ISystem>(ptr);
    system->m_handle = Handle();
    system->m_rank = order;
    m_systems.insert({order, system});
    m_indexedSystems[typeId] = system;
    m_mappedSystems[system->m_handle] = system;
    system->m_started = false;
    system->OnCreate();
    m_saved = false;
    return std::dynamic_pointer_cast<ISystem>(ptr);
}

void Scene::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "Scene" << YAML::Value << m_name;
    out << YAML::Key << "m_environmentSettings" << YAML::Value << YAML::BeginMap;
    m_environmentSettings.Serialize(out);
    out << YAML::EndMap;
    m_mainCamera.Save("m_mainCamera", out);
    std::unordered_map<Handle, std::shared_ptr<IAsset>> assetMap;
    std::vector<AssetRef> list;
    list.push_back(m_environmentSettings.m_environmentalMap);
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
        i.second->CollectAssetRef(list);
    }
    out << YAML::EndSeq;
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
            if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle())
            {
                if (asset->GetPath().empty())
                {
                    assetMap[asset->GetHandle()] = asset;
                }
                else if (!asset->m_saved)
                {
                    asset->Save();
                }
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
    auto self = AssetManager::Get<Scene>(m_handle);
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
    m_mainCamera.Load("m_mainCamera", in, self);
#pragma region Assets
    std::vector<std::shared_ptr<IAsset>> localAssets;
    auto inLocalAssets = in["LocalAssets"];
    if (inLocalAssets)
    {
        std::vector<bool> isLocal;
        for (const auto &i : inLocalAssets)
        {
            Handle handle = i["Handle"].as<uint64_t>();
            // First, find the asset in assetregistry
            auto asset = AssetManager::Get(handle);
            if (!asset)
            {
                asset = AssetManager::UnsafeCreateAsset(
                    i["TypeName"].as<std::string>(), handle, i["Name"].as<std::string>());
                isLocal.push_back(false);
            }
            else
            {
                isLocal.push_back(true);
            }
            localAssets.push_back(asset);
        }
        int index = 0;
        for (const auto &i : inLocalAssets)
        {
            if (!isLocal[index])
                localAssets[index++]->Deserialize(i);
        }
    }

#pragma endregion
    if (in["m_environmentSettings"])
        m_environmentSettings.Deserialize(in["m_environmentSettings"]);
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
                if (SerializationManager::HasSerializableType(name))
                {
                    auto ptr = std::static_pointer_cast<IPrivateComponent>(
                        SerializationManager::ProduceSerializable(name, hashCode));
                    ptr->m_enabled = inPrivateComponent["Enabled"].as<bool>();
                    ptr->m_started = false;
                    m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                    entityInfo.m_privateComponentElements.emplace_back(hashCode, ptr, entity, self);
                }
                else
                {
                    auto ptr = std::static_pointer_cast<IPrivateComponent>(
                        SerializationManager::ProduceSerializable("UnknownPrivateComponent", hashCode));
                    ptr->m_enabled = inPrivateComponent["Enabled"].as<bool>();
                    ptr->m_started = false;
                    m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                    entityInfo.m_privateComponentElements.emplace_back(hashCode, ptr, entity, self);
                }
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
        ptr->m_scene = self;
        ptr->OnCreate();
    }
#pragma endregion

    entityIndex = 1;
    for (const auto &inEntityInfo : inEntityInfos)
    {
        auto &entityInfo = sceneDataStorage.m_entityInfos.at(entityIndex);
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
    m_environmentSettings.m_environmentalMap = DefaultResources::Environmental::DefaultEnvironmentalMap;
}
bool Scene::LoadInternal(const std::filesystem::path &path)
{
    auto previousScene = EntityManager::GetCurrentScene();
    EntityManager::Attach(std::shared_ptr<Scene>(this, [](Scene *) {}));
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    m_name = in["m_name"].as<std::string>();
    Deserialize(in);
    EntityManager::Attach(previousScene);

    return true;
}
void Scene::Clone(const std::shared_ptr<Scene> &source, const std::shared_ptr<Scene> &newScene)
{
    newScene->m_name = source->m_name;
    newScene->m_environmentSettings = source->m_environmentSettings;
    newScene->m_projectRelativePath = source->m_projectRelativePath;
    newScene->m_saved = source->m_saved;
    newScene->m_worldBound = source->m_worldBound;
    newScene->m_sceneDataStorage.Clone(source->m_sceneDataStorage, newScene);
    newScene->m_projectRelativePath.clear();
    for (const auto &i : source->m_systems)
    {
        auto systemName = i.second->GetTypeName();
        size_t hashCode;
        auto system = std::dynamic_pointer_cast<ISystem>(
            SerializationManager::ProduceSerializable(systemName, hashCode, i.second->GetHandle()));
        newScene->m_systems.insert({i.first, system});
        newScene->m_indexedSystems[hashCode] = system;
        newScene->m_mappedSystems[i.second->GetHandle()] = system;
        system->m_scene = source;
        system->OnCreate();
        SerializationManager::CloneSystem(system, i.second);
    }

    auto mainCamera = source->m_mainCamera.Get();
    if(mainCamera)
    {
        newScene->m_mainCamera =
            EntityManager::GetOrSetPrivateComponent<Camera>(AssetManager::Get<Scene>(newScene->m_handle), mainCamera->GetOwner())
                .lock();
    }
}

void EnvironmentSettings::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_backgroundColor" << YAML::Value << m_backgroundColor;
    out << YAML::Key << "m_environmentGamma" << YAML::Value << m_environmentGamma;
    out << YAML::Key << "m_ambientLightIntensity" << YAML::Value << m_ambientLightIntensity;
    out << YAML::Key << "m_environmentType" << YAML::Value << (unsigned)m_environmentType;
    m_environmentalMap.Save("m_environmentSettings", out);
}
void EnvironmentSettings::Deserialize(const YAML::Node &in)
{
    if (in["m_backgroundColor"])
        m_backgroundColor = in["m_backgroundColor"].as<glm::vec3>();
    if (in["m_environmentGamma"])
        m_environmentGamma = in["m_environmentGamma"].as<float>();
    if (in["m_ambientLightIntensity"])
        m_ambientLightIntensity = in["m_ambientLightIntensity"].as<float>();
    if (in["m_environmentType"])
        m_environmentType = (EnvironmentType)in["m_environmentType"].as<unsigned>();
    m_environmentalMap.Load("m_environmentSettings", in);
}
void SceneDataStorage::Clone(const SceneDataStorage &source, const std::shared_ptr<Scene> &newScene)
{
    m_entities = source.m_entities;
    m_entityInfos.resize(source.m_entityInfos.size());

    std::unordered_map<Handle, Handle> entityMap;
    for(const auto& i : source.m_entityInfos){
        entityMap.insert({i.GetHandle(), i.GetHandle()});
    }

    for (int i = 0; i < m_entityInfos.size(); i++)
        m_entityInfos[i].Clone(entityMap, source.m_entityInfos[i], newScene);
    m_dataComponentStorages.resize(source.m_dataComponentStorages.size());
    for (int i = 0; i < m_dataComponentStorages.size(); i++)
        m_dataComponentStorages[i] = source.m_dataComponentStorages[i];
    m_entityMap = source.m_entityMap;
    m_entityPrivateComponentStorage = source.m_entityPrivateComponentStorage;
}
