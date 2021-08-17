#include <ISerializable.hpp>
#include <SerializationManager.hpp>
#include <Debug.hpp>
using namespace UniEngine;

std::string SerializationManager::GetSerializableTypeName(const size_t &typeId)
{
    return GetInstance().m_serializableNames.find(typeId)->second;
}

bool SerializationManager::RegisterDataComponentType(
    const std::string &typeName,
    const size_t &typeId,
    const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func)
{
    if (GetInstance().m_dataComponentNames.find(typeId) != GetInstance().m_dataComponentNames.end())
    {
        UNIENGINE_ERROR("DataComponent already registered!");
        return false;
    }
    GetInstance().m_dataComponentNames[typeId] = typeName;
    GetInstance().m_dataComponentIds[typeName] = typeId;
    return GetInstance().m_dataComponentGenerators.insert({typeName, func}).second;
}

std::shared_ptr<IDataComponent> SerializationManager::ProduceDataComponent(
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

bool SerializationManager::RegisterSerializableType(
    const std::string &typeName,
    const size_t &typeId,
    const std::function<std::shared_ptr<ISerializable>(size_t &)> &func)
{
    if (GetInstance().m_serializableNames.find(typeId) != GetInstance().m_serializableNames.end())
    {
        UNIENGINE_ERROR("Serializable already registered!");
        return false;
    }
    GetInstance().m_serializableNames[typeId] = typeName;
    GetInstance().m_serializableIds[typeName] = typeId;
    return GetInstance().m_serializableGenerators.insert({typeName, func}).second;
}

std::shared_ptr<ISerializable> SerializationManager::ProduceSerializable(const std::string &typeName, size_t &hashCode)
{
    auto &serializationManager = GetInstance();
    const auto it = serializationManager.m_serializableGenerators.find(typeName);
    if (it != serializationManager.m_serializableGenerators.end())
    {
        auto retVal = it->second(hashCode);
        retVal->m_typeName = typeName;
        return std::move(retVal);
    }
    UNIENGINE_ERROR("PrivateComponent " + typeName + "is not registered!");
    throw 1;
}
std::shared_ptr<ISerializable> SerializationManager::ProduceSerializable(
    const std::string &typeName, size_t &hashCode, const Handle &handle)
{
    auto &serializationManager = GetInstance();
    const auto it = serializationManager.m_serializableGenerators.find(typeName);
    if (it != serializationManager.m_serializableGenerators.end())
    {
        auto retVal = it->second(hashCode);
        retVal->m_typeName = typeName;
        retVal->m_handle = handle;
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

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::quat &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

YAML::Emitter &UniEngine::operator<<(YAML::Emitter &out, const glm::mat4 &v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v[0] << v[1] << v[2] << v[3] << YAML::EndSeq;
    return out;
}
size_t SerializationManager::GetDataComponentTypeId(const std::string &typeName)
{
    return GetInstance().m_dataComponentIds[typeName];
}

size_t SerializationManager::GetSerializableTypeId(const std::string &typeName)
{
    return GetInstance().m_serializableIds[typeName];
}