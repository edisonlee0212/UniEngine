#include <CameraComponent.hpp>
#include <MeshRenderer.hpp>
#include <RenderManager.hpp>
#include <Serializable.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;

ComponentDataRegistration<Transform> TransformRegistry(1);
ComponentDataRegistration<GlobalTransform> GlobalTransformRegistry(1);
ComponentDataRegistration<Ray> RayRegistry(1);
ComponentDataRegistration<SpotLight> SpotLightRegistry(1);
ComponentDataRegistration<PointLight> PointLightRegistry(1);
ComponentDataRegistration<DirectionalLight> DirectionalLightRegistry(1);
ComponentDataRegistration<CameraLayerMask> CameraLayerMaskRegistry(1);

SerializableRegistration<CameraComponent> CameraComponentRegistry(1);
SerializableRegistration<Particles> ParticlesRegistry(1);
SerializableRegistration<MeshRenderer> MeshRendererRegistry(1);

bool ComponentFactory::Register(
    const std::string &id, const std::function<std::shared_ptr<ComponentDataBase>(size_t &, size_t &)> &func)
{
    return GetInstance().m_componentDataGenerators.insert({id, func}).second;
}

std::shared_ptr<ComponentDataBase> ComponentFactory::ProduceComponentData(
    const std::string &id, size_t &hashCode, size_t &size)
{
    auto &factory = GetInstance();
    const auto it = factory.m_componentDataGenerators.find(id);
    if (it != GetInstance().m_componentDataGenerators.end())
    {
        return it->second(hashCode, size);
    }
    UNIENGINE_ERROR("Component " + id + "is not registered!");
    throw 1;
}

bool ComponentFactory::Register(const std::string &id, const std::function<Serializable *(size_t &)> &func)
{
    return GetInstance().m_classComponentGenerators.insert({id, func}).second;
}

Serializable *ComponentFactory::ProduceSerializableObject(const std::string &id, size_t &hashCode)
{
    const auto it = GetInstance().m_classComponentGenerators.find(id);
    if (it != GetInstance().m_classComponentGenerators.end())
    {
        return it->second(hashCode);
    }
    UNIENGINE_ERROR("Component " + id + "is not registered!");
    throw 1;
}

void UniEngine::SerializationManager::Init()
{
    RegisterComponentDataSerializerDeserializer<Transform>(
        {[](ComponentDataBase *data) {
             Transform *out = static_cast<Transform *>(data);
             glm::mat4 val = out->m_value;
             std::stringstream stream;
             EXPORT_PARAM(stream, val);
             return stream.str();
         },
         [](const std::string &data, ComponentDataBase *ptr) {
             std::stringstream stream;
             stream << data;
             Transform *out = static_cast<Transform *>(ptr);
             char temp;
             IMPORT_PARAM(stream, out->m_value, temp);
         }});
    RegisterComponentDataSerializerDeserializer<GlobalTransform>(
        {[](ComponentDataBase *data) {
             GlobalTransform *out = static_cast<GlobalTransform *>(data);
             glm::mat4 val = out->m_value;
             std::stringstream stream;
             EXPORT_PARAM(stream, val);
             return stream.str();
         },
         [](const std::string &data, ComponentDataBase *ptr) {
             std::stringstream stream;
             stream << data;
             GlobalTransform *out = static_cast<GlobalTransform *>(ptr);
             char temp;
             IMPORT_PARAM(stream, out->m_value, temp);
         }});

    RegisterComponentDataSerializerDeserializer<Ray>(
        {[](ComponentDataBase *data) {
             Ray *out = static_cast<Ray *>(data);
             std::stringstream stream;
             EXPORT_PARAM(stream, out->m_start);
             EXPORT_PARAM(stream, out->m_direction);
             EXPORT_PARAM(stream, out->m_length);
             return stream.str();
         },
         [](const std::string &data, ComponentDataBase *ptr) {
             std::stringstream stream;
             stream << data;
             Ray *out = static_cast<Ray *>(ptr);
             char temp;
             IMPORT_PARAM(stream, out->m_start, temp);
             IMPORT_PARAM(stream, out->m_direction, temp);
             IMPORT_PARAM(stream, out->m_length, temp);
         }});
    /*
    RegisterComponentDataSerializerDeserializer<SpotLight>(
        {
        [](ComponentDataBase* data)
        {
            SpotLight* out = static_cast<SpotLight*>(data);
            std::stringstream stream;
            EXPORT_PARAM(stream, out->innerDegrees);
            EXPORT_PARAM(stream, out->outerDegrees);
            EXPORT_PARAM(stream, out->constant);
            EXPORT_PARAM(stream, out->linear);
            EXPORT_PARAM(stream, out->quadratic);
            EXPORT_PARAM(stream, out->bias);
            EXPORT_PARAM(stream, out->farPlane);
            EXPORT_PARAM(stream, out->diffuse);
            EXPORT_PARAM(stream, out->diffuseBrightness);
            EXPORT_PARAM(stream, out->specular);
            EXPORT_PARAM(stream, out->specularBrightness);
            EXPORT_PARAM(stream, out->lightSize);
            return stream.str();
        },
        [](const std::string& data, ComponentDataBase* ptr)
        {
            std::stringstream stream;
            stream << data;
            SpotLight* out = static_cast<SpotLight*>(ptr);
            char temp;
            IMPORT_PARAM(stream, out->innerDegrees, temp);
            IMPORT_PARAM(stream, out->outerDegrees, temp);
            IMPORT_PARAM(stream, out->constant, temp);
            IMPORT_PARAM(stream, out->linear, temp);
            IMPORT_PARAM(stream, out->quadratic, temp);
            IMPORT_PARAM(stream, out->bias, temp);
            IMPORT_PARAM(stream, out->farPlane, temp);
            IMPORT_PARAM(stream, out->diffuse, temp);
            IMPORT_PARAM(stream, out->diffuseBrightness, temp);
            IMPORT_PARAM(stream, out->specular, temp);
            IMPORT_PARAM(stream, out->specularBrightness, temp);
            IMPORT_PARAM(stream, out->lightSize, temp);
        }
        }
    );
    RegisterComponentDataSerializerDeserializer<PointLight>(
        {
        [](ComponentDataBase* data)
        {
            PointLight* out = static_cast<PointLight*>(data);
            std::stringstream stream;
            EXPORT_PARAM(stream, out->constant);
            EXPORT_PARAM(stream, out->linear);
            EXPORT_PARAM(stream, out->quadratic);
            EXPORT_PARAM(stream, out->bias);
            EXPORT_PARAM(stream, out->farPlane);
            EXPORT_PARAM(stream, out->diffuse);
            EXPORT_PARAM(stream, out->diffuseBrightness);
            EXPORT_PARAM(stream, out->specular);
            EXPORT_PARAM(stream, out->specularBrightness);
            EXPORT_PARAM(stream, out->lightSize);
            return stream.str();
        },
        [](const std::string& data, ComponentDataBase* ptr)
        {
            std::stringstream stream;
            stream << data;
            PointLight* out = static_cast<PointLight*>(ptr);
            char temp;
            IMPORT_PARAM(stream, out->constant, temp);
            IMPORT_PARAM(stream, out->linear, temp);
            IMPORT_PARAM(stream, out->quadratic, temp);
            IMPORT_PARAM(stream, out->bias, temp);
            IMPORT_PARAM(stream, out->farPlane, temp);
            IMPORT_PARAM(stream, out->diffuse, temp);
            IMPORT_PARAM(stream, out->diffuseBrightness, temp);
            IMPORT_PARAM(stream, out->specular, temp);
            IMPORT_PARAM(stream, out->specularBrightness, temp);
            IMPORT_PARAM(stream, out->lightSize, temp);
        }
        }
    );
    RegisterComponentDataSerializerDeserializer<DirectionalLight>(
        {
        [](ComponentDataBase* data)
        {
            DirectionalLight* out = static_cast<DirectionalLight*>(data);
            std::stringstream stream;
            EXPORT_PARAM(stream, out->bias);
            EXPORT_PARAM(stream, out->normalOffset);
            EXPORT_PARAM(stream, out->diffuse);
            EXPORT_PARAM(stream, out->diffuseBrightness);
            EXPORT_PARAM(stream, out->specular);
            EXPORT_PARAM(stream, out->specularBrightness);
            EXPORT_PARAM(stream, out->lightSize);
            return stream.str();
        },
        [](const std::string& data, ComponentDataBase* ptr)
        {
            std::stringstream stream;
            stream << data;
            DirectionalLight* out = static_cast<DirectionalLight*>(ptr);
            char temp;
            IMPORT_PARAM(stream, out->bias, temp);
            IMPORT_PARAM(stream, out->normalOffset, temp);
            IMPORT_PARAM(stream, out->diffuse, temp);
            IMPORT_PARAM(stream, out->diffuseBrightness, temp);
            IMPORT_PARAM(stream, out->specular, temp);
            IMPORT_PARAM(stream, out->specularBrightness, temp);
            IMPORT_PARAM(stream, out->lightSize, temp);
        }
        }
    );
    */
    RegisterComponentDataSerializerDeserializer<CameraLayerMask>(
        {[](ComponentDataBase *data) {
             CameraLayerMask *out = static_cast<CameraLayerMask *>(data);
             std::stringstream stream;
             EXPORT_PARAM(stream, out->m_value);
             return stream.str();
         },
         [](const std::string &data, ComponentDataBase *ptr) {
             std::stringstream stream;
             stream << data;
             CameraLayerMask *out = static_cast<CameraLayerMask *>(ptr);
             char temp;
             IMPORT_PARAM(stream, out->m_value, temp);
         }});
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
    std::unique_ptr<World> &world, YAML::Emitter &out, const Entity &entity)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Entity" << YAML::Value << std::to_string(entity.GetIndex());
    out << YAML::Key << "IsEnabled" << YAML::Value << entity.IsEnabled();
    out << YAML::Key << "ArchetypeName" << YAML::Value
        << EntityManager::GetEntityArchetypeName(EntityManager::GetEntityArchetype(entity));
    out << YAML::Key << "Name" << YAML::Value << entity.GetName();
    out << YAML::Key << "Parent" << YAML::Value << EntityManager::GetParent(entity).GetIndex();
#pragma region ComponentData
    out << YAML::Key << "ComponentData" << YAML::Value << YAML::BeginSeq;
    auto &storage = world->m_worldEntityStorage;
    std::vector<ComponentDataType> &componentTypes =
        storage.m_entityComponentStorage[storage.m_entityInfos[entity.GetIndex()].m_archetypeInfoIndex]
            .m_archetypeInfo->m_componentTypes;
    for (const auto &type : componentTypes)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << type.m_name;
        std::string value;
        const auto it = GetInstance().m_componentDataSerializers.find(type.m_typeId);
        if (it != GetInstance().m_componentDataSerializers.end())
        {
            ComponentDataBase *ptr = EntityManager::GetComponentDataPointer(entity, type.m_typeId);
            value = it->second.first(ptr);
        }
        out << YAML::Key << "Content" << YAML::Value << value;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
#pragma endregion

#pragma region Private Components
    out << YAML::Key << "PrivateComponent" << YAML::Value << YAML::BeginSeq;
    EntityManager::ForEachPrivateComponent(entity, [&](PrivateComponentElement &data) {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << data.m_name;
        out << YAML::Key << "IsEnabled" << YAML::Value << data.m_privateComponentData->m_enabled;
        data.m_privateComponentData->Serialize(out);
        out << YAML::EndMap;
    });
    out << YAML::EndSeq;
#pragma endregion

    out << YAML::EndMap;
}

UniEngine::Entity UniEngine::SerializationManager::DeserializeEntity(
    std::unique_ptr<World> &world, const YAML::Node &node)
{
    const auto entityName = node["Name"].as<std::string>();
    const auto archetypeName = node["ArchetypeName"].as<std::string>();
    auto componentDatum = node["ComponentData"];
    Entity retVal;

    std::vector<std::shared_ptr<ComponentDataBase>> ptrs;
    std::vector<ComponentDataType> types;
    for (const auto &componentData : componentDatum)
    {
        auto name = componentData["Name"].as<std::string>();
        size_t hashCode;
        size_t size;
        auto ptr = ComponentFactory::ProduceComponentData(name, hashCode, size);

        // Deserialize componentData here.
        const auto it = GetInstance().m_componentDataSerializers.find(hashCode);
        if (it != GetInstance().m_componentDataSerializers.end())
        {
            it->second.second(componentData["Content"].as<std::string>(), ptr.get());
        }
        ptrs.push_back(ptr);
        types.emplace_back(name, hashCode, size);
    }
    const EntityArchetype archetype = EntityManager::CreateEntityArchetype(archetypeName, types);
    retVal = EntityManager::CreateEntity(archetype, entityName);
    for (int i = 0; i < ptrs.size(); i++)
    {
        EntityManager::SetComponentData(retVal, types[i].m_typeId, types[i].m_size, ptrs[i].get());
    }

    auto privateComponents = node["PrivateComponent"];
    if (privateComponents)
    {
        for (const auto &privateComponent : privateComponents)
        {
            auto name = privateComponent["Name"].as<std::string>();
            size_t hashCode;
            auto *ptr =
                dynamic_cast<PrivateComponentBase *>(ComponentFactory::ProduceSerializableObject(name, hashCode));
            ptr->Deserialize(privateComponent);
            ptr->m_enabled = privateComponent["IsEnabled"].as<bool>();
            EntityManager::SetPrivateComponent(retVal, name, hashCode, ptr);
        }
    }

    return retVal;
}

void UniEngine::SerializationManager::Serialize(std::unique_ptr<World> &world, const std::string &path)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "World";
    out << YAML::Value << "World_Name";
    out << YAML::Key << "Entities";
    out << YAML::Value << YAML::BeginSeq;
    for (const auto &entity : world->m_worldEntityStorage.m_entities)
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
}

bool UniEngine::SerializationManager::Deserialize(std::unique_ptr<World> &world, const std::string &path)
{
    std::ifstream stream(path);
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node data = YAML::Load(stringStream.str());
    if (!data["World"])
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
            world->m_worldEntityStorage.m_entityInfos[entity.GetIndex()].m_enabled = node["IsEnabled"].as<bool>();
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
            EntityManager::SetParent(entityMap[fst], entityMap[snd]);
        }
    }
    return true;
}