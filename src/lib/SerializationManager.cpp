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

std::string ComponentFactory::GetSerializableTypeName(const size_t &typeId)
{
    return GetInstance().m_serializableNames.find(typeId)->second;
}

bool ComponentFactory::RegisterDataComponent(
    const std::string &typeName,
    const size_t &typeId,
    const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func)
{
    GetInstance().m_dataComponentNames[typeId] = typeName;
    return GetInstance().m_dataComponentGenerators.insert({typeName, func}).second;
}

std::shared_ptr<IDataComponent> ComponentFactory::ProduceDataComponent(
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

bool ComponentFactory::RegisterSerializable(
    const std::string &typeName, const size_t &typeId, const std::function<ISerializable *(size_t &)> &func)
{
    GetInstance().m_serializableNames[typeId] = typeName;
    return GetInstance().m_serializableGenerators.insert({typeName, func}).second;
}

ISerializable *ComponentFactory::ProduceSerializable(const std::string &typeName, size_t &hashCode)
{
    const auto it = GetInstance().m_serializableGenerators.find(typeName);
    if (it != GetInstance().m_serializableGenerators.end())
    {
        return std::move(it->second(hashCode));
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
/*
void UniEngine::SerializationManager::SerializeEntity(
    std::shared_ptr<Scene> world, YAML::Emitter &out, const Entity &entity)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Entity" << YAML::Value << std::to_string(entity.GetIndex());
    out << YAML::Key << "IsEnabled" << YAML::Value << entity.IsEnabled();
    out << YAML::Key << "ArchetypeName" << YAML::Value
        << EntityManager::GetEntityArchetypeName(EntityManager::GetEntityArchetype(entity));
    out << YAML::Key << "Name" << YAML::Value << entity.GetName();
    out << YAML::Key << "Parent" << YAML::Value << EntityManager::GetParent(entity).GetIndex();
#pragma region ComponentData
    out << YAML::Key << "DataComponent" << YAML::Value << YAML::BeginSeq;
    auto &storage = world->m_sceneDataStorage;
    std::vector<DataComponentType> &componentTypes =
        storage.m_entityComponentStorage[storage.m_entityInfos[entity.GetIndex()].m_archetypeInfoIndex].m_componentTypes;
    for (const auto &type : componentTypes)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << type.m_name;
        auto *ptr =
            reinterpret_cast<const unsigned char *>(EntityManager::GetDataComponentPointer(entity, type.m_typeId));
        out << YAML::Key << "Data" << YAML::Value << YAML::Binary(ptr, type.m_size);
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
#pragma endregion

#pragma region Private Components
    out << YAML::Key << "PrivateComponent" << YAML::Value << YAML::BeginSeq;
    EntityManager::ForEachPrivateComponent(entity, [&](PrivateComponentElement &data) {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << ComponentFactory::GetSerializableTypeName(data.m_typeId);
        out << YAML::Key << "IsEnabled" << YAML::Value << data.m_privateComponentData->m_enabled;
        data.m_privateComponentData->Serialize(out);
        out << YAML::EndMap;
    });
    out << YAML::EndSeq;
#pragma endregion

    out << YAML::EndMap;
}

UniEngine::Entity UniEngine::SerializationManager::DeserializeEntity(
    std::shared_ptr<Scene> world, const YAML::Node &node)
{
    const auto entityName = node["Name"].as<std::string>();
    const auto archetypeName = node["ArchetypeName"].as<std::string>();
    auto componentDatum = node["DataComponent"];
    Entity retVal;

    std::vector<std::shared_ptr<IDataComponent>> ptrs;
    std::vector<DataComponentType> types;
    for (const auto &componentData : componentDatum)
    {
        auto name = componentData["Name"].as<std::string>();
        size_t hashCode;
        size_t size;
        auto ptr = ComponentFactory::ProduceDataComponent(name, hashCode, size);
        auto data = componentData["Data"].as<YAML::Binary>();
        std::memcpy(ptr.get(), data.data(), data.size());
        ptrs.push_back(ptr);
        types.emplace_back(name, hashCode, size);
    }
    const EntityArchetype archetype = EntityManager::CreateEntityArchetype(archetypeName, types);
    retVal = EntityManager::CreateEntity(archetype, entityName);
    for (int i = 0; i < ptrs.size(); i++)
    {
        EntityManager::SetDataComponent(retVal, types[i].m_typeId, types[i].m_size, ptrs[i].get());
    }

    auto privateComponents = node["PrivateComponent"];
    if (privateComponents)
    {
        for (const auto &privateComponent : privateComponents)
        {
            auto name = privateComponent["Name"].as<std::string>();
            size_t hashCode;
            auto *ptr = dynamic_cast<IPrivateComponent *>(ComponentFactory::ProduceSerializable(name, hashCode));
            ptr->Deserialize(privateComponent);
            ptr->m_enabled = privateComponent["IsEnabled"].as<bool>();
            EntityManager::SetPrivateComponent(retVal, name, hashCode, ptr);
        }
    }

    return retVal;
}
*/
void UniEngine::SerializationManager::SerializeScene(std::shared_ptr<Scene> scene, const std::string &path)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << scene->m_name;
    auto &sceneDataStorage = scene->m_sceneDataStorage;
#pragma region EntityInfo
    out << YAML::Key << "EntityInfo" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_entityInfos.size(); i++)
    {
        SerializeEntityInfo(sceneDataStorage.m_entityInfos[i], out);
    }
    out << YAML::EndSeq;
#pragma endregion
#pragma region DataComponentStorage
    out << YAML::Key << "DataComponentStorage" << YAML::Value << YAML::BeginSeq;
    for (int i = 1; i < sceneDataStorage.m_entityComponentStorage.size(); i++)
    {
        SerializeDataComponentStorage(sceneDataStorage.m_entityComponentStorage[i], out);
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

bool UniEngine::SerializationManager::DeserializeScene(std::shared_ptr<Scene> scene, const std::string &path)
{
    std::ifstream stream(path);
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node data = YAML::Load(stringStream.str());
    if (!data["Scene"])
    {
        return false;
    }
    UNIENGINE_LOG("Loading scene...");
    scene->Purge();
    auto &storage = scene->m_sceneDataStorage;
    scene->m_name = data["Scene"].as<std::string>();
#pragma region DataComponentStorage
    auto entityComponentStorage = data["DataComponentStorage"];
    for (const auto &dataComponentStorage : entityComponentStorage)
    {
    }
#pragma endregion
#pragma region EntityInfo
    auto entityInfos = data["EntityInfo"];
    for (const auto &entityInfo : entityInfos)
    {
        storage.m_entityInfos.emplace_back();
        auto &newInfo = storage.m_entityInfos.back();
        newInfo.m_name = entityInfo["Name"].as<std::string>();
        newInfo.m_version = entityInfo["Version"].as<bool>();
        newInfo.m_static = entityInfo["Static"].as<bool>();
        newInfo.m_enabled = entityInfo["Enabled"].as<bool>();
        Entity parent;
        parent.m_index = entityInfo["Parent.m_index"].as<unsigned>();
        parent.m_version = entityInfo["Parent.m_version"].as<unsigned>();
        newInfo.m_parent = parent;
        YAML::Binary childrenData = entityInfo["Children"].as<YAML::Binary>();
        const unsigned char *data = childrenData.data();
        std::size_t size = childrenData.size();
        newInfo.m_children.resize(size / sizeof(Entity));
        std::memcpy(newInfo.m_children.data(), data, size);
        //newInfo. = entityInfo["ArchetypeInfoIndex"].as<size_t>();
        newInfo.m_chunkArrayIndex = entityInfo["ChunkArrayIndex"].as<size_t>();
    }
#pragma endregion
#pragma region Entities
    auto entitiesData = data["Entities"].as<YAML::Binary>();
    const unsigned char *entitiesDataPtr = entitiesData.data();
    std::size_t entitiesSize = entitiesData.size();
    storage.m_entities.resize(entitiesSize / sizeof(Entity));
    std::memcpy(storage.m_entities.data(), entitiesDataPtr, entitiesSize);
#pragma endregion
    return true;
}
void SerializationManager::SerializeDataComponentStorage(const DataComponentStorage &storage, YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "EntityArchetypeInfo" << YAML::Value << YAML::BeginMap;
        {
            out << YAML::Key << "EntitySize" << YAML::Value << storage.m_entitySize;
            out << YAML::Key << "ChunkCapacity" << YAML::Value << storage.m_chunkCapacity;
            out << YAML::Key << "EntityCount" << YAML::Value << storage.m_entityCount;
            out << YAML::Key << "EntityAliveCount" << YAML::Value << storage.m_entityAliveCount;
            out << YAML::Key << "ComponentTypes" << YAML::Value << YAML::BeginSeq;
            for (const auto &i : storage.m_componentTypes)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << i.m_name;
                out << YAML::Key << "Size" << YAML::Value << i.m_size;
                out << YAML::Key << "Offset" << YAML::Value << i.m_offset;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;
        out << YAML::Key << "DataComponentChunkArray" << YAML::Value << YAML::BeginMap;
        {
            auto &array = storage.m_chunkArray;
            out << YAML::Key << "Entities" << YAML::Value
                << YAML::Binary((const unsigned char *)array.Entities.data(), array.Entities.size() * sizeof(Entity));
            out << YAML::Key << "Chunks" << YAML::Value << YAML::BeginSeq;
            for (const auto &i : array.Chunks)
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
        out << YAML::Key << "Parent.m_index" << YAML::Value << entityInfo.m_parent.m_index;
        out << YAML::Key << "Parent.m_version" << YAML::Value << entityInfo.m_parent.m_version;
        out << YAML::Key << "Children" << YAML::Value
            << YAML::Binary(
                   (const unsigned char *)entityInfo.m_children.data(), entityInfo.m_children.size() * sizeof(Entity));
        //out << YAML::Key << "ArchetypeInfoIndex" << YAML::Value << entityInfo.m_archetypeInfoIndex;
        out << YAML::Key << "ChunkArrayIndex" << YAML::Value << entityInfo.m_chunkArrayIndex;
#pragma region Private Components
        out << YAML::Key << "PrivateComponent" << YAML::Value << YAML::BeginSeq;
        for (const auto &element : entityInfo.m_privateComponentElements)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << ComponentFactory::GetSerializableTypeName(element.m_typeId);
            out << YAML::Key << "Enabled" << YAML::Value << element.m_privateComponentData->m_enabled;
            element.m_privateComponentData->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
#pragma endregion
    }
    out << YAML::EndMap;
}
