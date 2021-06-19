#pragma once
#include <EntityManager.hpp>
#include <Serializable.hpp>
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
    std::unordered_map<std::string, std::function<std::shared_ptr<ComponentDataBase>(size_t &, size_t &)>>
        m_componentDataGenerators;
    std::unordered_map<std::string, std::function<Serializable *(size_t &)>> m_classComponentGenerators;

  public:
    template <typename T = Serializable> static bool RegisterComponentData();
    template <typename T = ComponentDataBase> static bool RegisterSerializable();
    static bool Register(
        const std::string &id, const std::function<std::shared_ptr<ComponentDataBase>(size_t &, size_t &)> &func);
    static std::shared_ptr<ComponentDataBase> ProduceComponentData(
        const std::string &id, size_t &hashCode, size_t &size);
    static bool Register(const std::string &id, const std::function<Serializable *(size_t &)> &func);
    static Serializable *ProduceSerializableObject(const std::string &id, size_t &hashCode);
};

template <typename T> bool ComponentFactory::RegisterComponentData()
{
    std::string id = typeid(T).name();
    return Register(id, [](size_t &hashCode, size_t &size) {
        hashCode = typeid(T).hash_code();
        size = sizeof(T);
        return std::move(std::dynamic_pointer_cast<ComponentDataBase>(std::make_shared<T>()));
    });
}

template <typename T> bool ComponentFactory::RegisterSerializable()
{
    std::string id = typeid(T).name();
    return Register(id, [](size_t &hashCode) {
        hashCode = typeid(T).hash_code();
        return dynamic_cast<Serializable *>(new T());
    });
}

template <typename T = ComponentDataBase> class UNIENGINE_API ComponentDataRegistration
{
  public:
    ComponentDataRegistration(int none)
    {
        std::string id = typeid(T).name();
        ComponentFactory::Register(id, [](size_t &hashCode, size_t &size) {
            hashCode = typeid(T).hash_code();
            size = sizeof(T);
            return std::move(std::dynamic_pointer_cast<ComponentDataBase>(std::make_shared<T>()));
        });
    }
};

template <typename T = Serializable> class UNIENGINE_API SerializableRegistration
{
  public:
    SerializableRegistration(int none)
    {
        std::string id = typeid(T).name();
        ComponentFactory::Register(id, [](size_t &hashCode) {
            hashCode = typeid(T).hash_code();
            return dynamic_cast<Serializable *>(new T());
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
            std::function<std::string(ComponentDataBase *)>,
            std::function<void(const std::string &, ComponentDataBase *)>>>
        m_componentDataSerializers;
    static void SerializeEntity(std::unique_ptr<World> &world, YAML::Emitter &out, const Entity &entity);
    static Entity DeserializeEntity(std::unique_ptr<World> &world, const YAML::Node &node);

  public:
    template <typename T = ComponentDataBase>
    static bool RegisterComponentDataSerializerDeserializer(
        const std::pair<
            std::function<std::string(ComponentDataBase *)>,
            std::function<void(const std::string &, ComponentDataBase *)>> &funcPair);
    static void Serialize(std::unique_ptr<World> &world, const std::string &path);
    static bool Deserialize(std::unique_ptr<World> &world, const std::string &path);
    static void Init();
};

template <typename T>
bool SerializationManager::RegisterComponentDataSerializerDeserializer(
    const std::pair<
        std::function<std::string(ComponentDataBase *)>,
        std::function<void(const std::string &, ComponentDataBase *)>> &funcPair)
{
    return GetInstance().m_componentDataSerializers.insert({typeid(T).hash_code(), funcPair}).second;
}
} // namespace UniEngine
