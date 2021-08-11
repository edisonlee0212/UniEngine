#pragma once
#include "ISystem.hpp"
#include <Entity.hpp>
#include <ISerializable.hpp>
#include <ISingleton.hpp>
#include <Scene.hpp>

namespace YAML
{
class Node;
class Emitter;
template <> struct convert<glm::vec2>
{
    static Node encode(const glm::vec2 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node &node, glm::vec2 &rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
        {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
    }
};
template <> struct convert<glm::vec3>
{
    static Node encode(const glm::vec3 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node &node, glm::vec3 &rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
template <> struct convert<glm::vec4>
{
    static Node encode(const glm::vec4 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node &node, glm::vec4 &rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
        {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};
template <> struct convert<glm::mat4>
{
    static Node encode(const glm::mat4 &rhs)
    {
        Node node;
        node.push_back(rhs[0]);
        node.push_back(rhs[1]);
        node.push_back(rhs[2]);
        node.push_back(rhs[3]);
        return node;
    }

    static bool decode(const Node &node, glm::mat4 &rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
        {
            return false;
        }

        rhs[0] = node[0].as<glm::vec4>();
        rhs[1] = node[1].as<glm::vec4>();
        rhs[2] = node[2].as<glm::vec4>();
        rhs[3] = node[3].as<glm::vec4>();
        return true;
    }
};
} // namespace YAML

#define EXPORT_PARAM(x, y) (x) << "{" << (y) << "}"
#define IMPORT_PARAM(x, y, temp) (x) >> (temp) >> (y) >> (temp)
namespace UniEngine
{
#pragma region Component Factory
class UNIENGINE_API SerializationManager : public ISingleton<SerializationManager>
{
    friend class AssetManager;
    friend class ISerializable;
    std::map<std::string, std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)>> m_dataComponentGenerators;
    std::map<std::string, std::function<std::shared_ptr<ISerializable>(size_t &)>> m_serializableGenerators;

    std::map<std::string, size_t> m_dataComponentIds;
    std::map<size_t, std::string> m_dataComponentNames;
    std::map<size_t, std::string> m_serializableNames;

  public:
    template <typename T = ISerializable> static bool RegisterDataComponentType(const std::string &name);
    template <typename T = IDataComponent> static bool RegisterSerializableType(const std::string &name);
    static bool RegisterDataComponentType(
        const std::string &typeName,
        const size_t &typeId,
        const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func);
    static std::shared_ptr<IDataComponent> ProduceDataComponent(
        const std::string &typeName, size_t &hashCode, size_t &size);

    static bool RegisterSerializableType(
        const std::string &typeName, const size_t &typeId, const std::function<std::shared_ptr<ISerializable> (size_t &)> &func);
    static std::shared_ptr<ISerializable> ProduceSerializable(const std::string &typeName, size_t &hashCode);
    static std::shared_ptr<ISerializable> ProduceSerializable(const std::string &typeName, size_t &hashCode, const Handle& handle);
    template <typename T = ISerializable> static std::shared_ptr<T> ProduceSerializable();
    template <typename T = IDataComponent> static std::string GetDataComponentTypeName();
    template <typename T = ISerializable> static std::string GetSerializableTypeName();
    static std::string GetSerializableTypeName(const size_t &typeId);

    static size_t GetDataComponentTypeId(const std::string &typeName);
};
template <typename T> std::string SerializationManager::GetDataComponentTypeName()
{
    return GetInstance().m_dataComponentNames.find(typeid(T).hash_code())->second;
}
template <typename T> std::string SerializationManager::GetSerializableTypeName()
{
    return GetInstance().m_serializableNames.find(typeid(T).hash_code())->second;
}
template <typename T> bool SerializationManager::RegisterDataComponentType(const std::string &name)
{
    return RegisterDataComponentType(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
        hashCode = typeid(T).hash_code();
        size = sizeof(T);
        return std::move(std::dynamic_pointer_cast<IDataComponent>(std::make_shared<T>()));
    });
}

template <typename T> bool SerializationManager::RegisterSerializableType(const std::string &name)
{
    return RegisterSerializableType(name, typeid(T).hash_code(), [](size_t &hashCode) {
        hashCode = typeid(T).hash_code();
        auto ptr = std::static_pointer_cast<ISerializable>(std::make_shared<T>());
        return ptr;
    });
}

template <typename T = IDataComponent> class UNIENGINE_API ComponentDataRegistration
{
  public:
    ComponentDataRegistration(const std::string &name)
    {
        SerializationManager::RegisterDataComponentType<T>(name);
    }
};

template <typename T = ISerializable> class UNIENGINE_API SerializableRegistration
{
  public:
    SerializableRegistration(const std::string &name)
    {
        SerializationManager::RegisterSerializableType<T>(name);
    }
};

#pragma endregion

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::mat4 &v);

template <typename T> std::shared_ptr<T> SerializationManager::ProduceSerializable()
{
    auto& serializationManager = GetInstance();
    const auto typeName = GetSerializableTypeName<T>();
    const auto it = serializationManager.m_serializableGenerators.find(typeName);
    if (it != serializationManager.m_serializableGenerators.end())
    {
        size_t hashCode;
        auto retVal = it->second(hashCode);
        retVal->m_typeName = typeName;
        return std::move(std::static_pointer_cast<T>(retVal));
    }
    UNIENGINE_ERROR("PrivateComponent " + typeName + "is not registered!");
    throw 1;
}

} // namespace UniEngine
