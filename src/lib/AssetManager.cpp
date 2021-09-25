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
    FileRecord fileRecord;
    if(ProjectManager::GetInstance().m_assetRegistry.Find(handle, fileRecord)){
        auto retVal = CreateAsset(fileRecord.m_typeName, handle, fileRecord.m_name);
        retVal->SetPath(fileRecord.m_relativeFilePath);
        retVal->Load();
        assetManager.m_assets[handle] = retVal;
        return retVal;
    }
    return nullptr;
}
std::shared_ptr<IAsset> AssetManager::Get(const Handle &handle)
{
    auto &assetManager = GetInstance();
    for (auto &i : assetManager.m_sharedAssets)
    {
        auto search0 = i.second.find(handle);
        if (search0 != i.second.end())
        {
            if (!search0->second)
            {
                return search0->second;
            }
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
    FileRecord fileRecord;
    if(ProjectManager::GetInstance().m_assetRegistry.Find(handle, fileRecord)){
        auto retVal = CreateAsset(fileRecord.m_typeName, handle, fileRecord.m_name);
        retVal->SetPath(fileRecord.m_relativeFilePath);
        retVal->Load();
        assetManager.m_assets[handle] = retVal;
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
bool AssetManager::IsAsset(const std::string &typeName)
{
    return GetInstance().m_defaultExtensions.find(typeName) != GetInstance().m_defaultExtensions.end();
}
void AssetManager::OnInspect()
{
    auto &resourceManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Shared Assets", &resourceManager.m_enableAssetMenu);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    if (resourceManager.m_enableAssetMenu)
    {
        ImGui::Begin("Shared Assets");
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
                                IM_ASSERT(payload->DataSize == sizeof(Handle));
                                Handle payload_n = *static_cast<Handle *>(payload->Data);
                                AssetRef assetRef;
                                assetRef.m_assetHandle = payload_n;
                                assetRef.Update();
                                Share(assetRef.m_value);
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
    DefaultResources::Load();

    RegisterExternalAssetTypeExtensions<Prefab>({".obj", ".gltf", ".glb", ".blend", ".ply", ".fbx", ".dae"});
    RegisterExternalAssetTypeExtensions<Texture2D>({".png", ".jpg", ".jpeg", ".tga", ".hdr"});
}

void AssetRegistry::AddOrResetFile(Handle handle, const FileRecord &newFileRecord)
{
    std::string original = m_assetRecords[handle].m_relativeFilePath.string();
    m_assetRecords[handle] = newFileRecord;
    if (m_fileMap.find(original) != m_fileMap.end())
        m_fileMap.erase(original);
    m_fileMap[newFileRecord.m_relativeFilePath.string()] = handle;

    auto& assetManager = AssetManager::GetInstance();
    auto search = assetManager.m_assets.find(handle);
    if(search != assetManager.m_assets.end() && !search->second.expired()){
        search->second.lock()->m_projectRelativePath = newFileRecord.m_relativeFilePath;
    }
}
void AssetRegistry::RemoveFile(Handle handle)
{
    if (m_assetRecords.find(handle) != m_assetRecords.end())
    {
        m_fileMap.erase(m_assetRecords[handle].m_relativeFilePath.string());
        m_assetRecords.erase(handle);
    }
}
bool AssetRegistry::Find(Handle handle, FileRecord &target)
{
    auto search = m_assetRecords.find(handle);
    if (search != m_assetRecords.end())
    {
        target = search->second;
        return true;
    }
    return false;
}

bool AssetRegistry::Find(const std::filesystem::path &newRelativePath, Handle &handle)
{
    auto search = m_fileMap.find(newRelativePath.string());
    if (search != m_fileMap.end())
    {
        handle = search->second;
        return true;
    }
    return false;
}
void AssetRegistry::Clear()
{
    m_fileMap.clear();
    m_assetRecords.clear();
}
void AssetRegistry::ResetFilePath(Handle handle, const std::filesystem::path &newFilePath)
{
    std::string original = m_assetRecords[handle].m_relativeFilePath.string();
    m_assetRecords[handle].m_relativeFilePath = newFilePath;
    if (m_fileMap.find(original) != m_fileMap.end())
        m_fileMap.erase(original);
    m_fileMap[newFilePath.string()] = handle;
}
bool AssetRegistry::Find(const std::filesystem::path &newRelativePath)
{
    auto search = m_fileMap.find(newRelativePath.string());
    if (search != m_fileMap.end())
    {
        return true;
    }
    return false;
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
