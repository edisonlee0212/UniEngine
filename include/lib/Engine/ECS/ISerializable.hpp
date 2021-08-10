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
    friend class SerializableFactory;
    friend class AssetManager;
    friend class EntityManager;
    friend class SerializationManager;
    std::string m_typeName;
  public:
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
