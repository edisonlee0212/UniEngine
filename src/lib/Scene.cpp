#include "Scene.hpp"
#include "Application.hpp"
#include "Camera.hpp"
#include "DefaultResources.hpp"
#include "Editor.hpp"
#include "Entities.hpp"
#include "EntityMetadata.hpp"
#include "EnvironmentalMap.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;
AssetRegistration<Scene> SceneReg("Scene", {".uescene"});
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
    if (this == Application::GetActiveScene().get())
        if (Editor::DragAndDropButton<Camera>(m_mainCamera, "Main Camera", true))
            m_saved = false;
    if (ImGui::TreeNodeEx("Environment Settings", ImGuiTreeNodeFlags_DefaultOpen))
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
                "Environmental light intensity", &m_environmentSettings.m_ambientLightIntensity, 0.01f, 0.0f, 10.0f))
            m_saved = false;
        if (ImGui::DragFloat("Environmental light gamma", &m_environmentSettings.m_environmentGamma, 0.01f, 0.0f, 10.0f))
        {
            m_saved = false;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Systems"))
    {
        if (ImGui::BeginPopupContextWindow("SystemInspectorPopup"))
        {
            ImGui::Text("Add system: ");
            ImGui::Separator();
            static float rank = 0.0f;
            ImGui::DragFloat("Rank", &rank, 1.0f, 0.0f, 999.0f);
            for (auto &i : Editor::GetInstance().m_systemMenuList)
            {
                i.second(rank);
            }
            ImGui::Separator();
            ImGui::EndPopup();
        }
        for (auto &i : Application::GetActiveScene()->m_systems)
        {
            if (ImGui::CollapsingHeader(i.second->GetTypeName().c_str()))
            {
                bool enabled = i.second->Enabled();
                if (ImGui::Checkbox("Enabled", &enabled))
                {
                    if (i.second->Enabled() != enabled)
                    {
                        if (enabled)
                        {
                            i.second->Enable();
                        }
                        else
                        {
                            i.second->Disable();
                        }
                    }
                }
                i.second->OnInspect();
            }
        }
        ImGui::TreePop();
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
        entityMetadata.Serialize(out, std::dynamic_pointer_cast<Scene>(m_self.lock()));
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
    auto scene = std::dynamic_pointer_cast<Scene>(m_self.lock());
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
        newInfo.Deserialize(inEntityMetadata, scene);
        Entity entity;
        entity.m_version = 1;
        entity.m_index = currentIndex;
        m_sceneDataStorage.m_entityMap[newInfo.m_handle] = entity;
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
            chunk.m_data = static_cast<void *>(calloc(1, Entities::GetArchetypeChunkSize()));
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
            auto asset =
                ProjectManager::CreateTemporaryAsset(i["m_typeName"].as<std::string>(), i["m_handle"].as<uint64_t>());
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
    m_sceneDataStorage.m_entityPrivateComponentStorage.m_scene = std::dynamic_pointer_cast<Scene>(m_self.lock());
}
bool Scene::LoadInternal(const std::filesystem::path &path)
{
    auto previousScene = Application::GetActiveScene();
    Application::Attach(std::shared_ptr<Scene>(this, [](Scene *) {}));
    std::ifstream stream(path.string());
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    Deserialize(in);
    Application::Attach(previousScene);

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
        system->m_scene = newScene;
        system->OnCreate();
        Serialization::CloneSystem(system, i.second);
        system->m_scene = newScene;
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
void SceneDataStorage::Clone(
    std::unordered_map<Handle, Handle> &entityMap,
    const SceneDataStorage &source,
    const std::shared_ptr<Scene> &newScene)
{
    m_entities = source.m_entities;
    m_entityMetadataList.resize(source.m_entityMetadataList.size());

    for (const auto &i : source.m_entityMetadataList)
    {
        entityMap.insert({i.m_handle, i.m_handle});
    }
    m_dataComponentStorages.resize(source.m_dataComponentStorages.size());
    for (int i = 0; i < m_dataComponentStorages.size(); i++)
        m_dataComponentStorages[i] = source.m_dataComponentStorages[i];
    for (int i = 0; i < m_entityMetadataList.size(); i++)
        m_entityMetadataList[i].Clone(entityMap, source.m_entityMetadataList[i], newScene);

    m_entityMap = source.m_entityMap;
    m_entityPrivateComponentStorage = source.m_entityPrivateComponentStorage;
    m_entityPrivateComponentStorage.m_scene = newScene;
}

#pragma region Entity Management
void Scene::UnsafeForEachDataComponent(
    const Entity &entity, const std::function<void(const DataComponentType &type, void *data)> &func)
{
    assert(IsEntityValid(entity));
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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

void Scene::ForEachPrivateComponent(
    const Entity &entity, const std::function<void(PrivateComponentElement &data)> &func)
{
    assert(IsEntityValid(entity));
    auto elements = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
    for (auto &component : elements)
    {
        func(component);
    }
}

void Scene::UnsafeForEachEntityStorage(
    const std::function<void(int i, const std::string &name, const DataComponentStorage &storage)> &func)
{
    auto &archetypeInfos = Entities::GetInstance().m_entityArchetypeInfos;
    for (int i = 0; i < archetypeInfos.size(); i++)
    {
        auto dcs = GetDataComponentStorage(i);
        if (!dcs.has_value())
            continue;
        func(i, archetypeInfos[i].m_name, dcs->first.get());
    }
}

void Scene::DeleteEntityInternal(unsigned entityIndex)
{
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entityIndex);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
    Entity actualEntity = m_sceneDataStorage.m_entities.at(entityIndex);

    m_sceneDataStorage.m_entityPrivateComponentStorage.DeleteEntity(actualEntity);
    entityInfo.m_version = actualEntity.m_version + 1;
    entityInfo.m_enabled = true;

    m_sceneDataStorage.m_entityMap.erase(entityInfo.m_handle);
    entityInfo.m_handle = Handle(0);

    entityInfo.m_privateComponentElements.clear();
    // Set to version 0, marks it as deleted.
    actualEntity.m_version = 0;
    dataComponentStorage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = actualEntity;
    const auto originalIndex = entityInfo.m_chunkArrayIndex;
    if (entityInfo.m_chunkArrayIndex != dataComponentStorage.m_entityAliveCount - 1)
    {
        const auto swappedIndex =
            SwapEntity(dataComponentStorage, entityInfo.m_chunkArrayIndex, dataComponentStorage.m_entityAliveCount - 1);
        entityInfo.m_chunkArrayIndex = dataComponentStorage.m_entityAliveCount - 1;
        m_sceneDataStorage.m_entityMetadataList.at(swappedIndex).m_chunkArrayIndex = originalIndex;
    }
    dataComponentStorage.m_entityAliveCount--;

    m_sceneDataStorage.m_entities.at(entityIndex) = actualEntity;
}

std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> Scene::GetDataComponentStorage(
    unsigned entityArchetypeIndex)
{
    auto &archetypeInfo = Entities::GetInstance().m_entityArchetypeInfos.at(entityArchetypeIndex);
    int targetIndex = 0;
    for (auto &i : m_sceneDataStorage.m_dataComponentStorages)
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
    m_sceneDataStorage.m_dataComponentStorages.emplace_back(archetypeInfo);
    return {
        {std::ref(m_sceneDataStorage.m_dataComponentStorages.back()),
         m_sceneDataStorage.m_dataComponentStorages.size() - 1}};
}

std::optional<std::pair<std::reference_wrapper<DataComponentStorage>, unsigned>> Scene::GetDataComponentStorage(
    const EntityArchetype &entityArchetype)
{
    return GetDataComponentStorage(entityArchetype.m_index);
}

std::vector<std::reference_wrapper<DataComponentStorage>> Scene::QueryDataComponentStorages(
    const EntityQuery &entityQuery)
{
    return QueryDataComponentStorages(entityQuery.m_index);
}

void Scene::GetEntityStorage(const DataComponentStorage &storage, std::vector<Entity> &container, bool checkEnable)
{
    const size_t amount = storage.m_entityAliveCount;
    if (amount == 0)
        return;
    if (checkEnable)
    {
        auto &workers = Jobs::Workers();
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
                                          if (!m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
                                              continue;
                                          tempStorage[threadIndex].push_back(entity);
                                      }
                                      if (threadIndex < loadReminder)
                                      {
                                          const int i = threadIndex + threadSize * threadLoad;
                                          const auto chunkIndex = i / capacity;
                                          const auto remainder = i % capacity;
                                          const auto entity = entities->at(i);
                                          if (!m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled)
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

size_t Scene::SwapEntity(DataComponentStorage &storage, size_t index1, size_t index2)
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

void Scene::GetAllEntities(std::vector<Entity> &target)
{
    target.insert(target.end(), m_sceneDataStorage.m_entities.begin() + 1, m_sceneDataStorage.m_entities.end());
}

void Scene::ForEachDescendant(
    const Entity &target, const std::function<void(const Entity &entity)> &func, const bool &fromRoot)
{
    Entity realTarget = target;
    if (!IsEntityValid(realTarget))
        return;
    if (fromRoot)
        realTarget = GetRoot(realTarget);
    ForEachDescendantHelper(realTarget, func);
}

const std::vector<Entity> &Scene::UnsafeGetAllEntities()
{
    return m_sceneDataStorage.m_entities;
}

Entity Scene::CreateEntity(const std::string &name)
{
    return CreateEntity(Entities::GetInstance().m_basicArchetype, name);
}

Entity Scene::CreateEntity(const EntityArchetype &archetype, const std::string &name, const Handle &handle)
{
    assert(archetype.IsValid());
    m_saved = false;
    Entity retVal;
    auto search = GetDataComponentStorage(archetype);
    DataComponentStorage &storage = search->first;
    if (storage.m_entityCount == storage.m_entityAliveCount)
    {
        const size_t chunkIndex = storage.m_entityCount / storage.m_chunkCapacity + 1;
        if (storage.m_chunkArray.m_chunks.size() <= chunkIndex)
        {
            // Allocate new chunk;
            ComponentDataChunk chunk;
            chunk.m_data = static_cast<void *>(calloc(1, Entities::GetArchetypeChunkSize()));
            storage.m_chunkArray.m_chunks.push_back(chunk);
        }
        retVal.m_index = m_sceneDataStorage.m_entities.size();
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

        m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = retVal;
        m_sceneDataStorage.m_entityMetadataList.push_back(std::move(entityInfo));
        m_sceneDataStorage.m_entities.push_back(retVal);
        storage.m_entityCount++;
        storage.m_entityAliveCount++;
    }
    else
    {
        retVal = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(retVal.m_index);
        entityInfo.m_root = retVal;
        entityInfo.m_static = false;
        entityInfo.m_handle = handle;
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        retVal.m_version = entityInfo.m_version;

        m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = retVal;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = retVal;
        m_sceneDataStorage.m_entities.at(retVal.m_index) = retVal;
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
    SetDataComponent(retVal, Transform());
    SetDataComponent(retVal, GlobalTransform());
    SetDataComponent(retVal, GlobalTransformUpdateFlag());
    return retVal;
}

std::vector<Entity> Scene::CreateEntities(
    const EntityArchetype &archetype, const size_t &amount, const std::string &name)
{
    assert(archetype.IsValid());
    std::vector<Entity> retVal;
    m_saved = false;
    auto search = GetDataComponentStorage(archetype);
    DataComponentStorage &storage = search->first;
    auto remainAmount = amount;
    const Transform transform;
    const GlobalTransform globalTransform;
    const GlobalTransformUpdateFlag transformStatus;
    while (remainAmount > 0 && storage.m_entityAliveCount != storage.m_entityCount)
    {
        remainAmount--;
        Entity entity = storage.m_chunkArray.m_entities.at(storage.m_entityAliveCount);
        EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
        entityInfo.m_root = entity;
        entityInfo.m_static = false;
        entityInfo.m_enabled = true;
        entityInfo.m_name = name;
        entity.m_version = entityInfo.m_version;
        entityInfo.m_handle = Handle();
        m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = entity;
        storage.m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
        m_sceneDataStorage.m_entities.at(entity.m_index) = entity;
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
        SetDataComponent(entity, transform);
        SetDataComponent(entity, globalTransform);
        SetDataComponent(entity, GlobalTransformUpdateFlag());
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
        chunk.m_data = static_cast<void *>(calloc(1, Entities::GetArchetypeChunkSize()));
        storage.m_chunkArray.m_chunks.push_back(chunk);
    }
    const size_t originalSize = m_sceneDataStorage.m_entities.size();
    m_sceneDataStorage.m_entities.resize(originalSize + remainAmount);
    m_sceneDataStorage.m_entityMetadataList.resize(originalSize + remainAmount);

    for (int i = 0; i < remainAmount; i++)
    {
        auto &entity = m_sceneDataStorage.m_entities.at(originalSize + i);
        entity.m_index = originalSize + i;
        entity.m_version = 1;

        auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(originalSize + i);
        entityInfo = EntityMetadata();
        entityInfo.m_root = entity;
        entityInfo.m_static = false;
        entityInfo.m_name = name;
        entityInfo.m_dataComponentStorageIndex = search->second;
        entityInfo.m_chunkArrayIndex = storage.m_entityAliveCount - remainAmount + i;

        entityInfo.m_handle = Handle();

        m_sceneDataStorage.m_entityMap[entityInfo.m_handle] = entity;
    }

    storage.m_chunkArray.m_entities.insert(
        storage.m_chunkArray.m_entities.end(),
        m_sceneDataStorage.m_entities.begin() + originalSize,
        m_sceneDataStorage.m_entities.end());
    const int threadSize = Jobs::Workers().Size();
    int perThreadAmount = remainAmount / threadSize;
    if (perThreadAmount > 0)
    {
        std::vector<std::shared_future<void>> results;
        for (int i = 0; i < threadSize; i++)
        {
            results.push_back(Jobs::Workers()
                                  .Push([&, i, perThreadAmount, originalSize](int id) {
                                      const Transform transform;
                                      const GlobalTransform globalTransform;
                                      for (int index = originalSize + i * perThreadAmount;
                                           index < originalSize + (i + 1) * perThreadAmount;
                                           index++)
                                      {
                                          auto &entity = m_sceneDataStorage.m_entities.at(index);
                                          SetDataComponent(entity, transform);
                                          SetDataComponent(entity, globalTransform);
                                          SetDataComponent(entity, GlobalTransformUpdateFlag());
                                      }
                                  })
                                  .share());
        }
        results.push_back(Jobs::Workers()
                              .Push([&, perThreadAmount, originalSize, remainAmount, threadSize](int id) {
                                  const Transform transform;
                                  const GlobalTransform globalTransform;
                                  for (int index = originalSize + perThreadAmount * threadSize;
                                       index < originalSize + remainAmount;
                                       index++)
                                  {
                                      auto &entity = m_sceneDataStorage.m_entities.at(index);
                                      SetDataComponent(entity, transform);
                                      SetDataComponent(entity, globalTransform);
                                      SetDataComponent(entity, GlobalTransformUpdateFlag());
                                  }
                              })
                              .share());
        for (const auto &i : results)
            i.wait();
    }

    retVal.insert(
        retVal.end(), m_sceneDataStorage.m_entities.begin() + originalSize, m_sceneDataStorage.m_entities.end());
    return retVal;
}

std::vector<Entity> Scene::CreateEntities(const size_t &amount, const std::string &name)
{
    return CreateEntities(Entities::GetInstance().m_basicArchetype, amount, name);
}

void Scene::DeleteEntity(const Entity &entity)
{
    if (!IsEntityValid(entity))
    {
        return;
    }
    m_saved = false;
    const size_t entityIndex = entity.m_index;
    auto children = m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_children;
    for (const auto &child : children)
    {
        DeleteEntity(child);
    }
    if (m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_parent.m_index != 0)
        RemoveChild(entity, m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_parent);
    DeleteEntityInternal(entity.m_index);
}

std::string Scene::GetEntityName(const Entity &entity)
{
    assert(IsEntityValid(entity));
    const size_t index = entity.m_index;
    if (entity != m_sceneDataStorage.m_entities.at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return "";
    }
    return m_sceneDataStorage.m_entityMetadataList.at(index).m_name;
}

void Scene::SetEntityName(const Entity &entity, const std::string &name)
{
    assert(IsEntityValid(entity));
    const size_t index = entity.m_index;
    if (entity != m_sceneDataStorage.m_entities.at(index))
    {
        UNIENGINE_ERROR("Child already deleted!");
        return;
    }
    m_saved = false;
    if (name.length() != 0)
    {
        m_sceneDataStorage.m_entityMetadataList.at(index).m_name = name;
        return;
    }
    m_sceneDataStorage.m_entityMetadataList.at(index).m_name = "Unnamed";
}
void Scene::SetEntityStatic(const Entity &entity, bool value)
{
    assert(IsEntityValid(entity));
    const size_t childIndex = entity.m_index;
    auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(GetRoot(entity).m_index);
    entityInfo.m_static = value;
    m_saved = false;
}
void Scene::SetParent(const Entity &entity, const Entity &parent, const bool &recalculateTransform)
{
    assert(IsEntityValid(entity) && IsEntityValid(parent));
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    auto &parentEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(parentIndex);
    for (const auto &i : parentEntityInfo.m_children)
    {
        if (i == entity)
            return;
    }
    m_saved = false;
    auto &childEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(childIndex);
    if (childEntityInfo.m_parent.GetIndex() != 0)
    {
        RemoveChild(entity, childEntityInfo.m_parent);
    }
    if (recalculateTransform)
    {
        const auto childGlobalTransform = GetDataComponent<GlobalTransform>(entity);
        const auto parentGlobalTransform = GetDataComponent<GlobalTransform>(parent);
        Transform childTransform;
        childTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * childGlobalTransform.m_value;
        SetDataComponent(entity, childTransform);
    }
    childEntityInfo.m_parent = parent;
    childEntityInfo.m_root = parentEntityInfo.m_root;
    childEntityInfo.m_static = false;
    parentEntityInfo.m_children.push_back(entity);
}

Entity Scene::GetParent(const Entity &entity)
{
    assert(IsEntityValid(entity));
    const size_t entityIndex = entity.m_index;
    return m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_parent;
}

std::vector<Entity> Scene::GetChildren(const Entity &entity)
{
    assert(IsEntityValid(entity));
    const size_t entityIndex = entity.m_index;
    return m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_children;
}

Entity Scene::GetChild(const Entity &entity, int index)
{
    assert(IsEntityValid(entity));
    const size_t entityIndex = entity.m_index;
    auto &children = m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_children;
    if (children.size() > index)
        return children[index];
    return Entity();
}

size_t Scene::GetChildrenAmount(const Entity &entity)
{
    assert(IsEntityValid(entity));
    const size_t entityIndex = entity.m_index;
    return m_sceneDataStorage.m_entityMetadataList.at(entityIndex).m_children.size();
}

inline void Scene::ForEachChild(const Entity &entity, const std::function<void(Entity child)> &func)
{
    assert(IsEntityValid(entity));
    auto children = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_children;
    for (auto i : children)
    {
        if (IsEntityValid(i))
            func(i);
    }
}

void Scene::RemoveChild(const Entity &entity, const Entity &parent)
{
    assert(IsEntityValid(entity) && IsEntityValid(parent));
    const size_t childIndex = entity.m_index;
    const size_t parentIndex = parent.m_index;
    auto &childEntityMetadata = m_sceneDataStorage.m_entityMetadataList.at(childIndex);
    auto &parentEntityMetadata = m_sceneDataStorage.m_entityMetadataList.at(parentIndex);
    if (childEntityMetadata.m_parent.m_index == 0)
    {
        UNIENGINE_ERROR("No child by the parent!");
    }
    m_saved = false;
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
    const auto childGlobalTransform = GetDataComponent<GlobalTransform>(entity);
    Transform childTransform;
    childTransform.m_value = childGlobalTransform.m_value;
    SetDataComponent(entity, childTransform);
}

void Scene::RemoveDataComponent(const Entity &entity, const size_t &typeID)
{
    assert(IsEntityValid(entity));
    if (typeID == typeid(Transform).hash_code() || typeID == typeid(GlobalTransform).hash_code() ||
        typeID == typeid(GlobalTransformUpdateFlag).hash_code())
    {
        return;
    }
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    auto &entityArchetypeInfos = Entities::GetInstance().m_entityArchetypeInfos;
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
    newArchetypeInfo.m_chunkCapacity = Entities::GetArchetypeChunkSize() / newArchetypeInfo.m_entitySize;
    auto archetype = Entities::CreateEntityArchetypeHelper(newArchetypeInfo);
#pragma endregion
#pragma region Create new Entity with new archetype
    const Entity newEntity = CreateEntity(archetype);
    // Transfer component data
    for (const auto &type : newArchetypeInfo.m_dataComponentTypes)
    {
        SetDataComponent(newEntity.m_index, type.m_typeId, type.m_size, GetDataComponentPointer(entity, type.m_typeId));
    }
    // 5. Swap entity.
    EntityMetadata &newEntityInfo = m_sceneDataStorage.m_entityMetadataList.at(newEntity.m_index);
    const auto tempArchetypeInfoIndex = newEntityInfo.m_dataComponentStorageIndex;
    const auto tempChunkArrayIndex = newEntityInfo.m_chunkArrayIndex;
    newEntityInfo.m_dataComponentStorageIndex = entityInfo.m_dataComponentStorageIndex;
    newEntityInfo.m_chunkArrayIndex = entityInfo.m_chunkArrayIndex;
    entityInfo.m_dataComponentStorageIndex = tempArchetypeInfoIndex;
    entityInfo.m_chunkArrayIndex = tempChunkArrayIndex;
    // Apply to chunk.
    m_sceneDataStorage.m_dataComponentStorages.at(entityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[entityInfo.m_chunkArrayIndex] = entity;
    m_sceneDataStorage.m_dataComponentStorages.at(newEntityInfo.m_dataComponentStorageIndex)
        .m_chunkArray.m_entities[newEntityInfo.m_chunkArrayIndex] = newEntity;
    DeleteEntity(newEntity);
#pragma endregion
    m_saved = false;
}

void Scene::SetDataComponent(const unsigned &entityIndex, size_t id, size_t size, IDataComponent *data)
{
    m_saved = false;
    auto &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entityIndex);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
IDataComponent *Scene::GetDataComponentPointer(unsigned entityIndex, const size_t &id)
{
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entityIndex);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
IDataComponent *Scene::GetDataComponentPointer(const Entity &entity, const size_t &id)
{
    assert(IsEntityValid(entity));
    EntityMetadata &entityInfo = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
    auto &dataComponentStorage = m_sceneDataStorage.m_dataComponentStorages[entityInfo.m_dataComponentStorageIndex];
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
Handle Scene::GetEntityHandle(const Entity &entity)
{
    return m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_handle;
}
void Scene::SetPrivateComponent(const Entity &entity, const std::shared_ptr<IPrivateComponent> &ptr)
{
    assert(ptr && IsEntityValid(entity));
    auto typeName = ptr->GetTypeName();
    auto &elements = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
    for (auto &element : elements)
    {
        if (typeName == element.m_privateComponentData->GetTypeName())
        {
            return;
        }
    }

    auto id = Serialization::GetSerializableTypeId(typeName);
    m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, id);
    elements.emplace_back(id, ptr, entity, std::dynamic_pointer_cast<Scene>(m_self.lock()));
    m_saved = false;
}

void Scene::ForEachDescendantHelper(const Entity &target, const std::function<void(const Entity &entity)> &func)
{
    func(target);
    ForEachChild(target, [&](Entity child) { ForEachDescendantHelper(child, func); });
}

Entity Scene::GetRoot(const Entity &entity)
{
    Entity retVal = entity;
    auto parent = GetParent(retVal);
    while (parent.GetIndex() != 0)
    {
        retVal = parent;
        parent = GetParent(retVal);
    }
    return retVal;
}

Entity Scene::GetEntity(const size_t &index)
{
    if (index > 0 && index < m_sceneDataStorage.m_entities.size())
        return m_sceneDataStorage.m_entities.at(index);
    return {};
}

void Scene::RemovePrivateComponent(const Entity &entity, size_t typeId)
{
    assert(IsEntityValid(entity));
    auto &privateComponentElements =
        m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
    for (auto i = 0; i < privateComponentElements.size(); i++)
    {
        if (privateComponentElements[i].m_typeId == typeId)
        {
            m_sceneDataStorage.m_entityPrivateComponentStorage.RemovePrivateComponent(
                entity, typeId, privateComponentElements[i].m_privateComponentData);
            privateComponentElements.erase(privateComponentElements.begin() + i);
            m_saved = false;
            break;
        }
    }
}

void Scene::SetEnable(const Entity &entity, const bool &value)
{
    assert(IsEntityValid(entity));
    if (m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled != value)
    {
        for (auto &i : m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements)
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
    m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled = value;

    for (const auto &i : m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_children)
    {
        SetEnable(i, value);
    }
    m_saved = false;
}

void Scene::SetEnableSingle(const Entity &entity, const bool &value)
{
    assert(IsEntityValid(entity));
    auto &entityMetadata = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index);
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
    }
}

void Scene::ForAllEntities(const std::function<void(int i, Entity entity)> &func)
{
    for (int index = 0; index < m_sceneDataStorage.m_entities.size(); index++)
    {
        if (m_sceneDataStorage.m_entities.at(index).m_version != 0)
        {
            func(index, m_sceneDataStorage.m_entities.at(index));
        }
    }
}

void Scene::GetEntityArray(const EntityQuery &entityQuery, std::vector<Entity> &container, bool checkEnable)
{
    assert(entityQuery.IsValid());
    auto queriedStorages = QueryDataComponentStorages(entityQuery);
    for (const auto i : queriedStorages)
    {
        GetEntityStorage(i.get(), container, checkEnable);
    }
}

size_t Scene::GetEntityAmount(EntityQuery entityQuery, bool checkEnable)
{
    assert(entityQuery.IsValid());
    size_t retVal = 0;
    if (checkEnable)
    {
        auto queriedStorages = QueryDataComponentStorages(entityQuery);
        for (const auto i : queriedStorages)
        {
            for (int index = 0; index < i.get().m_entityAliveCount; index++)
            {
                if (IsEntityEnabled(i.get().m_chunkArray.m_entities[index]))
                    retVal++;
            }
        }
    }
    else
    {
        auto queriedStorages = QueryDataComponentStorages(entityQuery);
        for (const auto i : queriedStorages)
        {
            retVal += i.get().m_entityAliveCount;
        }
    }
    return retVal;
}

std::vector<Entity> Scene::GetDescendants(const Entity &entity)
{
    std::vector<Entity> retVal;
    GetDescendantsHelper(entity, retVal);
    return retVal;
}
void Scene::GetDescendantsHelper(const Entity &target, std::vector<Entity> &results)
{
    auto &children = m_sceneDataStorage.m_entityMetadataList.at(target.m_index).m_children;
    if (!children.empty())
        results.insert(results.end(), children.begin(), children.end());
    for (const auto &i : children)
        GetDescendantsHelper(i, results);
}
template <typename T> std::vector<Entity> Scene::GetPrivateComponentOwnersList(const std::shared_ptr<Scene> &scene)
{
    return m_sceneDataStorage.m_entityPrivateComponentStorage.GetOwnersList<T>();
}

std::weak_ptr<IPrivateComponent> Scene::GetPrivateComponent(const Entity &entity, const std::string &typeName)
{
    size_t i = 0;
    auto &elements = m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements;
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

Entity Scene::GetEntity(const Handle &handle)
{
    auto search = m_sceneDataStorage.m_entityMap.find(handle);
    if (search != m_sceneDataStorage.m_entityMap.end())
    {
        return search->second;
    }
    return {};
}
bool Scene::HasPrivateComponent(const Entity &entity, const std::string &typeName)
{
    assert(IsEntityValid(entity));
    for (auto &element : m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_privateComponentElements)
    {
        if (element.m_privateComponentData->m_typeName == typeName)
        {
            return true;
        }
    }
    return false;
}
std::vector<std::reference_wrapper<DataComponentStorage>> Scene::QueryDataComponentStorages(
    unsigned int entityQueryIndex)
{
    auto &queryInfos = Entities::GetInstance().m_entityQueryInfos.at(entityQueryIndex);
    auto &entityComponentStorage = m_sceneDataStorage.m_dataComponentStorages;
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
bool Scene::IsEntityValid(const Entity &entity)
{
    auto &storage = m_sceneDataStorage.m_entities;
    return entity.m_index != 0 && entity.m_version != 0 && entity.m_index < storage.size() &&
           storage.at(entity.m_index).m_version == entity.m_version;
}
bool Scene::IsEntityEnabled(const Entity &entity)
{
    assert(IsEntityValid(entity));
    return m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_enabled;
}
bool Scene::IsEntityRoot(const Entity &entity)
{
    assert(IsEntityValid(entity));
    return m_sceneDataStorage.m_entityMetadataList.at(entity.m_index).m_root == entity;
}
bool Scene::IsEntityStatic(const Entity &entity)
{
    assert(IsEntityValid(entity));
    return m_sceneDataStorage.m_entityMetadataList.at(GetRoot(entity).m_index).m_static;
}

#pragma endregion