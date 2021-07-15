#pragma once
#include <uniengine_export.h>
namespace YAML
{
class Emitter;
class Node;
} // namespace YAML

namespace UniEngine
{
class UNIENGINE_API ISerializable
{
  public:
    virtual ~ISerializable() = default;
    virtual void Serialize(YAML::Emitter &out)
    {
    }
    virtual void Deserialize(const YAML::Node &in)
    {
    }
};
} // namespace UniEngine
