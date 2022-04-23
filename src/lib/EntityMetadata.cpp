//
// Created by lllll on 8/13/2021.
//

#include "EntityMetadata.hpp"
#include "Serialization.hpp"
#include "Scene.hpp"
using namespace UniEngine;

void EntityMetadata::Deserialize(const YAML::Node &in, const std::shared_ptr<Scene> &scene)
{
    m_name = in["m_name"].as<std::string>();
    m_version = 1;
    m_enabled = in["m_enabled"].as<bool>();
    m_static = in["m_static"].as<bool>();
    m_handle.m_value = in["m_handle"].as<uint64_t>();
}

void EntityMetadata::Serialize(YAML::Emitter &out, const std::shared_ptr<Scene> &scene)
{
    out << YAML::BeginMap;
    {
        out << YAML::Key << "m_name" << YAML::Value << m_name;
        out << YAML::Key << "m_handle" << YAML::Value << m_handle.m_value;
        out << YAML::Key << "m_enabled" << YAML::Value << m_enabled;
        out << YAML::Key << "m_static" << YAML::Value << m_static;
        if(m_parent.GetIndex() != 0) out << YAML::Key << "Parent.Handle" << YAML::Value << scene->GetEntityHandle(m_parent);
        if(m_root.GetIndex() != 0)out << YAML::Key << "Root.Handle" << YAML::Value << scene->GetEntityHandle(m_root);

#pragma region Private Components
        out << YAML::Key << "m_privateComponentElements" << YAML::Value << YAML::BeginSeq;
        for (const auto &element : m_privateComponentElements)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "m_typeName" << YAML::Value << element.m_privateComponentData->m_typeName;
            out << YAML::Key << "m_enabled" << YAML::Value << element.m_privateComponentData->m_enabled;
            element.m_privateComponentData->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
#pragma endregion
    }
    out << YAML::EndMap;
}

void EntityMetadata::Clone(const std::unordered_map<Handle, Handle> &entityMap, const EntityMetadata &source, const std::shared_ptr<Scene> &scene)
{
    m_handle = source.m_handle;
    m_name = source.m_name;
    m_version = source.m_version;
    m_enabled = source.m_enabled;
    m_parent = source.m_parent;
    m_root = source.m_root;
    m_static = source.m_static;
    m_dataComponentStorageIndex = source.m_dataComponentStorageIndex;
    m_chunkArrayIndex = source.m_chunkArrayIndex;
    m_children = source.m_children;
    m_privateComponentElements.resize(source.m_privateComponentElements.size());
    for(int i = 0; i < m_privateComponentElements.size(); i++)
    {
        m_privateComponentElements[i].m_privateComponentData =
            std::dynamic_pointer_cast<IPrivateComponent>(Serialization::ProduceSerializable(
                source.m_privateComponentElements[i].m_privateComponentData->GetTypeName(),
                m_privateComponentElements[i].m_typeId));
        m_privateComponentElements[i].m_privateComponentData->m_scene = scene;
        m_privateComponentElements[i].m_privateComponentData->m_owner = source.m_privateComponentElements[i].m_privateComponentData->m_owner;
        m_privateComponentElements[i].m_privateComponentData->OnCreate();
        Serialization::ClonePrivateComponent(
            m_privateComponentElements[i].m_privateComponentData,
            source.m_privateComponentElements[i].m_privateComponentData);
        m_privateComponentElements[i].m_privateComponentData->m_scene = scene;
        m_privateComponentElements[i].m_privateComponentData->Relink(entityMap, scene);
    }
}
