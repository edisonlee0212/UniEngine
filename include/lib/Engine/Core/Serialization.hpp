#pragma once
#include "ISystem.hpp"
#include "Console.hpp"
#include "IDataComponent.hpp"
#include "IPrivateComponent.hpp"
#include "ISerializable.hpp"
#include "ISingleton.hpp"
namespace YAML
{
class Node;
class Emitter;
template <> struct UNIENGINE_API convert<glm::vec2>
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

template <> struct UNIENGINE_API convert<glm::vec3>
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
template <> struct UNIENGINE_API convert<glm::vec4>
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
template <> struct UNIENGINE_API convert<glm::quat>
{
    static Node encode(const glm::quat &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node &node, glm::quat &rhs)
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
template <> struct UNIENGINE_API convert<glm::mat4>
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
template <> struct UNIENGINE_API convert<glm::dvec2>
{
    static Node encode(const glm::dvec2 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node &node, glm::dvec2 &rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
        {
            return false;
        }

        rhs.x = node[0].as<double>();
        rhs.y = node[1].as<double>();
        return true;
    }
};

template <> struct UNIENGINE_API convert<glm::dvec3>
{
    static Node encode(const glm::dvec3 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node &node, glm::dvec3 &rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<double>();
        rhs.y = node[1].as<double>();
        rhs.z = node[2].as<double>();
        return true;
    }
};
template <> struct UNIENGINE_API convert<glm::dvec4>
{
    static Node encode(const glm::dvec4 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node &node, glm::dvec4 &rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
        {
            return false;
        }

        rhs.x = node[0].as<double>();
        rhs.y = node[1].as<double>();
        rhs.z = node[2].as<double>();
        rhs.w = node[3].as<double>();
        return true;
    }
};

template <> struct UNIENGINE_API convert<glm::ivec2>
{
    static Node encode(const glm::ivec2 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node &node, glm::ivec2 &rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
        {
            return false;
        }

        rhs.x = node[0].as<int>();
        rhs.y = node[1].as<int>();
        return true;
    }
};

template <> struct UNIENGINE_API convert<glm::ivec3>
{
    static Node encode(const glm::ivec3 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node &node, glm::ivec3 &rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<int>();
        rhs.y = node[1].as<int>();
        rhs.z = node[2].as<int>();
        return true;
    }
};
template <> struct UNIENGINE_API convert<glm::ivec4>
{
    static Node encode(const glm::ivec4 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node &node, glm::ivec4 &rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
        {
            return false;
        }

        rhs.x = node[0].as<int>();
        rhs.y = node[1].as<int>();
        rhs.z = node[2].as<int>();
        rhs.w = node[3].as<int>();
        return true;
    }
};
template <> struct UNIENGINE_API convert<glm::uvec2>
{
    static Node encode(const glm::uvec2 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node &node, glm::uvec2 &rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
        {
            return false;
        }

        rhs.x = node[0].as<unsigned>();
        rhs.y = node[1].as<unsigned>();
        return true;
    }
};

template <> struct UNIENGINE_API convert<glm::uvec3>
{
    static Node encode(const glm::uvec3 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node &node, glm::uvec3 &rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<unsigned>();
        rhs.y = node[1].as<unsigned>();
        rhs.z = node[2].as<unsigned>();
        return true;
    }
};
template <> struct UNIENGINE_API convert<glm::uvec4>
{
    static Node encode(const glm::uvec4 &rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(const Node &node, glm::uvec4 &rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
        {
            return false;
        }

        rhs.x = node[0].as<unsigned>();
        rhs.y = node[1].as<unsigned>();
        rhs.z = node[2].as<unsigned>();
        rhs.w = node[3].as<unsigned>();
        return true;
    }
};
} // namespace YAML
#define EXPORT_PARAM(x, y) (x) << "{" << (y) << "}"
#define IMPORT_PARAM(x, y, temp) (x) >> (temp) >> (y) >> (temp)
namespace UniEngine
{
#pragma region Component Factory
class UNIENGINE_API Serialization : public ISingleton<Serialization>
{
    friend class ISerializable;
    friend class ProjectManager;
    friend class ClassRegistry;
    std::map<std::string, std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)>> m_dataComponentGenerators;
    std::map<std::string, std::function<std::shared_ptr<ISerializable>(size_t &)>> m_serializableGenerators;
    std::map<
        std::string,
        std::function<void(std::shared_ptr<IPrivateComponent>, const std::shared_ptr<IPrivateComponent> &)>>
        m_privateComponentCloners;
    std::map<std::string, std::function<void(std::shared_ptr<ISystem>, const std::shared_ptr<ISystem> &)>>
        m_systemCloners;
    std::map<std::string, size_t> m_dataComponentIds;
    std::map<std::string, size_t> m_serializableIds;
    std::map<size_t, std::string> m_dataComponentNames;
    std::map<size_t, std::string> m_serializableNames;
    template <typename T = IDataComponent> static bool RegisterDataComponentType(const std::string &name);
    template <typename T = ISerializable> static bool RegisterSerializableType(const std::string &name);
    template <typename T = IPrivateComponent> static bool RegisterPrivateComponentType(const std::string &name);
    template <typename T = ISystem> static bool RegisterSystemType(const std::string &name);
    static bool RegisterDataComponentType(
        const std::string &typeName,
        const size_t &typeId,
        const std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)> &func);
    static bool RegisterSerializableType(
        const std::string &typeName,
        const size_t &typeId,
        const std::function<std::shared_ptr<ISerializable>(size_t &)> &func);

    static bool RegisterPrivateComponentType(
        const std::string &typeName,
        const size_t &typeId,
        const std::function<void(std::shared_ptr<IPrivateComponent>, const std::shared_ptr<IPrivateComponent> &)>
            &cloneFunc);
    static bool RegisterSystemType(
        const std::string &typeName,
        const std::function<void(std::shared_ptr<ISystem>, const std::shared_ptr<ISystem> &)> &cloneFunc);

  public:
    static std::shared_ptr<IDataComponent> ProduceDataComponent(
        const std::string &typeName, size_t &hashCode, size_t &size);
    static void ClonePrivateComponent(
        std::shared_ptr<IPrivateComponent> target, const std::shared_ptr<IPrivateComponent> &source);
    static void CloneSystem(std::shared_ptr<ISystem> target, const std::shared_ptr<ISystem> &source);
    static std::shared_ptr<ISerializable> ProduceSerializable(const std::string &typeName, size_t &hashCode);
    static std::shared_ptr<ISerializable> ProduceSerializable(
        const std::string &typeName, size_t &hashCode, const Handle &handle);
    template <typename T = ISerializable> static std::shared_ptr<T> ProduceSerializable();
    template <typename T = IDataComponent> static std::string GetDataComponentTypeName();
    template <typename T = ISerializable> static std::string GetSerializableTypeName();
    static std::string GetSerializableTypeName(const size_t &typeId);
    static bool HasSerializableType(const std::string &typeName);
    static bool HasComponentDataType(const std::string &typeName);
    static size_t GetSerializableTypeId(const std::string &typeName);
    static size_t GetDataComponentTypeId(const std::string &typeName);
};

template <typename T> std::string Serialization::GetDataComponentTypeName()
{
    return GetInstance().m_dataComponentNames.find(typeid(T).hash_code())->second;
}
template <typename T> std::string Serialization::GetSerializableTypeName()
{
    return GetInstance().m_serializableNames.find(typeid(T).hash_code())->second;
}
template <typename T> bool Serialization::RegisterDataComponentType(const std::string &name)
{
    return RegisterDataComponentType(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
        hashCode = typeid(T).hash_code();
        size = sizeof(T);
        return std::move(std::dynamic_pointer_cast<IDataComponent>(std::make_shared<T>()));
    });
}

template <typename T> bool Serialization::RegisterSerializableType(const std::string &name)
{
    return RegisterSerializableType(name, typeid(T).hash_code(), [](size_t &hashCode) {
        hashCode = typeid(T).hash_code();
        auto ptr = std::static_pointer_cast<ISerializable>(std::make_shared<T>());
        return ptr;
    });
}
template <typename T> bool Serialization::RegisterPrivateComponentType(const std::string &name)
{
    return RegisterPrivateComponentType(
        name,
        typeid(T).hash_code(),
        [](std::shared_ptr<IPrivateComponent> target, const std::shared_ptr<IPrivateComponent> &source) {
            target->m_handle = source->m_handle;
            target->m_enabled = source->m_enabled;
            target->m_owner = source->m_owner;
            *std::dynamic_pointer_cast<T>(target) = *std::dynamic_pointer_cast<T>(source);
            target->m_started = false;
            target->PostCloneAction(source);
        });
}
template <typename T> bool Serialization::RegisterSystemType(const std::string &name)
{
    return RegisterSystemType(name, [](std::shared_ptr<ISystem> target, const std::shared_ptr<ISystem> &source) {
        target->m_handle = source->m_handle;
        target->m_rank = source->m_rank;
        target->m_enabled = source->m_enabled;
        *std::dynamic_pointer_cast<T>(target) = *std::dynamic_pointer_cast<T>(source);
        target->m_started = false;
        target->PostCloneAction(source);
    });
}
#pragma endregion

UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v);

UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::quat &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::mat4 &v);

UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::dvec2 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::dvec3 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::dvec4 &v);

UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::ivec2 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::ivec3 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::ivec4 &v);

UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::uvec2 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::uvec3 &v);
UNIENGINE_API YAML::Emitter &operator<<(YAML::Emitter &out, const glm::uvec4 &v);

template <typename T> std::shared_ptr<T> Serialization::ProduceSerializable()
{
    auto &serializationManager = GetInstance();
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
