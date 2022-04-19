#include <Application.hpp>

#include "Engine/ECS/Entities.hpp"
#include <Scene.hpp>

using namespace UniEngine;

void Scene::Purge()
{
    m_sceneDataStorage.m_entityPrivateComponentStorage = PrivateComponentStorage();
    m_sceneDataStorage.m_entities.clear();
    m_sceneDataStorage.m_entityMetadataList.clear();
    for (int index = 1; index < m_sceneDataStorage.m_dataComponentStorages.size(); index++)
    {
        auto &i = m_sceneDataStorage.m_dataComponentStorages[index];
        for (auto &chunk : i.m_chunkArray.m_chunks)
            free(chunk.m_data);
    }
    m_sceneDataStorage.m_dataComponentStorages.clear();

    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityMetadataList.emplace_back();
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
        auto entityInfo = m_sceneDataStorage.m_entityMetadataList[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled)
                continue;
            if (!privateComponentElement.m_privateComponentData->m_started)
            {
                privateComponentElement.m_privateComponentData->Start();
                if (entity.m_version != entityInfo.m_version)
                    break;
                privateComponentElement.m_privateComponentData->m_started = true;
            }
            if (entity.m_version != entityInfo.m_version)
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
        auto entityInfo = m_sceneDataStorage.m_entityMetadataList[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled ||
                !privateComponentElement.m_privateComponentData->m_started)
                continue;
            privateComponentElement.m_privateComponentData->Update();
            if (entity.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled() && i.second->m_started)
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
        auto entityInfo = m_sceneDataStorage.m_entityMetadataList[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled ||
                !privateComponentElement.m_privateComponentData->m_started)
                continue;
            privateComponentElement.m_privateComponentData->LateUpdate();
            if (entity.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled() && i.second->m_started)
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
        auto entityInfo = m_sceneDataStorage.m_entityMetadataList[entity.m_index];
        if (!entityInfo.m_enabled)
            continue;
        for (auto &privateComponentElement : entityInfo.m_privateComponentElements)
        {
            if (!privateComponentElement.m_privateComponentData->m_enabled ||
                !privateComponentElement.m_privateComponentData->m_started)
                continue;
            privateComponentElement.m_privateComponentData->FixedUpdate();
            if (entity.m_version != entityInfo.m_version)
                break;
        }
    }

    for (auto &i : m_systems)
    {
        if (i.second->Enabled() && i.second->m_started)
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
    if (this == Entities::GetCurrentScene().get())
        if (Editor::DragAndDropButton<Camera>(m_mainCamera, "Main Camera", true))
            m_saved = false;
    if (ImGui::CollapsingHeader("Environment Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static int type = (int)m_environmentSettings.m_environmentType;
        if (ImGui::Combo("Environment type", &type, EnvironmentTypes, IM_ARRAYSIZE(EnvironmentTypes)))
        {
            m_environmentSettings.m_environmentType = (EnvironmentType)type;
            m_saved = false;
        }
        switch (m_environmentSettings.m_environmentType)
        {
        case EnvironmentType::EnvironmentalMap: {
            if (Editor::DragAndDropButton<EnvironmentalMap>(
                    m_environmentSettings.m_environmentalMap, "Environmental Map"))
                m_saved = false;
        }
        break;
        case EnvironmentType::Color: {
            if (ImGui::ColorEdit3("Background Color", &m_environmentSettings.m_backgroundColor.x))
                m_saved = false;
        }
        break;
        }
        if (ImGui::DragFloat(
                "Environmental light intensity", &m_environmentSettings.m_ambientLightIntensity, 0.01f, 0.0f, 2.0f))
            m_saved = false;
        if (ImGui::DragFloat("Environmental light gamma", &m_environmentSettings.m_environmentGamma, 0.01f, 0.0f, 2.0f))
        {
            m_saved = false;
        }
    }
}

std::shared_ptr<ISystem> Scene::GetOrCreateSystem(const std::string &systemName, float order)
{
    size_t typeId;
    auto ptr = Serialization::ProduceSerializable(systemName, typeId);
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
    out << YAML::Key << "m_environmentSettings" << YAML::Value << YAML::BeginMap;
    m_environmentSettings.Serialize(out);
    out << YAML::EndMap;
    m_mainCamera.Save("m_mainCamera", out);
    std::unordered_map<Handle, std::shared_ptr<IAsset>> assetMap;
    std::vector<AssetRef> list;
    list.push_back(m_environmentSettings.m_environmentalMap);
    auto &sceneDataStorage = m_sceneDataStorage;
#pragma region EntityInfo
    out << YAML::Key << "m_entityMetadataList" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_entityMetadataList.size(); i++)
    {
        auto &entityMetadata = sceneDataStorage.m_entityMetadataList[i];
        if (entityMetadata.m_handle == 0)
            continue;
        for (const auto &element : entityMetadata.m_privateComponentElements)
        {
            element.m_privateComponentData->CollectAssetRef(list);
        }
        entityMetadata.Serialize(out);
    }
    out << YAML::EndSeq;
#pragma endregion

#pragma region Systems
    out << YAML::Key << "m_systems" << YAML::Value << YAML::BeginSeq;
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
        if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle())
        {
            if (asset->IsTemporary())
            {
                assetMap[asset->GetHandle()] = asset;
            }
            else if (!asset->Saved())
            {
                asset->Save();
            }
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
                if (asset->IsTemporary())
                {
                    assetMap[asset->GetHandle()] = asset;
                }
                else if (!asset->Saved())
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
            out << YAML::Key << "m_typeName" << YAML::Value << i.second->GetTypeName();
            out << YAML::Key << "m_handle" << YAML::Value << i.second->GetHandle();
            i.second->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
#pragma endregion

#pragma region DataComponentStorage
    out << YAML::Key << "m_dataComponentStorages" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_dataComponentStorages.size(); i++)
    {
        SerializeDataComponentStorage(sceneDataStorage.m_dataComponentStorages[i], out);
    }
    out << YAML::EndSeq;
#pragma endregion
    out << YAML::EndMap;
}
void Scene::Deserialize(const YAML::Node &in)
{
    UNIENGINE_LOG("Loading scene...");
    m_sceneDataStorage.m_entities.clear();
    m_sceneDataStorage.m_entityMetadataList.clear();
    m_sceneDataStorage.m_dataComponentStorages.clear();
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityMetadataList.emplace_back();
    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
#pragma region EntityMetadata
    auto inEntityMetadataList = in["m_entityMetadataList"];
    int currentIndex = 1;
    for (const auto &inEntityMetadata : inEntityMetadataList)
    {
        m_sceneDataStorage.m_entityMetadataList.emplace_back();
        auto &newInfo = m_sceneDataStorage.m_entityMetadataList.back();
        newInfo.Deserialize(inEntityMetadata);
        Entity entity;
        entity.m_version = 1;
        entity.m_index = currentIndex;
        m_sceneDataStorage.m_entityMap[newInfo.GetHandle()] = entity;
        m_sceneDataStorage.m_entities.push_back(entity);
        currentIndex++;
    }
    currentIndex = 1;
    for (const auto &inEntityMetadata : inEntityMetadataList)
    {
        auto &metadata = m_sceneDataStorage.m_entityMetadataList[currentIndex];
        if (inEntityMetadata["Parent.Handle"])
        {
            metadata.m_parent =
                m_sceneDataStorage.m_entityMap[Handle(inEntityMetadata["Parent.Handle"].as<uint64_t>())];
            auto &parentMetadata = m_sceneDataStorage.m_entityMetadataList[metadata.m_parent.m_index];
            Entity entity;
            entity.m_version = 1;
            entity.m_index = currentIndex;
            parentMetadata.m_children.push_back(entity);
        }
        if (inEntityMetadata["Root.Handle"])
            metadata.m_root = m_sceneDataStorage.m_entityMap[Handle(inEntityMetadata["Root.Handle"].as<uint64_t>())];
        currentIndex++;
    }
#pragma endregion

#pragma region DataComponentStorage
    auto inDataComponentStorages = in["m_dataComponentStorages"];
    int storageIndex = 1;
    for (const auto &inDataComponentStorage : inDataComponentStorages)
    {
        m_sceneDataStorage.m_dataComponentStorages.emplace_back();
        auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages.back();
        dataComponentStorage.m_entitySize = inDataComponentStorage["m_entitySize"].as<size_t>();
        dataComponentStorage.m_chunkCapacity = inDataComponentStorage["m_chunkCapacity"].as<size_t>();
        dataComponentStorage.m_entityAliveCount = dataComponentStorage.m_entityCount =
            inDataComponentStorage["m_entityAliveCount"].as<size_t>();
        dataComponentStorage.m_chunkArray.m_entities.resize(dataComponentStorage.m_entityAliveCount);
        const size_t chunkSize = dataComponentStorage.m_entityCount / dataComponentStorage.m_chunkCapacity + 1;
        while (dataComponentStorage.m_chunkArray.m_chunks.size() <= chunkSize)
        {
            // Allocate new chunk;
            ComponentDataChunk chunk = {};
            chunk.m_data = static_cast<void *>(calloc(1, Entities::GetInstance().m_archetypeChunkSize));
            dataComponentStorage.m_chunkArray.m_chunks.push_back(chunk);
        }
        auto inDataComponentTypes = inDataComponentStorage["m_dataComponentTypes"];
        for (const auto &inDataComponentType : inDataComponentTypes)
        {
            DataComponentType dataComponentType;
            dataComponentType.m_name = inDataComponentType["m_name"].as<std::string>();
            dataComponentType.m_size = inDataComponentType["m_size"].as<size_t>();
            dataComponentType.m_offset = inDataComponentType["m_offset"].as<size_t>();
            dataComponentType.m_typeId = Serialization::GetDataComponentTypeId(dataComponentType.m_name);
            dataComponentStorage.m_dataComponentTypes.push_back(dataComponentType);
        }
        auto inDataChunkArray = inDataComponentStorage["m_chunkArray"];
        int chunkArrayIndex = 0;
        for (const auto &entityDataComponent : inDataChunkArray)
        {
            Handle handle = entityDataComponent["m_handle"].as<uint64_t>();
            Entity entity = m_sceneDataStorage.m_entityMap[handle];
            dataComponentStorage.m_chunkArray.m_entities[chunkArrayIndex] = entity;
            auto &metadata = m_sceneDataStorage.m_entityMetadataList[entity.m_index];
            metadata.m_dataComponentStorageIndex = storageIndex;
            metadata.m_chunkArrayIndex = chunkArrayIndex;
            const auto chunkIndex = metadata.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
            const auto chunkPointer = metadata.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
            const auto chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];

            int typeIndex = 0;
            for (const auto &inDataComponent : entityDataComponent["DataComponents"])
            {
                auto &type = dataComponentStorage.m_dataComponentTypes[typeIndex];
                auto data = inDataComponent["Data"].as<YAML::Binary>();
                std::memcpy(
                    chunk.GetDataPointer(static_cast<size_t>(
                        type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * type.m_size)),
                    data.data(),
                    data.size());
                typeIndex++;
            }

            chunkArrayIndex++;
        }
        storageIndex++;
    }
    auto self = std::dynamic_pointer_cast<Scene>(m_self.lock());
#pragma endregion
    m_mainCamera.Load("m_mainCamera", in, self);
#pragma region Assets
    std::vector<std::shared_ptr<IAsset>> localAssets;
    auto inLocalAssets = in["LocalAssets"];
    if (inLocalAssets)
    {
        for (const auto &i : inLocalAssets)
        {
            // First, find the asset in assetregistry
            auto asset = ProjectManager::CreateTemporaryAsset(i["m_typeName"].as<std::string>(), i["m_handle"].as<uint64_t>());
            localAssets.push_back(asset);
        }
        int index = 0;
        for (const auto &i : inLocalAssets)
        {
            localAssets[index++]->Deserialize(i);
        }
    }

#pragma endregion
    if (in["m_environmentSettings"])
        m_environmentSettings.Deserialize(in["m_environmentSettings"]);
    int entityIndex = 1;
    for (const auto &inEntityInfo : inEntityMetadataList)
    {
        auto &entityMetadata = m_sceneDataStorage.m_entityMetadataList.at(entityIndex);
        auto entity = m_sceneDataStorage.m_entities[entityIndex];
        auto inPrivateComponents = inEntityInfo["m_privateComponentElements"];
        if (inPrivateComponents)
        {
            for (const auto &inPrivateComponent : inPrivateComponents)
            {
                auto name = inPrivateComponent["m_typeName"].as<std::string>();
                size_t hashCode;
                if (Serialization::HasSerializableType(name))
                {
                    auto ptr =
                        std::static_pointer_cast<IPrivateComponent>(Serialization::ProduceSerializable(name, hashCode));
                    ptr->m_enabled = inPrivateComponent["m_enabled"].as<bool>();
                    ptr->m_started = false;
                    m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                    entityMetadata.m_privateComponentElements.emplace_back(hashCode, ptr, entity, self);
                }
                else
                {
                    auto ptr = std::static_pointer_cast<IPrivateComponent>(
                        Serialization::ProduceSerializable("UnknownPrivateComponent", hashCode));
                    ptr->m_enabled = false;
                    ptr->m_started = false;
                    m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                    entityMetadata.m_privateComponentElements.emplace_back(hashCode, ptr, entity, self);
                }
            }
        }
        entityIndex++;
    }

#pragma region Systems
    auto inSystems = in["m_systems"];
    std::vector<std::shared_ptr<ISystem>> systems;

    for (const auto &inSystem : inSystems)
    {
        auto name = inSystem["m_typeName"].as<std::string>();
        size_t hashCode;
        auto ptr = std::static_pointer_cast<ISystem>(Serialization::ProduceSerializable(name, hashCode));
        ptr->m_handle = Handle(inSystem["m_handle"].as<uint64_t>());
        ptr->m_enabled = inSystem["m_enabled"].as<bool>();
        ptr->m_rank = inSystem["m_rank"].as<float>();
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
    for (const auto &inEntityInfo : inEntityMetadataList)
    {
        auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entityIndex);
        auto inPrivateComponents = inEntityInfo["m_privateComponentElements"];
        int componentIndex = 0;
        if (inPrivateComponents)
        {
            for (const auto &inPrivateComponent : inPrivateComponents)
            {
                auto name = inPrivateComponent["m_typeName"].as<std::string>();
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
        auto name = inSystem["m_typeName"].as<std::string>();
        systems[systemIndex]->Deserialize(inSystem);
        systemIndex++;
    }
}
void Scene::SerializeDataComponentStorage(const DataComponentStorage &storage, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "m_entitySize" << YAML::Value << storage.m_entitySize;
        out << YAML::Key << "m_chunkCapacity" << YAML::Value << storage.m_chunkCapacity;
        out << YAML::Key << "m_entityAliveCount" << YAML::Value << storage.m_entityAliveCount;
        out << YAML::Key << "m_dataComponentTypes" << YAML::Value << YAML::BeginSeq;
        for (const auto &i : storage.m_dataComponentTypes)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "m_name" << YAML::Value << i.m_name;
            out << YAML::Key << "m_size" << YAML::Value << i.m_size;
            out << YAML::Key << "m_offset" << YAML::Value << i.m_offset;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "m_chunkArray" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < storage.m_entityAliveCount; i++)
        {
            auto entity = storage.m_chunkArray.m_entities[i];
            if (entity.m_version == 0)
                continue;

            out << YAML::BeginMap;
            auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
            out << YAML::Key << "m_handle" << YAML::Value << entityInfo.m_handle;

            auto &dataComponentStorage =
                m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
            const auto chunkIndex = entityInfo.m_chunkArrayIndex / dataComponentStorage.m_chunkCapacity;
            const auto chunkPointer = entityInfo.m_chunkArrayIndex % dataComponentStorage.m_chunkCapacity;
            const auto chunk = dataComponentStorage.m_chunkArray.m_chunks[chunkIndex];

            out << YAML::Key << "DataComponents" << YAML::Value << YAML::BeginSeq;
            for (const auto &type : dataComponentStorage.m_dataComponentTypes)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Data" << YAML::Value
                    << YAML::Binary(
                           (const unsigned char *)chunk.GetDataPointer(static_cast<size_t>(
                               type.m_offset * dataComponentStorage.m_chunkCapacity + chunkPointer * type.m_size)),
                           type.m_size);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
    out << YAML::EndMap;
}
void Scene::SerializeSystem(const std::shared_ptr<ISystem> &system, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "m_typeName" << YAML::Value << system->GetTypeName();
        out << YAML::Key << "m_enabled" << YAML::Value << system->m_enabled;
        out << YAML::Key << "m_rank" << YAML::Value << system->m_rank;
        out << YAML::Key << "m_handle" << YAML::Value << system->GetHandle();
        system->Serialize(out);
    }
    out << YAML::EndMap;
}
void Scene::OnCreate()
{
    m_sceneDataStorage.m_entities.emplace_back();
    m_sceneDataStorage.m_entityMetadataList.emplace_back();
    m_sceneDataStorage.m_dataComponentStorages.emplace_back();
    m_environmentSettings.m_environmentalMap = DefaultResources::Environmental::DefaultEnvironmentalMap;
}
bool Scene::LoadInternal(const std::filesystem::path &path)
{
    auto previousScene = Entities::GetCurrentScene();
    Entities::Attach(std::shared_ptr<Scene>(this, [](Scene *) {}));
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    Deserialize(in);
    Entities::Attach(previousScene);

    return true;
}
void Scene::Clone(const std::shared_ptr<Scene> &source, const std::shared_ptr<Scene> &newScene)
{
    newScene->m_environmentSettings = source->m_environmentSettings;
    newScene->m_saved = source->m_saved;
    newScene->m_worldBound = source->m_worldBound;
    std::unordered_map<Handle, Handle> entityMap;

    newScene->m_sceneDataStorage.Clone(entityMap, source->m_sceneDataStorage, newScene);
    for (const auto &i : source->m_systems)
    {
        auto systemName = i.second->GetTypeName();
        size_t hashCode;
        auto system = std::dynamic_pointer_cast<ISystem>(
            Serialization::ProduceSerializable(systemName, hashCode, i.second->GetHandle()));
        newScene->m_systems.insert({i.first, system});
        newScene->m_indexedSystems[hashCode] = system;
        newScene->m_mappedSystems[i.second->GetHandle()] = system;
        system->m_scene = source;
        system->OnCreate();
        Serialization::CloneSystem(system, i.second);
    }
    newScene->m_mainCamera.m_entityHandle = source->m_mainCamera.m_entityHandle;
    newScene->m_mainCamera.m_privateComponentTypeName = source->m_mainCamera.m_privateComponentTypeName;
    newScene->m_mainCamera.Relink(entityMap, newScene);
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
void SceneDataStorage::Clone(std::unordered_map<Handle, Handle> &entityMap, const SceneDataStorage &source, const std::shared_ptr<Scene> &newScene)
{
    m_entities = source.m_entities;
    m_entityMetadataList.resize(source.m_entityMetadataList.size());

    for (const auto &i : source.m_entityMetadataList)
    {
        entityMap.insert({i.GetHandle(), i.GetHandle()});
    }

    for (int i = 0; i < m_entityMetadataList.size(); i++)
        m_entityMetadataList[i].Clone(entityMap, source.m_entityMetadataList[i], newScene);
    m_dataComponentStorages.resize(source.m_dataComponentStorages.size());
    for (int i = 0; i < m_dataComponentStorages.size(); i++)
        m_dataComponentStorages[i] = source.m_dataComponentStorages[i];
    m_entityMap = source.m_entityMap;
    m_entityPrivateComponentStorage = source.m_entityPrivateComponentStorage;
}
