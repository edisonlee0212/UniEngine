//
// Created by lllll on 8/13/2021.
//

#include "EntityMetadata.hpp"
using namespace UniEngine;
void EntityMetadata::Deserialize(const YAML::Node &in)
{
    m_name = in["Name"].as<std::string>();
    m_version = in["Version"].as<unsigned>();
    m_enabled = in["Enabled"].as<bool>();
    m_handle.m_value = in["Handle"].as<uint64_t>();
    Entity parent;
    parent.m_index = in["Parent.Index"].as<unsigned>();
    parent.m_version = in["Parent.Version"].as<unsigned>();
    m_parent = parent;
    if (in["Children"].IsDefined())
    {
        YAML::Binary childrenData = in["Children"].as<YAML::Binary>();
        const unsigned char *data = childrenData.data();
        std::size_t size = childrenData.size();
        m_children.resize(size / sizeof(Entity));
        std::memcpy(m_children.data(), data, size);
    }
    m_dataComponentStorageIndex = in["DataComponentStorageIndex"].as<size_t>();
    m_chunkArrayIndex = in["ChunkArrayIndex"].as<size_t>();
}

void EntityMetadata::Serialize(YAML::Emitter &out)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "Name" << YAML::Value << m_name;
        out << YAML::Key << "Handle" << YAML::Value << m_handle.m_value;
        out << YAML::Key << "Version" << YAML::Value << m_version;
        out << YAML::Key << "Enabled" << YAML::Value << m_enabled;
        out << YAML::Key << "Parent.Index" << YAML::Value << m_parent.m_index;
        out << YAML::Key << "Parent.Version" << YAML::Value << m_parent.m_version;

        if (!m_children.empty())
        {
            out << YAML::Key << "Children" << YAML::Value
            << YAML::Binary(
                (const unsigned char *)m_children.data(),
                m_children.size() * sizeof(Entity));
        }
        out << YAML::Key << "DataComponentStorageIndex" << YAML::Value << m_dataComponentStorageIndex;
        out << YAML::Key << "ChunkArrayIndex" << YAML::Value << m_chunkArrayIndex;
#pragma region Private Components
        out << YAML::Key << "PrivateComponent" << YAML::Value << YAML::BeginSeq;
        for (const auto &element : m_privateComponentElements)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "TypeName" << YAML::Value << element.m_privateComponentData->GetTypeName();
            out << YAML::Key << "Enabled" << YAML::Value << element.m_privateComponentData->IsEnabled();
            element.m_privateComponentData->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
#pragma endregion
    }
    out << YAML::EndMap;
}