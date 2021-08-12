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

void AssetManager::RemoveFromShared(const std::string &typeName, const Handle &handle)
{
    GetInstance().m_assets[typeName].erase(handle);
}

Entity AssetManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture)
{
    const Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(texture->m_name);
    auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
    mmc->m_material = LoadMaterial(DefaultResources::GLPrograms::StandardProgram);
    mmc->m_material->SetTexture(TextureType::Albedo, texture);
    mmc->m_mesh = DefaultResources::Primitives::Quad;
    return entity;
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

void AssetManager::OnGui()
{
    auto &resourceManager = GetInstance();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Import..."))
            {
                FileUtils::OpenFile("Scene##Import", ".uescene", [&](const std::string &filePath) {
                    try
                    {
                        auto asset = Import<Scene>(filePath);
                        resourceManager.m_sharedAssets["Scene"][asset->m_handle] =
                            std::static_pointer_cast<IAsset>(asset);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                });

#ifdef USE_ASSIMP
                std::string modelFormat = ".obj,.gltf,.glb,.blend,.ply,.fbx,.dae";
#else
                std::string modelFormat = ".obj";
#endif
                FileUtils::OpenFile("Model##Import", modelFormat, [&](const std::string &filePath) {
                    try
                    {
                        auto asset = Import<Prefab>(filePath);
                        resourceManager.m_sharedAssets["Prefab"][asset->m_handle] =
                            std::static_pointer_cast<IAsset>(asset);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                });

                FileUtils::OpenFile("Texture2D##Import", ".png,.jpg,.jpeg,.tga,.hdr", [&](const std::string &filePath) {
                    try
                    {
                        auto asset = Import<Texture2D>(filePath);
                        resourceManager.m_sharedAssets["Texture2D"][asset->m_handle] =
                            std::static_pointer_cast<IAsset>(asset);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                });

                FileUtils::OpenFile("Cubemap##Import", ".png,.jpg,.jpeg,.tga,.hdr", [&](const std::string &filePath) {
                    try
                    {
                        auto asset = Import<Cubemap>(filePath);
                        resourceManager.m_sharedAssets["Cubemap"][asset->m_handle] =
                            std::static_pointer_cast<IAsset>(asset);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                });
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Export..."))
            {
                FileUtils::SaveFile("Scene##Export", ".uescene", [](const std::string &filePath) {
                    std::filesystem::path path = filePath;
                    path.replace_extension(".uescene");
                    try
                    {
                        Export(path, EntityManager::GetCurrentScene());
                        UNIENGINE_LOG("Saved to " + path.string());
                    }
                    catch (std::exception &e)
                    {
                        UNIENGINE_ERROR("Failed to save to " + path.string());
                    }
                });
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
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
                        if (ImGui::BeginDragDropTarget())
                        {
                            const std::string hash = collection.first;
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(hash.c_str()))
                            {
                                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<IAsset>));
                                std::shared_ptr<IAsset> payload_n =
                                    *static_cast<std::shared_ptr<IAsset> *>(payload->Data);
                                // Share(payload_n);
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
                            EditorManager::Draggable(i.second, false);
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
}
void AssetManager::ScanAssetFolder()
{
}

void AssetRegistry::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "Version" << YAML::Value << m_version;
    out << YAML::Key << "AssetRecords" << YAML::Value << YAML::BeginMap;
    for (const auto &i : m_assetRecords)
    {
        out << YAML::Key << "Handle" << i.first;
        out << YAML::Key << "FilePath" << i.second.m_filePath;
        out << YAML::Key << "TypeName" << i.second.m_typeName;
    }
    out << YAML::EndMap;
}

void AssetRegistry::Deserialize(const YAML::Node &in)
{
    m_version = in["Version"].as<size_t>();
    auto inAssetRecords = in["AssetRecords"];
    for (const auto &inAssetRecord : inAssetRecords)
    {
        Handle assetHandle(inAssetRecord["Handle"].as<uint64_t>());
        AssetRecord assetRecord;
        assetRecord.m_filePath = inAssetRecord["FilePath"].as<std::string>();
        assetRecord.m_typeName = inAssetRecord["TypeName"].as<std::string>();
        m_assetRecords.insert({assetHandle, assetRecord});
    }
}

std::filesystem::path AssetManager::GetAssetFolderPath()
{
    std::filesystem::path assetRootFolder = ProjectManager::GetInstance().m_projectPath / "Assets/";
    if (!std::filesystem::exists(assetRootFolder))
    {
        std::filesystem::create_directory(assetRootFolder);
    }
    return assetRootFolder;
}

void AssetManager::SetResourcePath(const std::filesystem::path &path)
{
    GetInstance().m_resourceRootPath = path;
}

std::filesystem::path AssetManager::GetResourceFolderPath()
{
    return GetInstance().m_resourceRootPath;
}
