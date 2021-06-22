#include <Application.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <FileIO.hpp>
#include <MeshRenderer.hpp>
#include <ResourceManager.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;

void ResourceManager::Remove(size_t id, size_t hashCode)
{
    GetInstance().m_resources[id].second.erase(hashCode);
}

std::shared_ptr<Model> UniEngine::ResourceManager::LoadModel(
    const bool &addResource, std::string const &path, std::shared_ptr<OpenGLUtils::GLProgram> glProgram)
{
    UNIENGINE_LOG("Loading model from: " + path);
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = ""; // Path to material files
    reader_config.triangulate = true;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    std::vector<std::shared_ptr<Texture2D>> texture2DsLoaded;
    auto retVal = std::make_shared<Model>();

    auto &attribute = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();
    const std::string directory = path.substr(0, path.find_last_of('/'));
    for (const auto &i : shapes)
    {
        ProcessNode(directory, glProgram, retVal->RootNode(), texture2DsLoaded, i, materials, attribute);
    }
    if (addResource)
        Push(retVal);
    return retVal;
}

Entity UniEngine::ResourceManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model)
{
    Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(model->m_name);
    Transform ltp;
    std::unique_ptr<ModelNode> &modelNode = model->RootNode();
    EntityManager::SetComponentData<Transform>(entity, ltp);
    for (auto &i : modelNode->m_meshMaterials)
    {
        auto mmc = std::make_unique<MeshRenderer>();
        mmc->m_mesh = i.second;
        mmc->m_material = i.first;
        EntityManager::SetPrivateComponent<MeshRenderer>(entity, std::move(mmc));
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildren(archetype, i, entity, model->m_name + "_" + std::to_string(index));
        index++;
    }
    return entity;
}

std::shared_ptr<Texture2D> ResourceManager::CollectTexture(
    const std::string &directory,
    const std::string &path,
    std::vector<std::shared_ptr<Texture2D>> &texture2DsLoaded, 
    const TextureType& textureType)
{
    for (unsigned j = 0; j < texture2DsLoaded.size(); j++)
    {
        if (texture2DsLoaded.at(j)->Path() == directory + "/" + path) return texture2DsLoaded.at(j);
    }
    auto texture2D = LoadTexture(false, directory + "/" + path, textureType);
    texture2DsLoaded.push_back(texture2D);
    return texture2D;
}

void ResourceManager::ProcessNode(
    const std::string &directory,
    std::shared_ptr<OpenGLUtils::GLProgram> glProgram,
    std::unique_ptr<ModelNode> &modelNode,
    std::vector<std::shared_ptr<Texture2D>> &texture2DsLoaded,
    const tinyobj::shape_t &shape,
    const std::vector<tinyobj::material_t> &materials,
    const tinyobj::attrib_t &attribute)
{
    std::unique_ptr<ModelNode> childNode = std::make_unique<ModelNode>();
    unsigned mask = 1;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    auto mesh = std::make_shared<Mesh>();
    auto material = std::make_shared<Material>();
    material->SetProgram(glProgram);
    // Loop over faces(polygon)
    size_t index_offset = 0;
    vertices.resize(shape.mesh.num_face_vertices.size() * 3);
    indices.resize(shape.mesh.num_face_vertices.size() * 3);
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
    {
        const size_t fv = size_t(shape.mesh.num_face_vertices[f]);
        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++)
        {
            // access to vertex
            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            indices[f * 3 + v] = f * 3 + v;
            vertices[f * 3 + v].m_position.x = attribute.vertices[3 * size_t(idx.vertex_index) + 0];
            vertices[f * 3 + v].m_position.y = attribute.vertices[3 * size_t(idx.vertex_index) + 1];
            vertices[f * 3 + v].m_position.z = attribute.vertices[3 * size_t(idx.vertex_index) + 2];

            // Check if `normal_index` is zero or positive. negative = no normal data
            if (idx.normal_index >= 0)
            {
                mask = mask | static_cast<unsigned>(VertexAttribute::Normal);
                vertices[f * 3 + v].m_normal.x = attribute.normals[3 * size_t(idx.normal_index) + 0];
                vertices[f * 3 + v].m_normal.y = attribute.normals[3 * size_t(idx.normal_index) + 1];
                vertices[f * 3 + v].m_normal.z = attribute.normals[3 * size_t(idx.normal_index) + 2];
            }

            // Check if `texcoord_index` is zero or positive. negative = no texcoord data
            if (idx.texcoord_index >= 0)
            {
                mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
                vertices[f * 3 + v].m_texCoords.x = attribute.texcoords[2 * size_t(idx.texcoord_index) + 0];
                vertices[f * 3 + v].m_texCoords.y = attribute.texcoords[2 * size_t(idx.texcoord_index) + 1];
            }
            if (!attribute.colors.empty())
            {
                mask = mask | static_cast<unsigned>(VertexAttribute::Color);
                vertices[f * 3 + v].m_color.x = attribute.colors[3 * size_t(idx.vertex_index) + 0];
                vertices[f * 3 + v].m_color.y = attribute.colors[3 * size_t(idx.vertex_index) + 1];
                vertices[f * 3 + v].m_color.z = attribute.colors[3 * size_t(idx.vertex_index) + 2];
            }
        }
        index_offset += fv;

        // per-face material
        if(shape.mesh.material_ids[f] != -1)
        {
            auto &importedMaterial = materials[shape.mesh.material_ids[f]];
            material->m_metallic = importedMaterial.metallic;
            material->m_roughness = importedMaterial.roughness;
            material->m_albedoColor = glm::vec3(importedMaterial.diffuse[0],importedMaterial.diffuse[1], importedMaterial.diffuse[2]);
            if(!importedMaterial.diffuse_texname.empty())
            {
                const auto albedo =
                    CollectTexture(directory, importedMaterial.diffuse_texname, texture2DsLoaded, TextureType::Albedo);
                if (albedo)
                {
                    material->SetTexture(albedo);
                }
            }
            if (!importedMaterial.bump_texname.empty())
            {
                const auto normal =
                    CollectTexture(directory, importedMaterial.bump_texname, texture2DsLoaded, TextureType::Normal);
                if (normal)
                {
                    material->SetTexture(normal);
                }
            }
            if (!importedMaterial.normal_texname.empty())
            {
                const auto normal =
                    CollectTexture(directory, importedMaterial.normal_texname, texture2DsLoaded, TextureType::Normal);
                if (normal)
                {
                    material->SetTexture(normal);
                }
            }
            if (!importedMaterial.roughness_texname.empty())
            {
                const auto roughness = CollectTexture(
                    directory, importedMaterial.roughness_texname, texture2DsLoaded, TextureType::Roughness);
                if (roughness)
                {
                    material->SetTexture(roughness);
                }
            }
            if (!importedMaterial.specular_highlight_texname.empty())
            {
                const auto roughness = CollectTexture(
                    directory, importedMaterial.specular_highlight_texname, texture2DsLoaded, TextureType::Roughness);
                if (roughness)
                {
                    material->SetTexture(roughness);
                }
            }
            if (!importedMaterial.metallic_texname.empty())
            {
                const auto metallic = CollectTexture(
                    directory, importedMaterial.metallic_texname, texture2DsLoaded, TextureType::Metallic);
                if (metallic)
                {
                    material->SetTexture(metallic);
                }
            }
            if (!importedMaterial.reflection_texname.empty())
            {
                const auto metallic = CollectTexture(
                    directory, importedMaterial.reflection_texname, texture2DsLoaded, TextureType::Metallic);
                if (metallic)
                {
                    material->SetTexture(metallic);
                }
            }

            if (!importedMaterial.ambient_texname.empty())
            {
                const auto ao = CollectTexture(
                    directory, importedMaterial.ambient_texname, texture2DsLoaded, TextureType::AO);
                if (ao)
                {
                    material->SetTexture(ao);
                }
            }
        }
    }
    mesh->SetVertices(mask, vertices, indices);
    childNode->m_localToParent = glm::translate(glm::vec3(0.0f)) * glm::mat4_cast(glm::quat(glm::vec3(0.0f))) * glm::scale(glm::vec3(1.0f));
    childNode->m_meshMaterials.emplace_back(material, mesh);
    modelNode->m_children.push_back(std::move(childNode));
}

void UniEngine::ResourceManager::AttachChildren(
    EntityArchetype archetype, std::unique_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName)
{
    Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(parentName);
    EntityManager::SetParent(entity, parentEntity);
    Transform ltp;
    ltp.m_value = modelNode->m_localToParent;
    EntityManager::SetComponentData<Transform>(entity, ltp);
    for (auto i : modelNode->m_meshMaterials)
    {
        auto mmc = std::make_unique<MeshRenderer>();
        mmc->m_mesh = i.second;
        mmc->m_material = i.first;
        EntityManager::SetPrivateComponent<MeshRenderer>(entity, std::move(mmc));
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildren(archetype, i, entity, (parentName + "_" + std::to_string(index)));
        index++;
    }
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(
    const bool &addResource, const std::string &path, TextureType type, const bool &generateMipmap, const float &gamma)
{
    stbi_set_flip_vertically_on_load(true);
    auto retVal = std::make_shared<Texture2D>(type);
    const std::string filename = path;
    retVal->m_path = filename;
    int width, height, nrComponents;
    stbi_ldr_to_hdr_gamma(gamma);
    float *data = stbi_loadf(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 2)
        {
            format = GL_RG;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            format = GL_RGBA;
        }
        GLsizei mipmap = generateMipmap ? static_cast<GLsizei>(log2((glm::max)(width, height))) + 1 : 1;
        retVal->m_texture = std::make_shared<OpenGLUtils::GLTexture2D>(mipmap, GL_RGBA32F, width, height, true);
        retVal->m_texture->SetData(0, format, GL_FLOAT, data);
        retVal->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_REPEAT);
        retVal->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_REPEAT);
        retVal->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        retVal->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (generateMipmap)
            retVal->m_texture->GenerateMipMap();
        stbi_image_free(data);
    }
    else
    {
        UNIENGINE_LOG("Texture failed to load at path: " + filename);
        stbi_image_free(data);
        return nullptr;
    }
    retVal->m_icon = retVal;
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    if (addResource)
        Push(retVal);
    return retVal;
}

std::shared_ptr<Cubemap> ResourceManager::LoadCubemap(
    const bool &addResource, const std::vector<std::string> &paths, const bool &generateMipmap, const float &gamma)
{
    int width, height, nrComponents;
    auto size = paths.size();
    if (size != 6)
    {
        UNIENGINE_ERROR("Texture::LoadCubeMap: Size error.");
        return nullptr;
    }
    stbi_ldr_to_hdr_gamma(gamma);
    float *temp = stbi_loadf(paths[0].c_str(), &width, &height, &nrComponents, 0);
    stbi_image_free(temp);
    GLsizei mipmap = generateMipmap ? static_cast<GLsizei>(log2((glm::max)(width, height))) + 1 : 1;
    auto texture = std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGBA32F, width, height, true);
    for (int i = 0; i < size; i++)
    {
        float *data = stbi_loadf(paths[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format = GL_RED;
            if (nrComponents == 2)
            {
                format = GL_RG;
            }
            else if (nrComponents == 3)
            {
                format = GL_RGB;
            }
            else if (nrComponents == 4)
            {
                format = GL_RGBA;
            }
            texture->SetData(static_cast<OpenGLUtils::CubeMapIndex>(i), 0, format, GL_FLOAT, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << paths[i] << std::endl;
            stbi_image_free(data);
            return nullptr;
        }
    }

    texture->SetInt(GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    if (generateMipmap)
        texture->GenerateMipMap();
    auto retVal = std::make_shared<Cubemap>();
    retVal->m_texture = std::move(texture);
    retVal->m_paths = paths;
    retVal->m_name = paths[0].substr(paths[0].find_last_of("/\\") + 1);
    if (addResource)
        Push(retVal);
    return retVal;
}

std::shared_ptr<Material> ResourceManager::LoadMaterial(
    const bool &addResource, const std::shared_ptr<OpenGLUtils::GLProgram> &program)
{
    auto retVal = std::make_shared<Material>();
    retVal->SetProgram(program);
    if (addResource)
        Push(retVal);
    return retVal;
}

std::shared_ptr<OpenGLUtils::GLProgram> ResourceManager::LoadProgram(
    const bool &addResource,
    const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
    const std::shared_ptr<OpenGLUtils::GLShader> &fragment)
{
    auto retVal = std::make_shared<OpenGLUtils::GLProgram>();
    retVal->Attach(vertex);
    retVal->Attach(fragment);
    retVal->Link();
    if (addResource)
        Push(retVal);
    return retVal;
}

std::shared_ptr<OpenGLUtils::GLProgram> ResourceManager::LoadProgram(
    const bool &addResource,
    const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
    const std::shared_ptr<OpenGLUtils::GLShader> &geometry,
    const std::shared_ptr<OpenGLUtils::GLShader> &fragment)
{
    auto retVal = std::make_shared<OpenGLUtils::GLProgram>();
    retVal->Attach(vertex);
    retVal->Attach(geometry);
    retVal->Attach(fragment);
    retVal->Link();
    if (addResource)
        Push(retVal);
    return retVal;
}

void ResourceManager::OnGui()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Open..."))
            {
                FileIO::OpenFile("Load World", ".uniengineworld", [](const std::string &filePath) {
                    SerializationManager::Deserialize(Application::GetCurrentWorld(), filePath);
                });
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Save..."))
            {
                FileIO::SaveFile("Save World", ".uniengineworld", [](const std::string &filePath) {
                    SerializationManager::Serialize(Application::GetCurrentWorld(), filePath);
                });
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Load"))
            {
                FileIO::OpenFile("Load Model", ".obj,.gltf,.glb,.blend,.ply,.fbx", [](const std::string &filePath) {
                    LoadModel(true, filePath, DefaultResources::GLPrograms::StandardProgram);
                    UNIENGINE_LOG("Loaded model from \"" + filePath);
                });

                FileIO::OpenFile("Load Texture", ".png,.jpg,.jpeg,.tga", [](const std::string &filePath) {
                    LoadTexture(true, filePath);
                    UNIENGINE_LOG("Loaded texture from \"" + filePath);
                });
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("Asset Manager", &GetInstance().m_enableAssetMenu);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    if (GetInstance().m_enableAssetMenu)
    {
        ImGui::Begin("Resource Manager");
        if (ImGui::BeginTabBar(
                "##Resource Tab", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            if (ImGui::BeginTabItem("Assets"))
            {
                uint32_t count = 0;
                for (const auto &entry : std::filesystem::recursive_directory_iterator(FileIO::GetAssetFolderPath()))
                    count++;
                static int selection_mask = 0;
                auto clickState =
                    FileIO::DirectoryTreeViewRecursive(FileIO::GetAssetFolderPath(), &count, &selection_mask);
                if (clickState.first)
                {
                    // Update selection state
                    // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
                    if (ImGui::GetIO().KeyCtrl)
                        selection_mask ^= 1 << (clickState.second); // CTRL+click to toggle
                    else // if (!(selection_mask & (1 << clickState.second))) // Depending on selection behavior you
                         // want, may want to preserve selection when clicking on item that is part of the selection
                        selection_mask = 1 << (clickState.second); // Click to single-select
                }
                ImGui::EndTabItem();
            }
            for (auto &collection : GetInstance().m_resources)
            {
                if (ImGui::BeginTabItem(collection.second.first.substr(6).c_str()))
                {
                    if (ImGui::BeginDragDropTarget())
                    {
                        const std::string hash = std::to_string(std::hash<std::string>{}(collection.second.first));
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(hash.c_str()))
                        {
                            IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<ResourceBehaviour>));
                            std::shared_ptr<ResourceBehaviour> payload_n =
                                *static_cast<std::shared_ptr<ResourceBehaviour> *>(payload->Data);
                            Push(payload_n);
                        }
                        ImGui::EndDragDropTarget();
                    }
                    for (auto &i : collection.second.second)
                    {
                        if (EditorManager::Draggable(collection.second.first, i.second))
                        {
                            Remove(i.first, i.second->GetHashCode());
                            break;
                        }
                    }

                    ImGui::EndTabItem();
                }
            }
        }
        ImGui::EndTabBar();
        ImGui::End();
    }
}