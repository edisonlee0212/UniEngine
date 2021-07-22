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

std::string ComponentFactory::GetSerializableTypeName(const size_t& typeId)
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
    const std::string &typeName, const size_t &typeId,
    const std::function<ISerializable*(size_t &)> &func)
{
    GetInstance().m_serializableNames[typeId] = typeName;
    return GetInstance().m_serializableGenerators.insert({typeName, func}).second;
}

ISerializable* ComponentFactory::ProduceSerializable(const std::string &typeName, size_t &hashCode)
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
        storage.m_entityComponentStorage[storage.m_entityInfos[entity.GetIndex()].m_archetypeInfoIndex]
            .m_archetypeInfo->m_componentTypes;
    for (const auto &type : componentTypes)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << type.m_name;
        auto *ptr = reinterpret_cast<const unsigned char*>(EntityManager::GetDataComponentPointer(entity, type.m_typeId));
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
            auto* ptr = dynamic_cast<IPrivateComponent*>(ComponentFactory::ProduceSerializable(name, hashCode));
            ptr->Deserialize(privateComponent);
            ptr->m_enabled = privateComponent["IsEnabled"].as<bool>();
            EntityManager::SetPrivateComponent(retVal, name, hashCode, ptr);
        }
    }

    return retVal;
}

void UniEngine::SerializationManager::Serialize(std::shared_ptr<Scene> world, const std::string &path)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene";
    out << YAML::Value << "World_Name";
    out << YAML::Key << "Entities";
    out << YAML::Value << YAML::BeginSeq;
    for (const auto &entity : world->m_sceneDataStorage.m_entities)
    {
        if (entity.GetVersion() == 0)
            continue;
        SerializeEntity(world, out, entity);
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(path);
    fout << out.c_str();
    fout.flush();
    UNIENGINE_LOG("Scene saved to " + path);
}

bool UniEngine::SerializationManager::Deserialize(std::shared_ptr<Scene> world, const std::string &path)
{
    std::ifstream stream(path);
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node data = YAML::Load(stringStream.str());
    if (!data["Scene"])
    {
        return false;
    }
    UNIENGINE_LOG("Loading world...");
    world->Purge();
    auto entities = data["Entities"];
    if (entities)
    {
        std::unordered_map<unsigned, Entity> entityMap;
        std::vector<std::pair<unsigned, unsigned>> childParentPairs;
        for (const auto &node : entities)
        {
            auto id = node["Entity"].as<unsigned>();
            auto parent = node["Parent"].as<unsigned>();

            auto entity = DeserializeEntity(world, node);
            world->m_sceneDataStorage.m_entityInfos[entity.GetIndex()].m_enabled = node["IsEnabled"].as<bool>();
            if (entity.IsNull())
            {
                UNIENGINE_ERROR("Error!");
            }
            entityMap.insert({id, entity});
            if (parent == 0)
                continue;
            childParentPairs.emplace_back(id, parent);
        }
        for (const auto &[fst, snd] : childParentPairs)
        {
            EntityManager::SetParent(entityMap[fst], entityMap[snd], false);
        }
    }
    return true;
}