#include <ISerializable.hpp>
#include "Serialization.hpp"
using namespace UniEngine;
void ISerializable::Serialize(const std::string &name, YAML::Emitter &out)
{
    out << YAML::Key << name << YAML::Value << YAML::BeginMap;
    {
        Serialize(out);
    }
    out << YAML::EndMap;
}
void ISerializable::Deserialize(const std::string &name, const YAML::Node &in)
{
    if (in[name]) {
        const auto &cd = in[name];
        Deserialize(cd);
    }
}
