#include "Editor.hpp"
#include "Engine/Core/Serialization.hpp"
#include "Engine/Rendering/Graphics.hpp"
#include <Application.hpp>
#include <DefaultResources.hpp>
#include <MeshRenderer.hpp>
#include <Prefab.hpp>
#include <ProjectManager.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <Utilities.hpp>
#include "ClassRegistry.hpp"
using namespace UniEngine;
AssetRegistration<Prefab> PrefabReg("Prefab", {".ueprefab", ".obj", ".gltf", ".glb", ".blend", ".ply", ".fbx", ".dae", ".x3d"});
void Prefab::OnCreate()
{
    m_name = "New Prefab";
}

Entity Prefab::ToEntity(const std::shared_ptr<Scene>& scene) const
{
    std::unordered_map<Handle, Handle> entityMap;
    std::vector<DataComponentType> types;
    for (auto &i : m_dataComponents)
    {
        types.emplace_back(i.m_type);
    }
    auto archetype = Entities::CreateEntityArchetype("", types);
    const Entity entity = scene->CreateEntity(archetype, m_name);
    entityMap[m_entityHandle] = scene->GetEntityHandle(entity);
    for (auto &i : m_dataComponents)
    {
        scene->SetDataComponent(entity.GetIndex(), i.m_type.m_typeId, i.m_type.m_size, i.m_data.get());
    }
    int index = 0;
    for (const auto &i : m_children)
    {
        AttachChildren(scene, i, entity, m_name + "_" + std::to_string(index), entityMap);
        index++;
    }

    for (auto &i : m_privateComponents)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            Serialization::ProduceSerializable(i.m_data->GetTypeName(), id));
        Serialization::ClonePrivateComponent(ptr, i.m_data);
        ptr->m_scene = scene;
        scene->SetPrivateComponent(entity, ptr);
    }
    for (const auto &i : m_children)
    {
        AttachChildrenPrivateComponent(scene, i, entity, entityMap);
        index++;
    }

    RelinkChildren(scene, entity, entityMap);

    scene->SetEnable(entity, m_enabled);
    return entity;
}
void Prefab::AttachChildren(const std::shared_ptr<Scene>& scene,
    const std::shared_ptr<Prefab> &modelNode,
    Entity parentEntity,
    const std::string &parentName,
    std::unordered_map<Handle, Handle> &map) const
{
    std::vector<DataComponentType> types;
    for (auto &i : modelNode->m_dataComponents)
    {
        types.emplace_back(i.m_type);
    }
    auto archetype = Entities::CreateEntityArchetype("", types);
    auto entity = scene->CreateEntity(archetype, m_name);
    map[modelNode->m_entityHandle] = scene->GetEntityHandle(entity);
    scene->SetParent(entity, parentEntity);
    for (auto &i : modelNode->m_dataComponents)
    {
        scene->SetDataComponent(entity.GetIndex(), i.m_type.m_typeId, i.m_type.m_size, i.m_data.get());
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildren(scene, i, entity, (parentName + "_" + std::to_string(index)), map);
        index++;
    }
}

void Prefab::AttachChildrenPrivateComponent(const std::shared_ptr<Scene>& scene,
                                            const std::shared_ptr<Prefab> &modelNode,
    const Entity &parentEntity,
    const std::unordered_map<Handle, Handle> &map) const
{
    Entity entity;
    auto children = scene->GetChildren(parentEntity);
    for (auto &i : children)
    {
        auto a = scene->GetEntityHandle(i).GetValue();
        auto b = map.at(modelNode->m_entityHandle).GetValue();
        if (a == b)
            entity = i;
    }
    if (entity.GetIndex() == 0)
        return;
    for (auto &i : modelNode->m_privateComponents)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            Serialization::ProduceSerializable(i.m_data->GetTypeName(), id));
        Serialization::ClonePrivateComponent(ptr, i.m_data);
        ptr->m_scene = scene;
        scene->SetPrivateComponent(entity, ptr);
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildrenPrivateComponent(scene, i, entity, map);
        index++;
    }
    scene->SetEnable(entity, m_enabled);
}
#pragma region Model Loading
bool Prefab::LoadInternal(const std::filesystem::path &path)
{
    if (path.extension() == ".ueprefab")
    {
        std::ifstream stream(path.string());
        std::stringstream stringStream;
        stringStream << stream.rdbuf();
        YAML::Node in = YAML::Load(stringStream.str());
#pragma region Assets
        std::vector<std::shared_ptr<IAsset>> localAssets;
        auto inLocalAssets = in["LocalAssets"];
        if (inLocalAssets)
        {
            for (const auto &i : inLocalAssets)
            {
                Handle handle = i["Handle"].as<uint64_t>();
                localAssets.push_back(ProjectManager::CreateTemporaryAsset(i["TypeName"].as<std::string>(), handle));
            }
            int index = 0;
            for (const auto &i : inLocalAssets)
            {
                localAssets[index++]->Deserialize(i);
            }
        }

#pragma endregion
        Deserialize(in);
    }
    else
    {
        LoadModelInternal(path);
    }
    return true;
}
void Prefab::AttachAnimator(Prefab *parent, const Handle &animatorEntityHandle)
{
    auto smr = parent->GetPrivateComponent<SkinnedMeshRenderer>();
    if (smr)
    {
        smr->m_animator.m_entityHandle = animatorEntityHandle;
        smr->m_animator.m_privateComponentTypeName = "Animator";
    }
    for (auto &i : parent->m_children)
    {
        AttachAnimator(i.get(), animatorEntityHandle);
    }
}
glm::mat4 mat4_cast(const aiMatrix4x4 &m)
{
    return glm::transpose(glm::make_mat4(&m.a1));
}
glm::mat4 mat4_cast(const aiMatrix3x3 &m)
{
    return glm::transpose(glm::make_mat3(&m.a1));
}
AssimpNode::AssimpNode(aiNode *node)
{
    m_correspondingNode = node;
    if (node->mParent)
        m_localTransform.m_value = mat4_cast(node->mTransformation);
    m_name = node->mName.C_Str();
}
void AssimpNode::AttachToAnimator(std::shared_ptr<Animation> &animation, size_t &index)
{
    animation->m_rootBone = m_bone;
    animation->m_rootBone->m_index = index;
    for (auto &i : m_children)
    {
        index += 1;
        i->AttachChild(m_bone, index);
    }
}
void AssimpNode::AttachChild(std::shared_ptr<Bone> &parent, size_t &index)
{
    m_bone->m_index = index;
    parent->m_children.push_back(m_bone);
    for (auto &i : m_children)
    {
        index += 1;
        i->AttachChild(m_bone, index);
    }
}
bool AssimpNode::NecessaryWalker(std::map<std::string, std::shared_ptr<Bone>> &boneMap)
{
    bool necessary = false;
    for (int i = 0; i < m_children.size(); i++)
    {
        if (!m_children[i]->NecessaryWalker(boneMap))
        {
            m_children.erase(m_children.begin() + i);
            i--;
        }
        else
        {
            necessary = true;
        }
    }
    const auto search = boneMap.find(m_name);
    if (search != boneMap.end())
    {
        m_bone = search->second;
        necessary = true;
    }
    else if (necessary)
    {
        m_bone = std::make_shared<Bone>();
        m_bone->m_name = m_name;
    }

    return necessary;
}
void Prefab::ReadKeyFrame(BoneKeyFrames &boneAnimation, const aiNodeAnim *channel)
{
    const auto numPositions = channel->mNumPositionKeys;
    boneAnimation.m_positions.resize(numPositions);
    for (int positionIndex = 0; positionIndex < numPositions; ++positionIndex)
    {
        const aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        const float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        BonePosition data;
        data.m_value = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
        data.m_timeStamp = timeStamp;
        boneAnimation.m_positions.push_back(data);
        boneAnimation.m_maxTimeStamp = glm::max(boneAnimation.m_maxTimeStamp, timeStamp);
    }

    const auto numRotations = channel->mNumRotationKeys;
    boneAnimation.m_rotations.resize(numRotations);
    for (int rotationIndex = 0; rotationIndex < numRotations; ++rotationIndex)
    {
        const aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        const float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        BoneRotation data;
        data.m_value = glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z);
        data.m_timeStamp = timeStamp;
        boneAnimation.m_rotations.push_back(data);
        boneAnimation.m_maxTimeStamp = glm::max(boneAnimation.m_maxTimeStamp, timeStamp);
    }

    const auto numScales = channel->mNumScalingKeys;
    boneAnimation.m_scales.resize(numScales);
    for (int keyIndex = 0; keyIndex < numScales; ++keyIndex)
    {
        const aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        const float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        BoneScale data;
        data.m_value = glm::vec3(scale.x, scale.y, scale.z);
        data.m_timeStamp = timeStamp;
        boneAnimation.m_scales.push_back(data);
        boneAnimation.m_maxTimeStamp = glm::max(boneAnimation.m_maxTimeStamp, timeStamp);
    }
}
void Prefab::ReadAnimations(
    const aiScene *importerScene,
    std::shared_ptr<Animation> &animator,
    std::map<std::string, std::shared_ptr<Bone>> &bonesMap)
{
    for (int i = 0; i < importerScene->mNumAnimations; i++)
    {
        aiAnimation *importerAnimation = importerScene->mAnimations[i];
        const std::string animationName = importerAnimation->mName.C_Str();
        float maxAnimationTimeStamp = 0.0f;
        for (int j = 0; j < importerAnimation->mNumChannels; j++)
        {
            aiNodeAnim *importerNodeAmination = importerAnimation->mChannels[j];
            const std::string nodeName = importerNodeAmination->mNodeName.C_Str();
            const auto search = bonesMap.find(nodeName);
            if (search != bonesMap.end())
            {
                auto &bone = search->second;
                bone->m_animations[animationName] = BoneKeyFrames();
                ReadKeyFrame(bone->m_animations[animationName], importerNodeAmination);
                maxAnimationTimeStamp =
                    glm::max(maxAnimationTimeStamp, bone->m_animations[animationName].m_maxTimeStamp);
            }
        }
        animator->m_animationNameAndLength[animationName] = maxAnimationTimeStamp;
    }
}
std::shared_ptr<Texture2D> Prefab::CollectTexture(
    const std::string &directory,
    const std::string &path,
    std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures)
{
    const std::string fileName = directory + "/" + path;
    if (const auto search = loadedTextures.find(fileName); search != loadedTextures.end())
    {
        return search->second;
    }
    auto texture2D = std::dynamic_pointer_cast<Texture2D>(ProjectManager::GetOrCreateAsset(
        std::filesystem::relative(directory + "/" + path, ProjectManager::GetProjectPath().parent_path())));
    loadedTextures[fileName] = texture2D;
    return texture2D;
}
std::shared_ptr<Material> Prefab::ReadMaterial(
    const std::string &directory,
    const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
    std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
    aiMaterial *importerMaterial)
{
    auto targetMaterial = ProjectManager::CreateTemporaryAsset<Material>();
    targetMaterial->SetProgram(glProgram);
    if (importerMaterial)
    {
        // PBR
        if (importerMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
            targetMaterial->SetTexture(TextureType::Albedo, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            targetMaterial->SetTexture(TextureType::Albedo, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &str);
            targetMaterial->SetTexture(TextureType::Normal, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_METALNESS, 0, &str);
            targetMaterial->SetTexture(TextureType::Metallic, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
            targetMaterial->SetTexture(
                TextureType::Roughness, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
            targetMaterial->SetTexture(TextureType::AO, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        if (importerMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            aiString str;
            importerMaterial->GetTexture(aiTextureType_HEIGHT, 0, &str);
            targetMaterial->SetTexture(TextureType::Normal, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
        }
        aiColor3D color;
        if(importerMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
        {
            targetMaterial->m_materialProperties.m_albedoColor = glm::vec3(color.r, color.g, color.b);
        }else if(importerMaterial->Get(AI_MATKEY_BASE_COLOR, color) == aiReturn_SUCCESS)
        {
            targetMaterial->m_materialProperties.m_albedoColor = glm::vec3(color.r, color.g, color.b);
        }
        ai_real factor;
        if (importerMaterial->Get(AI_MATKEY_METALLIC_FACTOR, factor) == aiReturn_SUCCESS)
        {
            targetMaterial->m_materialProperties.m_metallic = factor;
        }
        if (importerMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == aiReturn_SUCCESS)
        {
            targetMaterial->m_materialProperties.m_roughness = factor;
        }
        if (importerMaterial->Get(AI_MATKEY_SPECULAR_FACTOR, factor) == aiReturn_SUCCESS)
        {
            targetMaterial->m_materialProperties.m_specular = factor;
        }
    }
    return targetMaterial;
}
bool Prefab::ProcessNode(
    const std::string &directory,
    Prefab *modelNode,
    std::map<unsigned, std::shared_ptr<Material>> &loadedMaterials,
    std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
    std::map<std::string, std::shared_ptr<Bone>> &boneMaps,
    aiNode *importerNode,
    std::shared_ptr<AssimpNode> assimpNode,
    const aiScene *importerScene,
    const std::shared_ptr<Animation> &animation)
{
    bool addedMeshRenderer = false;
    for (unsigned i = 0; i < importerNode->mNumMeshes; i++)
    {
        // the modelNode object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, modelNode is just to keep stuff organized (like relations between nodes).
        aiMesh *importerMesh = importerScene->mMeshes[importerNode->mMeshes[i]];
        if (!importerMesh)
            continue;
        auto childNode = ProjectManager::CreateTemporaryAsset<Prefab>();
        childNode->m_name = std::string(importerMesh->mName.C_Str());
        const auto search = loadedMaterials.find(importerMesh->mMaterialIndex);
        bool isSkinnedMesh = importerMesh->mNumBones != 0xffffffff && importerMesh->mBones;
        std::shared_ptr<Material> material;
        if (search == loadedMaterials.end())
        {
            aiMaterial *importerMaterial = nullptr;
            if (importerMesh->mMaterialIndex != 0xffffffff &&
                importerMesh->mMaterialIndex < importerScene->mNumMaterials)
                importerMaterial = importerScene->mMaterials[importerMesh->mMaterialIndex];
            material = ReadMaterial(
                directory,
                isSkinnedMesh ? DefaultResources::GLPrograms::StandardSkinnedProgram
                              : DefaultResources::GLPrograms::StandardProgram,
                texture2DsLoaded,
                importerMaterial);
        }
        else
        {
            material = search->second;
        }

        if (isSkinnedMesh)
        {
            auto skinnedMeshRenderer = Serialization::ProduceSerializable<SkinnedMeshRenderer>();
            skinnedMeshRenderer->m_material.Set<Material>(material);
            skinnedMeshRenderer->m_skinnedMesh.Set<SkinnedMesh>(ReadSkinnedMesh(boneMaps, importerMesh));
            if (!skinnedMeshRenderer->m_skinnedMesh.Get())
                continue;
            addedMeshRenderer = true;
            PrivateComponentHolder holder;
            holder.m_enabled = true;
            holder.m_data = std::static_pointer_cast<IPrivateComponent>(skinnedMeshRenderer);
            childNode->m_privateComponents.push_back(holder);
        }
        else
        {
            auto meshRenderer = Serialization::ProduceSerializable<MeshRenderer>();
            meshRenderer->m_material.Set<Material>(material);
            meshRenderer->m_mesh.Set<Mesh>(ReadMesh(importerMesh));
            if (!meshRenderer->m_mesh.Get())
                continue;
            addedMeshRenderer = true;
            PrivateComponentHolder holder;
            holder.m_enabled = true;
            holder.m_data = std::static_pointer_cast<IPrivateComponent>(meshRenderer);
            childNode->m_privateComponents.push_back(holder);
        }
        auto transform = std::make_shared<Transform>();
        transform->m_value = mat4_cast(importerNode->mTransformation);
        if (!importerNode->mParent)
            transform->m_value = Transform().m_value;

        DataComponentHolder holder;
        holder.m_type = Typeof<Transform>();
        holder.m_data = transform;
        childNode->m_dataComponents.push_back(holder);

        modelNode->m_children.push_back(std::move(childNode));
    }

    for (unsigned i = 0; i < importerNode->mNumChildren; i++)
    {
        auto childNode = ProjectManager::CreateTemporaryAsset<Prefab>();
        childNode->m_name = std::string(importerNode->mChildren[i]->mName.C_Str());
        auto childAssimpNode = std::make_shared<AssimpNode>(importerNode->mChildren[i]);
        childAssimpNode->m_parent = assimpNode;
        const bool childAdd = ProcessNode(
            directory,
            childNode.get(),
            loadedMaterials,
            texture2DsLoaded,
            boneMaps,
            importerNode->mChildren[i],
            childAssimpNode,
            importerScene,
            animation);
        if (childAdd)
        {
            modelNode->m_children.push_back(std::move(childNode));
        }
        addedMeshRenderer = addedMeshRenderer | childAdd;
        assimpNode->m_children.push_back(std::move(childAssimpNode));
    }
    return addedMeshRenderer;
}
std::shared_ptr<Mesh> Prefab::ReadMesh(aiMesh *importerMesh)
{
    unsigned mask = 1;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    if (importerMesh->mNumVertices == 0 || !importerMesh->HasFaces())
        return nullptr;
    vertices.resize(importerMesh->mNumVertices);
    // Walk through each of the mesh's vertices
    for (int i = 0; i < importerMesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 v3; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly
        // convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        v3.x = importerMesh->mVertices[i].x;
        v3.y = importerMesh->mVertices[i].y;
        v3.z = importerMesh->mVertices[i].z;
        vertex.m_position = v3;
        if (importerMesh->HasNormals())
        {
            v3.x = importerMesh->mNormals[i].x;
            v3.y = importerMesh->mNormals[i].y;
            v3.z = importerMesh->mNormals[i].z;
            vertex.m_normal = v3;
            mask = mask | static_cast<unsigned>(VertexAttribute::Normal);
        }
        if (importerMesh->HasTangentsAndBitangents())
        {
            v3.x = importerMesh->mTangents[i].x;
            v3.y = importerMesh->mTangents[i].y;
            v3.z = importerMesh->mTangents[i].z;
            vertex.m_tangent = v3;
            mask = mask | static_cast<unsigned>(VertexAttribute::Tangent);
        }
        if (importerMesh->HasVertexColors(0))
        {
            v3.x = importerMesh->mColors[0][i].r;
            v3.y = importerMesh->mColors[0][i].g;
            v3.z = importerMesh->mColors[0][i].b;
            vertex.m_color = glm::vec4(v3, 1.0f);
            mask = mask | static_cast<unsigned>(VertexAttribute::Color);
        }
        glm::vec2 v2;
        if (importerMesh->HasTextureCoords(0))
        {
            v2.x = importerMesh->mTextureCoords[0][i].x;
            v2.y = importerMesh->mTextureCoords[0][i].y;
            vertex.m_texCoord = v2;
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        else
        {
            vertex.m_texCoord = glm::vec2(0.0f, 0.0f);
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        vertices[i] = vertex;
    }
    // now walk through each of the mesh's _Faces (a face is a mesh its triangle) and retrieve the corresponding vertex
    // indices.
    for (int i = 0; i < importerMesh->mNumFaces; i++)
    {
        assert(importerMesh->mFaces[i].mNumIndices == 3);
        // retrieve all indices of the face and store them in the indices vector
        for (int j = 0; j < 3; j++)
            indices.push_back(importerMesh->mFaces[i].mIndices[j]);
    }
    auto mesh = ProjectManager::CreateTemporaryAsset<Mesh>();
    mesh->SetVertices(mask, vertices, indices);
    return mesh;
}
std::shared_ptr<SkinnedMesh> Prefab::ReadSkinnedMesh(
    std::map<std::string, std::shared_ptr<Bone>> &bonesMap, aiMesh *importerMesh)
{
    unsigned mask = 1;
    std::vector<SkinnedVertex> vertices;
    std::vector<unsigned> indices;
    if (importerMesh->mNumVertices == 0 || !importerMesh->HasFaces())
        return nullptr;
    vertices.resize(importerMesh->mNumVertices);
    // Walk through each of the mesh's vertices
    for (int i = 0; i < importerMesh->mNumVertices; i++)
    {
        SkinnedVertex vertex;
        glm::vec3 v3; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly
        // convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        v3.x = importerMesh->mVertices[i].x;
        v3.y = importerMesh->mVertices[i].y;
        v3.z = importerMesh->mVertices[i].z;
        vertex.m_position = v3;
        if (importerMesh->HasNormals())
        {
            v3.x = importerMesh->mNormals[i].x;
            v3.y = importerMesh->mNormals[i].y;
            v3.z = importerMesh->mNormals[i].z;
            vertex.m_normal = v3;
            mask = mask | static_cast<unsigned>(VertexAttribute::Normal);
        }
        if (importerMesh->HasTangentsAndBitangents())
        {
            v3.x = importerMesh->mTangents[i].x;
            v3.y = importerMesh->mTangents[i].y;
            v3.z = importerMesh->mTangents[i].z;
            vertex.m_tangent = v3;
            mask = mask | static_cast<unsigned>(VertexAttribute::Tangent);
        }
        if (importerMesh->HasVertexColors(0))
        {
            v3.x = importerMesh->mColors[0][i].r;
            v3.y = importerMesh->mColors[0][i].g;
            v3.z = importerMesh->mColors[0][i].b;
            vertex.m_color = glm::vec4(v3, 1.0f);
            mask = mask | static_cast<unsigned>(VertexAttribute::Color);
        }
        glm::vec2 v2;
        if (importerMesh->HasTextureCoords(0))
        {
            v2.x = importerMesh->mTextureCoords[0][i].x;
            v2.y = importerMesh->mTextureCoords[0][i].y;
            vertex.m_texCoord = v2;
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        else
        {
            vertex.m_texCoord = glm::vec2(0.0f, 0.0f);
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        vertices[i] = vertex;
    }
    // now walk through each of the mesh's _Faces (a face is a mesh its triangle) and retrieve the corresponding vertex
    // indices.
    for (int i = 0; i < importerMesh->mNumFaces; i++)
    {
        assert(importerMesh->mFaces[i].mNumIndices == 3);
        // retrieve all indices of the face and store them in the indices vector
        for (int j = 0; j < 3; j++)
            indices.push_back(importerMesh->mFaces[i].mIndices[j]);
    }
    auto skinnedMesh = ProjectManager::CreateTemporaryAsset<SkinnedMesh>();
#pragma region Read bones
    std::vector<std::vector<std::pair<int, float>>> verticesBoneIdWeights;
    verticesBoneIdWeights.resize(vertices.size());
    for (unsigned i = 0; i < importerMesh->mNumBones; i++)
    {
        aiBone *importerBone = importerMesh->mBones[i];
        auto name = importerBone->mName.C_Str();
        if (const auto search = bonesMap.find(name); search == bonesMap.end()) // If we can't find this bone
        {
            std::shared_ptr<Bone> bone = std::make_shared<Bone>();
            bone->m_name = name;
            bone->m_offsetMatrix.m_value = mat4_cast(importerBone->mOffsetMatrix);
            bonesMap[name] = bone;
            skinnedMesh->m_bones.push_back(bone);
        }
        else
        {
            skinnedMesh->m_bones.push_back(search->second);
        }

        for (int j = 0; j < importerBone->mNumWeights; j++)
        {
            verticesBoneIdWeights[importerBone->mWeights[j].mVertexId].emplace_back(
                i, importerBone->mWeights[j].mWeight);
        }
    }
    for (unsigned i = 0; i < verticesBoneIdWeights.size(); i++)
    {
        auto ids = glm::ivec4(-1);
        auto weights = glm::vec4(0.0f);
        auto &list = verticesBoneIdWeights[i];
        for (unsigned j = 0; j < 4; j++)
        {
            if (!list.empty())
            {
                int extract = -1;
                float max = -1.0f;
                for (int k = 0; k < list.size(); k++)
                {
                    if (list[k].second > max)
                    {
                        max = list[k].second;
                        extract = k;
                    }
                }
                ids[j] = list[extract].first;
                weights[j] = list[extract].second;
                list.erase(list.begin() + extract);
            }
            else
                break;
        }
        vertices[i].m_bondId = ids;
        vertices[i].m_weight = weights;

        ids = glm::ivec4(-1);
        weights = glm::vec4(0.0f);
        for (unsigned j = 0; j < 4; j++)
        {
            if (!list.empty())
            {
                int extract = -1;
                float max = -1.0f;
                for (int k = 0; k < list.size(); k++)
                {
                    if (list[k].second > max)
                    {
                        max = list[k].second;
                        extract = k;
                    }
                }
                ids[j] = list[extract].first;
                weights[j] = list[extract].second;
                list.erase(list.begin() + extract);
            }
            else
                break;
        }
        vertices[i].m_bondId2 = ids;
        vertices[i].m_weight2 = weights;
    }
#pragma endregion
    skinnedMesh->SetVertices(mask, vertices, indices);
    return skinnedMesh;
}
void Prefab::ApplyBoneIndices(Prefab *node)
{
    auto smr = node->GetPrivateComponent<SkinnedMeshRenderer>();
    if (smr)
    {
        smr->m_skinnedMesh.Get<SkinnedMesh>()->FetchIndices();
        smr->m_skinnedMesh.Get<SkinnedMesh>()->m_bones.clear();
    }
    for (auto &i : node->m_children)
    {
        ApplyBoneIndices(i.get());
    }
}
void Prefab::FromEntity(const Entity &entity)
{
    auto scene = Application::GetActiveScene();
    if (!scene)
    {
        UNIENGINE_ERROR("Scene not attached!");
        return;
    }
    m_entityHandle = scene->GetEntityHandle(entity);
    m_name = scene->GetEntityName(entity);
    m_enabled = scene->IsEntityEnabled(entity);
    scene->UnsafeForEachDataComponent(entity, [&](const DataComponentType &type, void *data) {
            DataComponentHolder holder;
            holder.m_type = type;
            size_t id;
            size_t size;
            holder.m_data =
                std::static_pointer_cast<IDataComponent>(Serialization::ProduceDataComponent(type.m_name, id, size));
            memcpy(holder.m_data.get(), data, type.m_size);
            m_dataComponents.push_back(std::move(holder));
        });

    auto &elements = scene->m_sceneDataStorage.m_entityMetadataList.at(entity.GetIndex()).m_privateComponentElements;
    for (auto &element : elements)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            Serialization::ProduceSerializable(element.m_privateComponentData->GetTypeName(), id));
        ptr->OnCreate();
        Serialization::ClonePrivateComponent(ptr, element.m_privateComponentData);
        PrivateComponentHolder holder;
        holder.m_enabled = element.m_privateComponentData->m_enabled;
        holder.m_data = ptr;
        m_privateComponents.push_back(holder);
    }

    auto children = scene->GetChildren(entity);
    for (auto &i : children)
    {
        auto temp = ProjectManager::CreateTemporaryAsset<Prefab>();
        temp->m_name = scene->GetEntityName(i);
        m_children.push_back(temp);
        m_children.back()->FromEntity(i);
    }
}

#pragma endregion
void DataComponentHolder::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_type.m_name" << YAML::Value << m_type.m_name;
    out << YAML::Key << "m_data" << YAML::Value << YAML::Binary((const unsigned char *)m_data.get(), m_type.m_size);
}
bool DataComponentHolder::Deserialize(const YAML::Node &in)
{
    m_type.m_name = in["m_type.m_name"].as<std::string>();
    if (!Serialization::HasComponentDataType(m_type.m_name))
        return false;
    m_data = Serialization::ProduceDataComponent(m_type.m_name, m_type.m_typeId, m_type.m_size);
    if (in["m_data"])
    {
        YAML::Binary data = in["m_data"].as<YAML::Binary>();
        std::memcpy(m_data.get(), data.data(), data.size());
    }
    return true;
}
void Prefab::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_name" << YAML::Value << m_name;
    out << YAML::Key << "m_enabled" << YAML::Value << m_enabled;
    out << YAML::Key << "m_entityHandle" << YAML::Value << m_entityHandle.GetValue();
    if (!m_dataComponents.empty())
    {
        out << YAML::Key << "m_dataComponents" << YAML::BeginSeq;
        for (auto &i : m_dataComponents)
        {
            out << YAML::BeginMap;
            i.Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    if (!m_privateComponents.empty())
    {
        out << YAML::Key << "m_privateComponents" << YAML::BeginSeq;
        for (auto &i : m_privateComponents)
        {
            out << YAML::BeginMap;
            i.Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }

    if (!m_children.empty())
    {
        out << YAML::Key << "m_children" << YAML::BeginSeq;
        for (auto &i : m_children)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "m_handle" << i->GetHandle().GetValue();
            i->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}
void Prefab::Deserialize(const YAML::Node &in)
{
    m_name = in["m_name"].as<std::string>();
    m_enabled = in["m_enabled"].as<bool>();
    m_entityHandle = Handle(in["m_entityHandle"].as<uint64_t>());
    if (in["m_dataComponents"])
    {
        for (const auto &i : in["m_dataComponents"])
        {
            DataComponentHolder holder;
            if (holder.Deserialize(i))
            {
                m_dataComponents.push_back(holder);
            }
        }
    }
    if (in["m_privateComponents"])
    {
        for (const auto &i : in["m_privateComponents"])
        {
            PrivateComponentHolder holder;
            holder.Deserialize(i);
            m_privateComponents.push_back(holder);
        }
    }

    if (in["m_children"])
    {
        for (const auto &i : in["m_children"])
        {
            auto child = ProjectManager::CreateTemporaryAsset<Prefab>();
            child->m_handle = i["m_handle"].as<uint64_t>();
            child->Deserialize(i);
            m_children.push_back(child);
        }
    }
}
void Prefab::CollectAssets(std::unordered_map<Handle, std::shared_ptr<IAsset>> &map)
{
    std::vector<AssetRef> list;
    for (auto &i : m_privateComponents)
    {
        i.m_data->CollectAssetRef(list);
    }
    for (auto &i : list)
    {
        auto asset = i.Get<IAsset>();
        if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle() && asset->IsTemporary())
        {
            map[asset->GetHandle()] = asset;
        }
    }
    bool listCheck = true;
    while (listCheck)
    {
        size_t currentSize = map.size();
        list.clear();
        for (auto &i : map)
        {
            i.second->CollectAssetRef(list);
        }
        for (auto &i : list)
        {
            auto asset = i.Get<IAsset>();
            if (asset && asset->GetHandle().GetValue() >= DefaultResources::GetMaxHandle() && asset->IsTemporary())
            {
                map[asset->GetHandle()] = asset;
            }
        }
        if (map.size() == currentSize)
            listCheck = false;
    }
    for (auto &i : m_children)
        i->CollectAssets(map);
}
bool Prefab::SaveInternal(const std::filesystem::path &path)
{
    if (path.extension() == ".ueprefab")
    {
        auto directory = path;
        directory.remove_filename();
        std::filesystem::create_directories(directory);
        YAML::Emitter out;
        out << YAML::BeginMap;
        Serialize(out);
        std::unordered_map<Handle, std::shared_ptr<IAsset>> assetMap;
        CollectAssets(assetMap);
        if (!assetMap.empty())
        {
            out << YAML::Key << "LocalAssets" << YAML::Value << YAML::BeginSeq;
            for (auto &i : assetMap)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "TypeName" << YAML::Value << i.second->GetTypeName();
                out << YAML::Key << "Handle" << YAML::Value << i.first.GetValue();
                i.second->Serialize(out);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;

        std::ofstream fout(path.string());
        fout << out.c_str();
        fout.flush();
    }
    else if (path.extension() == ".obj")
    {
        SaveModelInternal(path);
    }
    return true;
}
void Prefab::RelinkChildren(const std::shared_ptr<Scene>& scene, const Entity &parentEntity, const std::unordered_map<Handle, Handle> &map) const
{
    scene->ForEachPrivateComponent(parentEntity, [&](PrivateComponentElement &data) {
        data.m_privateComponentData->Relink(map, scene);
    });
    scene->ForEachChild(parentEntity, [&](Entity child) {
            RelinkChildren(scene, child, map);
        });
}
void Prefab::LoadModel(const std::filesystem::path &path, bool optimize, unsigned flags)
{
    LoadModelInternal(ProjectManager::GetProjectPath().parent_path() / path, optimize, flags);
}

void Prefab::SaveModelInternal(const std::filesystem::path &path)
{
    Assimp::Exporter exporter;
    aiScene *scene = new aiScene();

    delete scene;
}
void Prefab::LoadModelInternal(const std::filesystem::path &path, bool optimize, unsigned int flags)
{
    if (optimize)
    {
        flags = flags | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes;
    }
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path.string(), flags);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        UNIENGINE_LOG("Assimp: " + std::string(importer.GetErrorString()));
        return;
    }
    // retrieve the directory path of the filepath
    auto temp = path;
    const std::string directory = temp.remove_filename().string();
    std::map<std::string, std::shared_ptr<Texture2D>> texture2DsLoaded;
    m_name = path.filename().string();
    std::map<unsigned, std::shared_ptr<Material>> loadedMaterials;
    std::map<std::string, std::shared_ptr<Bone>> bonesMap;
    std::shared_ptr<Animation> animation;
    if (!bonesMap.empty() || scene->HasAnimations())
    {
        animation = ProjectManager::CreateTemporaryAsset<Animation>();
    }
    std::shared_ptr<AssimpNode> rootAssimpNode = std::make_shared<AssimpNode>(scene->mRootNode);
    if (!ProcessNode(
            directory,
            this,
            loadedMaterials,
            texture2DsLoaded,
            bonesMap,
            scene->mRootNode,
            rootAssimpNode,
            scene,
            animation))
    {
        UNIENGINE_ERROR("Model is empty!");
        return;
    }
    if (!bonesMap.empty() || scene->HasAnimations())
    {
        rootAssimpNode->NecessaryWalker(bonesMap);
        size_t index = 0;
        rootAssimpNode->AttachToAnimator(animation, index);
        animation->m_boneSize = index + 1;
        ReadAnimations(scene, animation, bonesMap);
        ApplyBoneIndices(this);

        auto animator = Serialization::ProduceSerializable<Animator>();
        animator->Setup(animation);
        AttachAnimator(this, m_entityHandle);
        PrivateComponentHolder holder;
        holder.m_enabled = true;
        holder.m_data = std::static_pointer_cast<IPrivateComponent>(animator);
        m_privateComponents.push_back(holder);
    }
    return;
}
void PrivateComponentHolder::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_enabled" << YAML::Value << m_enabled;
    out << YAML::Key << "m_typeName" << YAML::Value << m_data->GetTypeName();

    out << YAML::Key << "m_data" << YAML::BeginMap;
    out << YAML::Key << "m_handle" << m_data->GetHandle().GetValue();
    m_data->Serialize(out);
    out << YAML::EndMap;
}
void PrivateComponentHolder::Deserialize(const YAML::Node &in)
{
    m_enabled = in["m_enabled"].as<bool>();
    auto typeName = in["m_typeName"].as<std::string>();
    auto inData = in["m_data"];
    if (Serialization::HasSerializableType(typeName))
    {
        size_t hashCode;
        m_data = std::dynamic_pointer_cast<IPrivateComponent>(
            Serialization::ProduceSerializable(typeName, hashCode, Handle(inData["m_handle"].as<uint64_t>())));
    }
    else
    {
        size_t hashCode;
        m_data = std::dynamic_pointer_cast<IPrivateComponent>(Serialization::ProduceSerializable(
            "UnknownPrivateComponent", hashCode, Handle(inData["m_handle"].as<uint64_t>())));
    }
    m_data->OnCreate();
    m_data->Deserialize(inData);
}
