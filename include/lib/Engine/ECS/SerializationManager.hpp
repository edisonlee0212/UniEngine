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
class UNIENGINE_API SerializableFactory : public ISingleton<SerializableFactory>
{
    friend class SerializationManager;
    std::map<std::string, std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)>> m_dataComponentGenerators;
    std::map<std::string, std::function<ISerializable *(size_t &)>> m_serializableGenerators;

    std::map<std::string, size_t> m_dataComponentIds;
    std::map<size_t, std::string> m_dataComponentNames;
    std::map<size_t, std::string> m_serializableNames;

  public:
    template <typename T = ISerializable>
    static void Serialize(const std::string &filePath, const std::shared_ptr<T> &target);
    template <typename T = ISerializable>
    static void Deserialize(const std::string &filePath, std::shared_ptr<T> &target);

    template <typename T = ISerializable> static bool RegisterDataComponent(const std::string &name);
    template <typename T = IDataComponent> static bool RegisterSerializable(const std::string &name);
    static bool RegisterDataComponent(
        const std::string &typeName,
        const size_t &typeId,
        const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func);
    static std::shared_ptr<IDataComponent> ProduceDataComponent(
        const std::string &typeName, size_t &hashCode, size_t &size);

    static bool RegisterSerializable(
        const std::string &typeName, const size_t &typeId, const std::function<ISerializable *(size_t &)> &func);
    static ISerializable *ProduceSerializable(const std::string &typeName, size_t &hashCode);
    template <typename T = IDataComponent> static std::string GetDataComponentTypeName();
    template <typename T = ISerializable> static std::string GetSerializableTypeName();
    static std::string GetSerializableTypeName(const size_t &typeId);

    static size_t GetDataComponentTypeId(const std::string &typeName);
};
template <typename T> std::string SerializableFactory::GetDataComponentTypeName()
{
    return GetInstance().m_dataComponentNames.find(typeid(T).hash_code())->second;
}
template <typename T> std::string SerializableFactory::GetSerializableTypeName()
{
    return GetInstance().m_serializableNames.find(typeid(T).hash_code())->second;
}
template <typename T> bool SerializableFactory::RegisterDataComponent(const std::string &name)
{
    return RegisterDataComponent(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
        hashCode = typeid(T).hash_code();
        size = sizeof(T);
        return std::move(std::dynamic_pointer_cast<IDataComponent>(std::make_shared<T>()));
    });
}

template <typename T> bool SerializableFactory::RegisterSerializable(const std::string &name)
{
    return RegisterSerializable(name, typeid(T).hash_code(), [](size_t &hashCode) {
        hashCode = typeid(T).hash_code();
        return dynamic_cast<ISerializable *>(new T());
    });
}

template <typename T = IDataComponent> class UNIENGINE_API ComponentDataRegistration
{
  public:
    ComponentDataRegistration(const std::string &name)
    {
        SerializableFactory::RegisterDataComponent(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
            hashCode = typeid(T).hash_code();
            size = sizeof(T);
            return std::move(std::dynamic_pointer_cast<IDataComponent>(std::make_shared<T>()));
        });
    }
};

template <typename T = ISerializable> class UNIENGINE_API SerializableRegistration
{
  public:
    SerializableRegistration(const std::string &name)
    {
        SerializableFactory::RegisterSerializable(name, typeid(T).hash_code(), [](size_t &hashCode) {
            hashCode = typeid(T).hash_code();
            return dynamic_cast<ISerializable *>(new T());
        });
    }
};

#pragma endregion

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v);
YAML::Emitter &operator<<(YAML::Emitter &out, const glm::mat4 &v);


template <typename T> void SerializableFactory::Serialize(const std::string &filePath, const std::shared_ptr<T> &target)
{
    auto ptr = std::dynamic_pointer_cast<ISerializable>(target);
    YAML::Emitter out;
    ptr->Serialize(out);
    std::ofstream fout(filePath);
    fout << out.c_str();
    fout.flush();
}
template <typename T> void SerializableFactory::Deserialize(const std::string &filePath, std::shared_ptr<T> &target)
{
    auto ptr = std::dynamic_pointer_cast<ISerializable>(target);
    std::ifstream stream(filePath);
    std::stringstream stringStream;
    stringStream << stream.rdbuf();
    YAML::Node in = YAML::Load(stringStream.str());
    ptr->Deserialize(in);
}
} // namespace UniEngine
