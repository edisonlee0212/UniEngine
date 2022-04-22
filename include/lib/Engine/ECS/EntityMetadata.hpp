#pragma once
#include <ISerializable.hpp>
#include <IPrivateComponent.hpp>
#include <Entity.hpp>
namespace UniEngine
{
class Scene;

struct EntityMetadata
{
    std::string m_name;
    bool m_static = false;
    unsigned m_version = 1;
    bool m_enabled = true;
    Entity m_parent = Entity();
    Entity m_root = Entity();
    std::vector<PrivateComponentElement> m_privateComponentElements;
    std::vector<Entity> m_children;
    size_t m_dataComponentStorageIndex = 0;
    size_t m_chunkArrayIndex = 0;
    Handle m_handle;
    void Serialize(YAML::Emitter &out, const std::shared_ptr<Scene> &scene);
    void Deserialize(const YAML::Node &in, const std::shared_ptr<Scene> &scene);
    void Clone(const std::unordered_map<Handle, Handle> &entityMap, const EntityMetadata &source, const std::shared_ptr<Scene> &scene);
};

} // namespace UniEngine