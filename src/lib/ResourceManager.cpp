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
    const bool &addResource,
    std::string const &path,
    std::shared_ptr<OpenGLUtils::GLProgram> shader,
    const bool &gamma,
    const unsigned &flags)
{
    stbi_set_flip_vertically_on_load(true);
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, flags);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        UNIENGINE_LOG("ERROR::ASSIMP::" + std::string(importer.GetErrorString()));
        return nullptr;
    }
    // retrieve the directory path of the filepath
    std::string directory = path.substr(0, path.find_last_of('/'));
    std::vector<std::shared_ptr<Texture2D>> Texture2DsLoaded;
    auto retVal = std::make_shared<Model>();
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    ProcessNode(directory, shader, retVal->RootNode(), Texture2DsLoaded, scene->mRootNode, scene);
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

void ResourceManager::ProcessNode(
    std::string directory,
    std::shared_ptr<OpenGLUtils::GLProgram> shader,
    std::unique_ptr<ModelNode> &modelNode,
    std::vector<std::shared_ptr<Texture2D>> &Texture2DsLoaded,
    aiNode *node,
    const aiScene *scene)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        // the modelNode object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, modelNode is just to keep stuff organized (like relations between nodes).
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ReadMesh(i, modelNode, directory, shader, Texture2DsLoaded, mesh, scene);
        modelNode->m_localToParent = glm::mat4(
            node->mTransformation.a1,
            node->mTransformation.a2,
            node->mTransformation.a3,
            node->mTransformation.a4,
            node->mTransformation.b1,
            node->mTransformation.b2,
            node->mTransformation.b3,
            node->mTransformation.b4,
            node->mTransformation.c1,
            node->mTransformation.c2,
            node->mTransformation.c3,
            node->mTransformation.c4,
            node->mTransformation.d1,
            node->mTransformation.d2,
            node->mTransformation.d3,
            node->mTransformation.d4);
    }
    for (unsigned i = 0; i < node->mNumChildren; i++)
    {
        std::unique_ptr<ModelNode> childNode = std::make_unique<ModelNode>();
        ProcessNode(directory, shader, childNode, Texture2DsLoaded, node->mChildren[i], scene);
        modelNode->m_children.push_back(std::move(childNode));
    }
}

void ResourceManager::ReadMesh(
    unsigned meshIndex,
    std::unique_ptr<ModelNode> &modelNode,
    std::string directory,
    std::shared_ptr<OpenGLUtils::GLProgram> shader,
    std::vector<std::shared_ptr<Texture2D>> &Texture2DsLoaded,
    aiMesh *aimesh,
    const aiScene *scene)
{
    unsigned mask = 1;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    // Walk through each of the mesh's vertices
    for (unsigned i = 0; i < aimesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 v3; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly
                      // convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        v3.x = aimesh->mVertices[i].x;
        v3.y = aimesh->mVertices[i].y;
        v3.z = aimesh->mVertices[i].z;
        vertex.m_position = v3;
        if (aimesh->mNormals)
        {
            v3.x = aimesh->mNormals[i].x;
            v3.y = aimesh->mNormals[i].y;
            v3.z = aimesh->mNormals[i].z;
            vertex.m_normal = v3;
            mask = mask | (1 << 1);
        }
        if (aimesh->mTangents)
        {
            v3.x = aimesh->mTangents[i].x;
            v3.y = aimesh->mTangents[i].y;
            v3.z = aimesh->mTangents[i].z;
            vertex.m_tangent = v3;
            mask = mask | (1 << 2);
        }
        glm::vec4 v4;
        if (aimesh->mColors[0])
        {
            v4.x = aimesh->mColors[0][i].r;
            v4.y = aimesh->mColors[0][i].g;
            v4.z = aimesh->mColors[0][i].b;
            v4.w = aimesh->mColors[0][i].a;
            vertex.m_color = v4;
            mask = mask | (1 << 3);
        }
        glm::vec2 v2;
        if (aimesh->mTextureCoords[0])
        {
            v2.x = aimesh->mTextureCoords[0][i].x;
            v2.y = aimesh->mTextureCoords[0][i].y;
            vertex.m_texCoords = v2;
            mask = mask | (1 << 4);
        }
        else
        {
            vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
            mask = mask | (1 << 5);
        }
        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's _Faces (a face is a mesh its triangle) and retrieve the corresponding vertex
    // indices.
    for (unsigned i = 0; i < aimesh->mNumFaces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *pointMaterial = scene->mMaterials[aimesh->mMaterialIndex];

    auto mesh = std::make_shared<Mesh>();
    mesh->SetVertices(mask, vertices, indices);

    auto material = std::make_shared<Material>();
    float shininess;
    pointMaterial->Get(AI_MATKEY_SHININESS, shininess);
    if (shininess == 0.0f)
        shininess = 32.0f;
    material->SetProgram(shader);
    // PBR
    if (pointMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Albedo);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Albedo);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Normal);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_METALNESS, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Metallic);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Roughness);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::AO);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    if (pointMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        aiString str;
        pointMaterial->GetTexture(aiTextureType_HEIGHT, 0, &str);
        bool skip = false;
        for (unsigned j = 0; j < Texture2DsLoaded.size(); j++)
        {
            if (Texture2DsLoaded.at(j)->Path().compare(directory + "/" + str.C_Str()) == 0)
            {
                material->SetTexture(Texture2DsLoaded.at(j));
                skip = true; // a Texture2D with the same filepath has already been loaded, continue to next one.
                             // (optimization)
                break;
            }
        }
        if (!skip)
        { // if Texture2D hasn't been loaded already, load it
            auto texture2D = LoadTexture(false, directory + "/" + str.C_Str(), TextureType::Normal);
            material->SetTexture(texture2D);
            Texture2DsLoaded.push_back(texture2D); // store it as Texture2D loaded for entire model, to ensure we won't
                                                   // unnecesery load duplicate Texture2Ds.
        }
    }
    modelNode->m_meshMaterials.emplace_back(material, mesh);
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
                    LoadModel(true, filePath, Default::GLPrograms::StandardProgram);
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
