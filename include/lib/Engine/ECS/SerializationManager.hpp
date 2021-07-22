#pragma once
#include <Entity.hpp>
#include <ISerializable.hpp>
#include <ISingleton.hpp>
#include <World.hpp>
namespace YAML
{
class Node;
class Emitter;
} // namespace YAML

#define EXPORT_PARAM(x, y) (x) << "{" << (y) << "}"
#define IMPORT_PARAM(x, y, temp) (x) >> (temp) >> (y) >> (temp)
namespace UniEngine
{
#pragma region Component Factory
class UNIENGINE_API ComponentFactory : public ISingleton<ComponentFactory>
{
    friend class SerializationManager;
    std::map<std::string, std::function<std::shared_ptr<IDataComponent>(size_t &, size_t &)>> m_dataComponentGenerators;
    std::map<std::string, std::function<ISerializable *(size_t &)>> m_serializableGenerators;

    std::map<size_t, std::string> m_dataComponentNames;
    std::map<size_t, std::string> m_serializableNames;

  public:
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
};
template <typename T> std::string ComponentFactory::GetDataComponentTypeName()
{
    return GetInstance().m_dataComponentNames.find(typeid(T).hash_code())->second;
}
template <typename T> std::string ComponentFactory::GetSerializableTypeName()
{
    return GetInstance().m_serializableNames.find(typeid(T).hash_code())->second;
}
template <typename T> bool ComponentFactory::RegisterDataComponent(const std::string &name)
{
    return RegisterDataComponent(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
        hashCode = typeid(T).hash_code();
        size = sizeof(T);
        return std::move(std::dynamic_pointer_cast<IDataComponent>(std::make_shared<T>()));
    });
}

template <typename T> bool ComponentFactory::RegisterSerializable(const std::string &name)
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
        ComponentFactory::RegisterDataComponent(name, typeid(T).hash_code(), [](size_t &hashCode, size_t &size) {
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
        ComponentFactory::RegisterSerializable(name, typeid(T).hash_code(), [](size_t &hashCode) {
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

std::ostream &operator<<(std::ostream &out, const glm::vec2 &v);
std::ostream &operator<<(std::ostream &out, const glm::vec3 &v);
std::ostream &operator<<(std::ostream &out, const glm::vec4 &v);
std::ostream &operator<<(std::ostream &out, const glm::mat4 &v);

std::istream &operator>>(std::istream &in, glm::vec2 &v);
std::istream &operator>>(std::istream &in, glm::vec3 &v);
std::istream &operator>>(std::istream &in, glm::vec4 &v);
std::istream &operator>>(std::istream &in, glm::mat4 &v);

class UNIENGINE_API SerializationManager : public ISingleton<SerializationManager>
{
    std::unordered_map<
        size_t,
        std::pair<
            std::function<std::string(IDataComponent *)>,
            std::function<void(const std::string &, IDataComponent *)>>>
        m_componentDataSerializers;
    static void SerializeEntity(std::unique_ptr<World> &world, YAML::Emitter &out, const Entity &entity);
    static Entity DeserializeEntity(std::unique_ptr<World> &world, const YAML::Node &node);

  public:
    static void Serialize(std::unique_ptr<World> &world, const std::string &path);
    static bool Deserialize(std::unique_ptr<World> &world, const std::string &path);
};

} // namespace UniEngine
