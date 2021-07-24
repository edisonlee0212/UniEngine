#include <Camera.hpp>
#include <ISerializable.hpp>
#include <MeshRenderer.hpp>
#include <PhysicsManager.hpp>
#include <PostProcessing.hpp>
#include <RenderManager.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;

ComponentDataRegistration<Transform> TransformRegistry("Transform");
ComponentDataRegistration<GlobalTransform> GlobalTransformRegistry("GlobalTransform");
ComponentDataRegistration<GlobalTransformUpdateFlag> GlobalTransformUpdateFlagRegistry("GlobalTransformUpdateFlag");
ComponentDataRegistration<Ray> RayRegistry("Ray");

SerializableRegistration<Animator> AnimatorRegistry("Animator");
SerializableRegistration<Joint> JointRegistry("Joint");
SerializableRegistration<Articulation> ArticulationRegistry("Articulation");
SerializableRegistration<RigidBody> RigidBodyRegistry("RigidBody");
SerializableRegistration<SpotLight> SpotLightRegistry("SpotLight");
SerializableRegistration<PointLight> PointLightRegistry("PointLight");
SerializableRegistration<DirectionalLight> DirectionalLightRegistry("DirectionalLight");
SerializableRegistration<Camera> CameraRegistry("Camera");
SerializableRegistration<Particles> ParticlesRegistry("Particles");
SerializableRegistration<MeshRenderer> MeshRendererRegistry("MeshRenderer");
SerializableRegistration<PostProcessing> PostProcessingRegistry("PostProcessing");
SerializableRegistration<SkinnedMeshRenderer> SkinnedMeshRendererRegistry("SkinnedMeshRenderer");


SerializableRegistration<PhysicsSystem> PhysicsSystemRegistry("PhysicsSystem");


std::string SerializableFactory::GetSerializableTypeName(const size_t &typeId)
{
    return GetInstance().m_serializableNames.find(typeId)->second;
}

bool SerializableFactory::RegisterDataComponent(
    const std::string &typeName,
    const size_t &typeId,
    const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func)
{
    GetInstance().m_dataComponentNames[typeId] = typeName;
    GetInstance().m_dataComponentIds[typeName] = typeId;
    return GetInstance().m_dataComponentGenerators.insert({typeName, func}).second;
}

std::shared_ptr<IDataComponent> SerializableFactory::ProduceDataComponent(
    const std::string &typeName, size_t &hashCode, size_t &size)
{
    auto &factory = GetInstance();
    const auto it = factory.m_dataComponentGenerators.find(typeName);
    if (it != GetInstance().m_dataComponentGenerators.end())
    {
        return it->second(hashCode, size);
    }
    UNIENGINE_ERROR("DataComponent " + typeName + "is not registered!");
    throw 1;
}

bool SerializableFactory::RegisterSerializable(
    const std::string &typeName, const size_t &typeId, const std::function<ISerializable *(size_t &)> &func)
{
    GetInstance().m_serializableNames[typeId] = typeName;
    return GetInstance().m_serializableGenerators.insert({typeName, func}).second;
}

ISerializable *SerializableFactory::ProduceSerializable(const std::string &typeName, size_t &hashCode)
{
    const auto it = GetInstance().m_serializableGenerators.find(typeName);
    if (it != GetInstance().m_serializableGenerators.end())
    {
        auto retVal = it->second(hashCode);
        retVal->m_typeName = typeName;
        return std::move(retVal);
    }
    UNIENGINE_ERROR("PrivateComponent " + typeName + "is not registered!");
    throw 1;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::vec2 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::vec3 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::vec4 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::mat4 &v)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Row0" << YAML::Value << v[0];
    out << YAML::Key << "Row1" << YAML::Value << v[1];
    out << YAML::Key << "Row2" << YAML::Value << v[2];
    out << YAML::Key << "Row3" << YAML::Value << v[3];
    out << YAML::EndMap;
    return out;
}

std::ostream &UniEngine::operator<<(std::ostream &out, const glm::vec2 &v)
{
    out << "[" << v.x << ',' << v.y << ']';
    return out;
}

std::ostream &UniEngine::operator<<(std::ostream &out, const glm::vec3 &v)
{
    out << "[" << v.x << ',' << v.y << ',' << v.z << ']';
    return out;
}

std::ostream &UniEngine::operator<<(std::ostream &out, const glm::vec4 &v)
{
    out << "[" << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ']';
    return out;
}

std::ostream &UniEngine::operator<<(std::ostream &out, const glm::mat4 &v)
{
    out << "[" << v[0] << ',' << v[1] << ',' << v[2] << ',' << v[3] << ']';
    return out;
}

std::istream &UniEngine::operator>>(std::istream &in, glm::vec2 &v)
{
    char temp;
    in >> temp >> v.x >> temp >> v.y >> temp;
    return in;
}

std::istream &UniEngine::operator>>(std::istream &in, glm::vec3 &v)
{
    char temp;
    in >> temp >> v.x >> temp >> v.y >> temp >> v.z >> temp;
    return in;
}

std::istream &UniEngine::operator>>(std::istream &in, glm::vec4 &v)
{
    char temp;
    in >> temp >> v.x >> temp >> v.y >> temp >> v.z >> temp >> v.w >> temp;
    return in;
}

std::istream &UniEngine::operator>>(std::istream &in, glm::mat4 &v)
{
    char temp;
    in >> temp >> v[0] >> temp >> v[1] >> temp >> v[2] >> temp >> v[3] >> temp;
    return in;
}

void UniEngine::SerializationManager::SerializeScene(const std::shared_ptr<Scene> &scene, const std::string &path)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << scene->m_name;
    auto &sceneDataStorage = scene->m_sceneDataStorage;
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
    for (const auto& i : scene->m_systems)
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
    std::ofstream fout(path);
    fout << out.c_str();
    fout.flush();
    UNIENGINE_LOG("Scene saved to " + path);
}
void SerializationManager::SerializeDataComponentStorage(const DataComponentStorage &storage, YAML::Emitter &out)
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
void SerializationManager::SerializeEntityInfo(const EntityInfo &entityInfo, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "Name" << YAML::Value << entityInfo.m_name;
        out << YAML::Key << "Version" << YAML::Value << entityInfo.m_version;
        out << YAML::Key << "Static" << YAML::Value << entityInfo.m_static;
        out << YAML::Key << "Enabled" << YAML::Value << entityInfo.m_enabled;
        out << YAML::Key << "Parent.Index" << YAML::Value << entityInfo.m_parent.m_index;
        out << YAML::Key << "Parent.Version" << YAML::Value << entityInfo.m_parent.m_version;
        if(!entityInfo.m_children.empty())
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

void SerializationManager::SerializeSystem(const std::shared_ptr<ISystem>& system, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "TypeName" << YAML::Value << system->m_typeName;
        out << YAML::Key << "Enabled" << YAML::Value << system->m_enabled;
        out << YAML::Key << "Rank" << YAML::Value << system->m_rank;

        system->Serialize(out);
    }
    out << YAML::EndMap;
}
std::shared_ptr<Scene> UniEngine::SerializationManager::DeserializeScene(const std::string &path)
{
    std::ifstream stream(path);
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node data = YAML::Load(stringStream.str());
    if (!data["Scene"])
    {
        UNIENGINE_ERROR("Scene load failed!");
        return nullptr;
    }
    UNIENGINE_LOG("Loading scene...");
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    auto &sceneDataStorage = scene->m_sceneDataStorage;
    scene->m_name = data["Scene"].as<std::string>();
#pragma region DataComponentStorage
    auto inDataComponentStorages = data["DataComponentStorages"];
    for (const auto &inDataComponentStorage : inDataComponentStorages)
    {
        sceneDataStorage.m_dataComponentStorages.emplace_back();
        auto &dataComponentStorage = sceneDataStorage.m_dataComponentStorages.back();
        dataComponentStorage.m_entitySize = inDataComponentStorage["EntitySize"].as<size_t>();
        dataComponentStorage.m_chunkCapacity = inDataComponentStorage["ChunkCapacity"].as<size_t>();
        dataComponentStorage.m_entityCount = inDataComponentStorage["EntityCount"].as<size_t>();
        dataComponentStorage.m_entityAliveCount = inDataComponentStorage["EntityAliveCount"].as<size_t>();
        auto inDataComponentTypes = inDataComponentStorage["DataComponentTypes"];
        for(const auto& inDataComponentType : inDataComponentTypes){
            DataComponentType dataComponentType;
            dataComponentType.m_name = inDataComponentType["Name"].as<std::string>();
            dataComponentType.m_size = inDataComponentType["Size"].as<size_t>();
            dataComponentType.m_offset = inDataComponentType["Offset"].as<size_t>();
            auto& ids = SerializableFactory::GetInstance().m_dataComponentIds;
            dataComponentType.m_typeId = ids[dataComponentType.m_name];
            dataComponentStorage.m_dataComponentTypes.push_back(dataComponentType);
        }
        auto inDataChunkArray = inDataComponentStorage["DataComponentChunkArray"];
        if(inDataChunkArray["Entities"].IsDefined())
        {
            YAML::Binary entitiesData = inDataChunkArray["Entities"].as<YAML::Binary>();
            const unsigned char *data = entitiesData.data();
            std::size_t size = entitiesData.size();
            dataComponentStorage.m_chunkArray.m_entities.resize(size / sizeof(Entity));
            std::memcpy(dataComponentStorage.m_chunkArray.m_entities.data(), data, size);
        }
        auto inChunks = inDataChunkArray["Chunks"];
        for(const auto& chunk : inChunks){
            dataComponentStorage.m_chunkArray.m_chunks.emplace_back();
            auto& componentDataChunk = dataComponentStorage.m_chunkArray.m_chunks.back();
            componentDataChunk.m_data = static_cast<void *>(calloc(1, EntityManager::GetInstance().m_archetypeChunkSize));
            YAML::Binary chunkData = chunk["Data"].as<YAML::Binary>();
            const unsigned char *data = chunkData.data();
            std::size_t size = chunkData.size();
            assert(size == ARCHETYPE_CHUNK_SIZE);
            std::memcpy(componentDataChunk.m_data, data, size);
        }
    }
#pragma endregion
#pragma region EntityInfo
    auto inEntityInfos = data["EntityInfos"];
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
        if(inEntityInfo["Children"].IsDefined())
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
                scene->m_sceneDataStorage.m_entityPrivateComponentStorage.SetPrivateComponent(entity, hashCode);
                ptr->Deserialize(inPrivateComponent);
            }
        }
        entityIndex++;
    }
#pragma endregion
#pragma region Entities
    auto entitiesData = data["Entities"].as<YAML::Binary>();
    const unsigned char *entitiesDataPtr = entitiesData.data();
    std::size_t entitiesSize = entitiesData.size();
    sceneDataStorage.m_entities.resize(entitiesSize / sizeof(Entity));
    std::memcpy(sceneDataStorage.m_entities.data(), entitiesDataPtr, entitiesSize);
#pragma endregion

#pragma region Systems
    auto inSystems = data["Systems"];
    for(const auto& inSystem : inSystems){
        auto name = inSystem["TypeName"].as<std::string>();
        size_t hashCode;
        auto ptr = std::shared_ptr<ISystem>(dynamic_cast<ISystem *>(SerializableFactory::ProduceSerializable(name, hashCode)));
        ptr->m_enabled = inSystem["Enabled"].as<bool>();
        ptr->m_scene = scene;
        ptr->m_rank = inSystem["Rank"].as<float>();
        scene->m_systems.insert({ptr->m_rank, ptr});
        scene->m_indexedSystems.insert({hashCode, ptr});
        ptr->Deserialize(inSystem);
    }
#pragma endregion

    return scene;
}

