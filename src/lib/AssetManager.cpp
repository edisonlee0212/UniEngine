#include <Application.hpp>
#include <AssetManager.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <MeshRenderer.hpp>
#include <Prefab.hpp>
#include <ProjectManager.hpp>
#include <RenderManager.hpp>
#include <SerializationManager.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <Utilities.hpp>
using namespace UniEngine;

std::shared_ptr<IAsset> AssetManager::Get(const std::string &typeName, const Handle &handle)
{
    auto &assetManager = GetInstance();
    auto search0 = assetManager.m_sharedAssets[typeName].find(handle);
    if (search0 != assetManager.m_sharedAssets[typeName].end())
    {
        if (!search0->second)
        {
            return search0->second;
        }
    }

    auto search1 = assetManager.m_assets.find(handle);
    if (search1 != assetManager.m_assets.end())
    {
        if (!search1->second.expired())
        {
            return search1->second.lock();
        }
    }
    auto &assetRecords = ProjectManager::GetInstance().m_assetRegistry->m_assetRecords;
    auto search2 = assetRecords.find(handle);
    if (search2 != assetRecords.end())
    {
        assert(search2->second.m_typeName == typeName);
        auto retVal = CreateAsset(search2->second.m_typeName, handle, search2->second.m_name);
        retVal->SetPath(search2->second.m_relativeFilePath);
        retVal->Load();
        return retVal;
    }
    return nullptr;
}

void AssetManager::RemoveFromShared(const std::string &typeName, const Handle &handle)
{
    GetInstance().m_sharedAssets[typeName].erase(handle);
}

Entity AssetManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture)
{
    const Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(texture->m_name);
    auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
    mmc->m_material.Set<Material>(LoadMaterial(DefaultResources::GLPrograms::StandardProgram));
    mmc->m_material.Get<Material>()->SetTexture(TextureType::Albedo, texture);
    mmc->m_mesh.Set<Mesh>(DefaultResources::Primitives::Quad);
    return entity;
}

void AssetManager::RegisterAsset(std::shared_ptr<IAsset> resource)
{
    auto &assetManager = GetInstance();
    assetManager.m_assets[resource->m_handle] = resource;
}

std::shared_ptr<Material> AssetManager::LoadMaterial(const std::shared_ptr<OpenGLUtils::GLProgram> &program)
{
    auto retVal = CreateAsset<Material>();
    retVal->SetProgram(program);
    return retVal;
}

std::shared_ptr<OpenGLUtils::GLProgram> AssetManager::LoadProgram(
    const std::shared_ptr<OpenGLUtils::GLShader> &vertex, const std::shared_ptr<OpenGLUtils::GLShader> &fragment)
{
    auto retVal = CreateAsset<OpenGLUtils::GLProgram>();
    retVal->Attach(vertex);
    retVal->Attach(fragment);
    retVal->Link();
    return retVal;
}

std::shared_ptr<OpenGLUtils::GLProgram> AssetManager::LoadProgram(
    const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
    const std::shared_ptr<OpenGLUtils::GLShader> &geometry,
    const std::shared_ptr<OpenGLUtils::GLShader> &fragment)
{
    auto retVal = CreateAsset<OpenGLUtils::GLProgram>();
    retVal->Attach(vertex);
    retVal->Attach(geometry);
    retVal->Attach(fragment);
    retVal->Link();
    return retVal;
}

void AssetManager::OnInspect()
{
    auto &resourceManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Asset Manager", &resourceManager.m_enableAssetMenu);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    if (resourceManager.m_enableAssetMenu)
    {
        ImGui::Begin("Asset Manager");
        if (ImGui::BeginTabBar(
                "##Resource Tab", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            if (ImGui::BeginTabItem("Assets"))
            {
                for (auto &collection : resourceManager.m_sharedAssets)
                {
                    if (ImGui::CollapsingHeader(collection.first.c_str()))
                    {
                        if (ImGui::BeginPopupContextItem((collection.first + "##NewAsset").c_str()))
                        {
                            if (ImGui::Button("New..."))
                            {
                                Share(CreateAsset(collection.first, Handle(), ""));
                            }
                            ImGui::EndPopup();
                        }
                        if (ImGui::BeginDragDropTarget())
                        {
                            const std::string hash = collection.first;
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(hash.c_str()))
                            {
                                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
                                std::shared_ptr<IAsset> payload_n =
                                    *static_cast<std::shared_ptr<IAsset> *>(payload->Data);
                                Share(payload_n);
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if (collection.first == "Prefab")
                        {
                            if (ImGui::BeginDragDropTarget())
                            {
                                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Entity"))
                                {
                                    IM_ASSERT(payload->DataSize == sizeof(Entity));
                                    auto prefab = CreateAsset<Prefab>();
                                    prefab->FromEntity(*static_cast<Entity *>(payload->Data));
                                    resourceManager.m_sharedAssets["Prefab"][prefab->GetHandle()] = prefab;
                                }
                                ImGui::EndDragDropTarget();
                            }
                        }

                        for (auto &i : collection.second)
                        {
                            ImGui::Button(i.second->m_name.c_str());
                            EditorManager::DraggableAsset(i.second);
                            const std::string type = i.second->GetTypeName();
                            const std::string tag = "##" + type + std::to_string(i.second->GetHandle());
                            if (ImGui::BeginPopupContextItem(tag.c_str()))
                            {
                                if (ImGui::Button(("Remove" + tag).c_str()) &&
                                    i.first >= DefaultResources::GetMaxHandle())
                                {
                                    collection.second.erase(i.first);
                                    ImGui::EndPopup();
                                    break;
                                }
                                ImGui::EndPopup();
                            }
                        }
                    }
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}
void AssetManager::Init()
{
    ProjectManager::GetInstance().m_assetRegistry = SerializationManager::ProduceSerializable<AssetRegistry>();
    DefaultResources::Load();

    RegisterExternalAssetTypeExtensions<Prefab>({".obj", ".gltf", ".glb", ".blend", ".ply", ".fbx", ".dae"});
    RegisterExternalAssetTypeExtensions<Texture2D>({".png", ".jpg", ".jpeg", ".tga", ".hdr"});
}

void AssetRegistry::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "AssetRecords" << YAML::Value << YAML::BeginSeq;
    for (const auto &i : m_assetRecords)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Handle" << i.first;
        out << YAML::Key << "RelativeFilePath" << i.second.m_relativeFilePath.string();
        out << YAML::Key << "TypeName" << i.second.m_typeName;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}

void AssetRegistry::Deserialize(const YAML::Node &in)
{
    auto inAssetRecords = in["AssetRecords"];
    m_assetRecords.clear();
    m_fileMap.clear();
    for (const auto &inAssetRecord : inAssetRecords)
    {
        Handle assetHandle(inAssetRecord["Handle"].as<uint64_t>());
        FileRecord assetRecord;
        assetRecord.m_relativeFilePath = inAssetRecord["RelativeFilePath"].as<std::string>();
        assetRecord.m_typeName = inAssetRecord["TypeName"].as<std::string>();
        if (std::filesystem::exists(ProjectManager::GetProjectPath().parent_path() / assetRecord.m_relativeFilePath))
        {
            m_assetRecords.insert({assetHandle, assetRecord});
        }
    }
    for (const auto &i : m_assetRecords)
    {
        m_fileMap[i.second.m_relativeFilePath.string()] = i.first;
    }
}

std::shared_ptr<IAsset> AssetManager::CreateAsset(
    const std::string &typeName, const Handle &handle, const std::string &name)
{
    size_t hashCode;
    auto retVal =
        std::dynamic_pointer_cast<IAsset>(SerializationManager::ProduceSerializable(typeName, hashCode, handle));
    RegisterAsset(retVal);
    retVal->OnCreate();
    if (!name.empty())
        retVal->m_name = name;
    else
    {
        retVal->m_name = "New " + typeName;
    }
    return retVal;
}
std::vector<std::string> AssetManager::GetExtension(const std::string &typeName)
{
    return GetInstance().m_defaultExtensions[typeName];
}

