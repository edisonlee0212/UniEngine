#pragma once
#include <IHandle.hpp>
#include <uniengine_export.h>
namespace YAML
{
class Emitter;
class Node;
} // namespace YAML

namespace UniEngine
{
class UNIENGINE_API ISerializable : public IHandle
{
    friend class Serialization;
    friend class IAsset;
    friend class Entities;
    friend class Scene;
    friend class Serialization;
    friend class EntityMetadata;
    friend class AssetRecord;
    friend class Folder;
    std::string m_typeName;
  public:
    void Serialize(const std::string &name, YAML::Emitter &out);
    void Deserialize(const std::string &name, const YAML::Node &in);
    [[nodiscard]] std::string GetTypeName()
    {
        return m_typeName;
    }
    virtual ~ISerializable() = default;
    virtual void Serialize(YAML::Emitter &out)
    {
    }
    virtual void Deserialize(const YAML::Node &in)
    {
    }
};
} // namespace UniEngine
