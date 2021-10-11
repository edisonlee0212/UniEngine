#pragma once
#include <ISerializable.hpp>
#include <IPrivateComponent.hpp>
#include <Entity.hpp>
namespace UniEngine
{
struct EntityMetadata : public ISerializable
{
    std::string m_name;
    unsigned m_version = 1;
    bool m_enabled = true;
    Entity m_parent = Entity();
    std::vector<PrivateComponentElement> m_privateComponentElements;
    std::vector<Entity> m_children;
    size_t m_dataComponentStorageIndex = 0;
    size_t m_chunkArrayIndex = 0;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

    EntityMetadata &operator=(const EntityMetadata &source);
};

} // namespace UniEngine