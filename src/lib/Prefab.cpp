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
void Prefab::OnCreate()
{
    m_name = "New Prefab";
}

Entity Prefab::ToEntity() const
{
    std::unordered_map<Handle, Handle> entityMap;
    std::vector<DataComponentType> types;
    for(auto& i : m_dataComponents)
    {
        types.emplace_back(i.m_type);
    }
    auto archetype = EntityManager::CreateEntityArchetype("", types);
    const Entity entity = EntityManager::CreateEntity(archetype, m_name);
    auto& entityInfo = EntityManager::GetInstance().m_currentAttachedWorldEntityStorage->m_entityInfos.at(entity.GetIndex());
    entityInfo.m_static = m_static;
    entityMap[m_entityHandle] = entity.GetHandle();
    for (auto &i : m_dataComponents)
    {
        EntityManager::SetDataComponent(entity.GetIndex(), i.m_type.m_typeId, i.m_type.m_size, i.m_data.get());
    }
    int index = 0;
    for (const auto &i : m_children)
    {
        AttachChildren(i, entity, m_name + "_" + std::to_string(index), entityMap);
        index++;
    }

    for (auto &i : m_privateComponents)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            SerializationManager::ProduceSerializable(i->GetTypeName(), id));
        ptr->Clone(i);
        ptr->Relink(entityMap);
        EntityManager::SetPrivateComponent(entity, ptr);
    }
    for (const auto &i : m_children)
    {
        AttachChildrenPrivateComponent(i, entity, entityMap);
        index++;
    }
    entity.SetEnabled(m_enabled);
    return entity;
}
void Prefab::AttachChildren(
    const std::shared_ptr<Prefab> &modelNode,
    Entity parentEntity,
    const std::string &parentName,
    std::unordered_map<Handle, Handle> &map) const
{
    std::vector<DataComponentType> types;
    for(auto& i : modelNode->m_dataComponents)
    {
        types.emplace_back(i.m_type);
    }
    auto archetype = EntityManager::CreateEntityArchetype("", types);
    Entity entity = EntityManager::CreateEntity(archetype, m_name);
    auto& entityInfo = EntityManager::GetInstance().m_currentAttachedWorldEntityStorage->m_entityInfos.at(entity.GetIndex());
    entityInfo.m_static = m_static;
    map[modelNode->m_entityHandle] = entity.GetHandle();
    entity.SetParent(parentEntity);
    for (auto &i : modelNode->m_dataComponents)
    {
        EntityManager::SetDataComponent(entity.GetIndex(), i.m_type.m_typeId, i.m_type.m_size, i.m_data.get());
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildren(i, entity, (parentName + "_" + std::to_string(index)), map);
        index++;
    }
}

void Prefab::AttachChildrenPrivateComponent(
    const std::shared_ptr<Prefab> &modelNode, Entity parentEntity, const std::unordered_map<Handle, Handle> &map) const
{
    Entity entity;
    auto children = parentEntity.GetChildren();
    for (auto &i : children)
    {
        auto a = i.GetHandle().GetValue();
        auto b = map.at(modelNode->m_entityHandle).GetValue();
        if (a == b)
            entity = i;
    }
    if (entity.IsNull())
        return;
    for (auto &i : modelNode->m_privateComponents)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            SerializationManager::ProduceSerializable(i->GetTypeName(), id));
        ptr->Clone(i);
        ptr->Relink(map);
        EntityManager::SetPrivateComponent(entity, ptr);
    }
    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildrenPrivateComponent(i, entity, map);
        index++;
    }
    entity.SetEnabled(m_enabled);
}
#pragma region Model Loading
void Prefab::Load(const std::filesystem::path &path)
{
    if (path.extension() == ".obj" || path.extension() == ".fbx")
    {
        unsigned flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
        bool optimize = false;
#ifdef USE_ASSIMP

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
            animation = AssetManager::CreateAsset<Animation>(path.filename().string());
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

            auto animator = SerializationManager::ProduceSerializable<Animator>();
            animator->Setup(animation);
            // animator->Animate();
            AttachAnimator(this, m_entityHandle);
            m_privateComponents.push_back(std::static_pointer_cast<IPrivateComponent>(animator));
        }
        return;
#else
        stbi_hdr_to_ldr_gamma(gamma);
        stbi_ldr_to_hdr_gamma(gamma);
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

        auto retVal = CreateResource<Model>();
        retVal->m_typeName = path.substr(path.find_last_of("/\\") + 1);
        auto &attribute = reader.GetAttrib();
        auto &shapes = reader.GetShapes();
        auto &materials = reader.GetMaterials();
        const std::string directory = path.substr(0, path.find_last_of('/'));
        std::map<std::string, std::shared_ptr<Texture2D>> loadedTextures;
        if (!optimize)
        {
            std::map<int, std::shared_ptr<Material>> loadedMaterials;
            for (const auto &i : shapes)
            {
                std::map<int, std::vector<Vertex>> meshMaterials;
                meshMaterials[-1] = std::vector<Vertex>();
                ProcessNode(directory, meshMaterials, i, attribute);
                for (auto &i : meshMaterials)
                {
                    std::unique_ptr<ModelNode> childNode = std::make_unique<ModelNode>();
                    const auto materialId = i.first;
                    auto &vertices = i.second;
                    if (vertices.empty())
                        continue;
                    const auto mask = (unsigned)VertexAttribute::Normal | (unsigned)VertexAttribute::TexCoord |
                                      (unsigned)VertexAttribute::Position | (unsigned)VertexAttribute::Color;
#pragma region Material
                    std::shared_ptr<Material> material;
                    if (materialId != -1)
                    {
                        auto search = loadedMaterials.find(materialId);
                        if (search != loadedMaterials.end())
                        {
                            material = search->second;
                        }
                        else
                        {
                            material = CreateResource<Material>();
                            auto &importedMaterial = materials[materialId];
                            material->m_metallic = importedMaterial.metallic == 0 ? 0.0f : importedMaterial.metallic;
                            material->m_roughness = importedMaterial.roughness == 0 ? 1.0f : importedMaterial.roughness;
                            material->m_albedoColor = glm::vec3(
                                importedMaterial.diffuse[0], importedMaterial.diffuse[1], importedMaterial.diffuse[2]);
#pragma region Textures
                            if (!importedMaterial.diffuse_texname.empty())
                            {
                                const auto albedo =
                                    CollectTexture(directory, importedMaterial.diffuse_texname, loadedTextures, gamma);
                                if (albedo)
                                {
                                    material->SetTexture(TextureType::Albedo, albedo);
                                }
                            }
                            if (!importedMaterial.bump_texname.empty())
                            {
                                const auto normal =
                                    CollectTexture(directory, importedMaterial.bump_texname, loadedTextures, gamma);
                                if (normal)
                                {
                                    material->SetTexture(TextureType::Normal, normal);
                                }
                            }
                            if (!importedMaterial.normal_texname.empty())
                            {
                                const auto normal =
                                    CollectTexture(directory, importedMaterial.normal_texname, loadedTextures, gamma);
                                if (normal)
                                {
                                    material->SetTexture(TextureType::Normal, normal);
                                }
                            }
                            if (!importedMaterial.roughness_texname.empty())
                            {
                                const auto roughness = CollectTexture(
                                    directory, importedMaterial.roughness_texname, loadedTextures, gamma);
                                if (roughness)
                                {
                                    material->SetTexture(TextureType::Roughness, roughness);
                                }
                            }
                            if (!importedMaterial.specular_highlight_texname.empty())
                            {
                                const auto roughness = CollectTexture(
                                    directory, importedMaterial.specular_highlight_texname, loadedTextures, gamma);
                                if (roughness)
                                {
                                    material->SetTexture(TextureType::Roughness, roughness);
                                }
                            }
                            if (!importedMaterial.metallic_texname.empty())
                            {
                                const auto metallic =
                                    CollectTexture(directory, importedMaterial.metallic_texname, loadedTextures, gamma);
                                if (metallic)
                                {
                                    material->SetTexture(TextureType::Metallic, metallic);
                                }
                            }
                            if (!importedMaterial.reflection_texname.empty())
                            {
                                const auto metallic = CollectTexture(
                                    directory, importedMaterial.reflection_texname, loadedTextures, gamma);
                                if (metallic)
                                {
                                    material->SetTexture(TextureType::Metallic, metallic);
                                }
                            }

                            if (!importedMaterial.ambient_texname.empty())
                            {
                                const auto ao =
                                    CollectTexture(directory, importedMaterial.ambient_texname, loadedTextures, gamma);
                                if (ao)
                                {
                                    material->SetTexture(TextureType::AO, ao);
                                }
                            }
#pragma endregion
                            loadedMaterials[materialId] = material;
                        }
                    }
                    else
                    {
                        material = DefaultResources::Materials::StandardMaterial;
                    }
#pragma endregion
#pragma region Mesh
                    auto mesh = CreateResource<Mesh>();
                    auto indices = std::vector<unsigned>();
                    assert(vertices.size() % 3 == 0);
                    for (unsigned i = 0; i < vertices.size(); i++)
                    {
                        indices.push_back(i);
                    }
                    mesh->SetVertices(mask, vertices, indices);
#pragma endregion
                    childNode->m_localTransform.m_value = glm::translate(glm::vec3(0.0f)) *
                                                          glm::mat4_cast(glm::quat(glm::vec3(0.0f))) *
                                                          glm::scale(glm::vec3(1.0f));
                    childNode->m_mesh = mesh;
                    childNode->m_value = material;
                    retVal->RootNode()->m_children.push_back(std::move(childNode));
                }
            }
        }
        else
        {
            std::map<int, std::vector<Vertex>> meshMaterials;
            meshMaterials[-1] = std::vector<Vertex>();
            for (const auto &i : shapes)
            {
                ProcessNode(directory, meshMaterials, i, attribute);
            }

            for (auto &i : meshMaterials)
            {
                std::unique_ptr<ModelNode> childNode = std::make_unique<ModelNode>();
                const auto materialId = i.first;
                auto &vertices = i.second;
                if (vertices.empty())
                    continue;
                const auto mask = (unsigned)VertexAttribute::Normal | (unsigned)VertexAttribute::TexCoord |
                                  (unsigned)VertexAttribute::Position | (unsigned)VertexAttribute::Color;
#pragma region Material
                auto material = std::make_shared<Material>();
                if (materialId != -1)
                {
                    auto &importedMaterial = materials[materialId];
                    material->m_metallic = importedMaterial.metallic;
                    material->m_roughness = importedMaterial.roughness;
                    material->m_albedoColor = glm::vec3(
                        importedMaterial.diffuse[0], importedMaterial.diffuse[1], importedMaterial.diffuse[2]);
                    if (!importedMaterial.diffuse_texname.empty())
                    {
                        const auto albedo =
                            CollectTexture(directory, importedMaterial.diffuse_texname, loadedTextures, gamma);
                        if (albedo)
                        {
                            material->SetTexture(TextureType::Albedo, albedo);
                        }
                    }
                    if (!importedMaterial.bump_texname.empty())
                    {
                        const auto normal =
                            CollectTexture(directory, importedMaterial.bump_texname, loadedTextures, gamma);
                        if (normal)
                        {
                            material->SetTexture(TextureType::Normal, normal);
                        }
                    }
                    if (!importedMaterial.normal_texname.empty())
                    {
                        const auto normal =
                            CollectTexture(directory, importedMaterial.normal_texname, loadedTextures, gamma);
                        if (normal)
                        {
                            material->SetTexture(TextureType::Normal, normal);
                        }
                    }
                    if (!importedMaterial.roughness_texname.empty())
                    {
                        const auto roughness =
                            CollectTexture(directory, importedMaterial.roughness_texname, loadedTextures, gamma);
                        if (roughness)
                        {
                            material->SetTexture(TextureType::Roughness, roughness);
                        }
                    }
                    if (!importedMaterial.specular_highlight_texname.empty())
                    {
                        const auto roughness = CollectTexture(
                            directory, importedMaterial.specular_highlight_texname, loadedTextures, gamma);
                        if (roughness)
                        {
                            material->SetTexture(TextureType::Roughness, roughness);
                        }
                    }
                    if (!importedMaterial.metallic_texname.empty())
                    {
                        const auto metallic =
                            CollectTexture(directory, importedMaterial.metallic_texname, loadedTextures, gamma);
                        if (metallic)
                        {
                            material->SetTexture(TextureType::Metallic, metallic);
                        }
                    }
                    if (!importedMaterial.reflection_texname.empty())
                    {
                        const auto metallic =
                            CollectTexture(directory, importedMaterial.reflection_texname, loadedTextures, gamma);
                        if (metallic)
                        {
                            material->SetTexture(TextureType::Metallic, metallic);
                        }
                    }

                    if (!importedMaterial.ambient_texname.empty())
                    {
                        const auto ao =
                            CollectTexture(directory, importedMaterial.ambient_texname, loadedTextures, gamma);
                        if (ao)
                        {
                            material->SetTexture(TextureType::AO, ao);
                        }
                    }
                }
                else
                {
                    material = DefaultResources::Materials::StandardMaterial;
                }
#pragma endregion
#pragma region Mesh
                auto mesh = std::make_shared<Mesh>();
                auto indices = std::vector<unsigned>();
                assert(vertices.size() % 3 == 0);
                for (unsigned i = 0; i < vertices.size(); i++)
                {
                    indices.push_back(i);
                }
                mesh->SetVertices(mask, vertices, indices);
#pragma endregion
                childNode->m_localTransform.m_value = glm::translate(glm::vec3(0.0f)) *
                                                      glm::mat4_cast(glm::quat(glm::vec3(0.0f))) *
                                                      glm::scale(glm::vec3(1.0f));
                childNode->m_mesh = mesh;
                childNode->m_value = material;
                retVal->RootNode()->m_children.push_back(std::move(childNode));
            }
        }
        if (addResource)
            Share(retVal);
        return retVal;
#endif
    }
}
#ifdef USE_ASSIMP
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
    auto texture2D = AssetManager::Import<Texture2D>(directory + "/" + path);
    loadedTextures[fileName] = texture2D;
    return texture2D;
}
std::shared_ptr<Material> Prefab::ReadMaterial(
    const std::string &directory,
    const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
    std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
    aiMaterial *importerMaterial)
{
    auto targetMaterial = AssetManager::LoadMaterial(glProgram);

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
        targetMaterial->SetTexture(TextureType::Roughness, CollectTexture(directory, str.C_Str(), texture2DsLoaded));
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
        auto childNode = SerializationManager::ProduceSerializable<Prefab>();
        const auto search = loadedMaterials.find(importerMesh->mMaterialIndex);
        bool isSkinnedMesh = !importerMesh->HasBones();
        std::shared_ptr<Material> material;
        if (search == loadedMaterials.end())
        {
            aiMaterial *importerMaterial = importerScene->mMaterials[importerMesh->mMaterialIndex];
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

        if (importerMesh->HasBones())
        {
            auto skinnedMeshRenderer = SerializationManager::ProduceSerializable<SkinnedMeshRenderer>();
            skinnedMeshRenderer->m_material.Set<Material>(material);
            skinnedMeshRenderer->m_skinnedMesh.Set<SkinnedMesh>(ReadSkinnedMesh(boneMaps, importerMesh));
            if (!skinnedMeshRenderer->m_skinnedMesh.Get())
                continue;
            addedMeshRenderer = true;
            skinnedMeshRenderer->m_skinnedMesh.Get<SkinnedMesh>()->m_animation.Set<Animation>(animation);
            childNode->m_privateComponents.push_back(std::static_pointer_cast<IPrivateComponent>(skinnedMeshRenderer));
        }
        else
        {
            auto meshRenderer = SerializationManager::ProduceSerializable<MeshRenderer>();
            meshRenderer->m_material.Set<Material>(material);
            meshRenderer->m_mesh.Set<Mesh>(ReadMesh(importerMesh));
            if (!meshRenderer->m_mesh.Get())
                continue;
            addedMeshRenderer = true;
            childNode->m_privateComponents.push_back(std::static_pointer_cast<IPrivateComponent>(meshRenderer));
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
        auto childNode = std::make_shared<Prefab>();
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
        glm::vec4 v4;
        if (importerMesh->HasVertexColors(0))
        {
            v4.x = importerMesh->mColors[0][i].r;
            v4.y = importerMesh->mColors[0][i].g;
            v4.z = importerMesh->mColors[0][i].b;
            v4.w = importerMesh->mColors[0][i].a;
            vertex.m_color = v4;
            mask = mask | static_cast<unsigned>(VertexAttribute::Color);
        }
        glm::vec2 v2;
        if (importerMesh->HasTextureCoords(0))
        {
            v2.x = importerMesh->mTextureCoords[0][i].x;
            v2.y = importerMesh->mTextureCoords[0][i].y;
            vertex.m_texCoords = v2;
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        else
        {
            vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
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
    auto mesh = AssetManager::CreateAsset<Mesh>();
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
        glm::vec4 v4;
        if (importerMesh->HasVertexColors(0))
        {
            v4.x = importerMesh->mColors[0][i].r;
            v4.y = importerMesh->mColors[0][i].g;
            v4.z = importerMesh->mColors[0][i].b;
            v4.w = importerMesh->mColors[0][i].a;
            vertex.m_color = v4;
            mask = mask | static_cast<unsigned>(VertexAttribute::Color);
        }
        glm::vec2 v2;
        if (importerMesh->HasTextureCoords(0))
        {
            v2.x = importerMesh->mTextureCoords[0][i].x;
            v2.y = importerMesh->mTextureCoords[0][i].y;
            vertex.m_texCoords = v2;
            mask = mask | static_cast<unsigned>(VertexAttribute::TexCoord);
        }
        else
        {
            vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
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
    auto skinnedMesh = AssetManager::CreateAsset<SkinnedMesh>(importerMesh->mName.C_Str());
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
        smr->m_skinnedMesh.Get<SkinnedMesh>()->FetchIndices();
    for (auto &i : node->m_children)
    {
        ApplyBoneIndices(i.get());
    }
}
void Prefab::FromEntity(const Entity &entity)
{
    m_entityHandle = entity.GetHandle();
    m_name = entity.GetName();
    m_enabled = entity.IsEnabled();
    m_static = entity.IsStatic();
    EntityManager::UnsafeForEachDataComponent(entity, [&](const DataComponentType &type, void *data) {
        DataComponentHolder holder;
        holder.m_type = type;
        size_t id;
        size_t size;
        holder.m_data = std::static_pointer_cast<IDataComponent>(
            SerializationManager::ProduceDataComponent(type.m_name, id, size));
        memcpy(holder.m_data.get(), data, type.m_size);
        m_dataComponents.push_back(std::move(holder));
    });

    auto &elements =
        EntityManager::GetInstance().m_entityMetaDataCollection->at(entity.GetIndex()).m_privateComponentElements;
    for (auto &element : elements)
    {
        size_t id;
        auto ptr = std::static_pointer_cast<IPrivateComponent>(
            SerializationManager::ProduceSerializable(element.m_privateComponentData->GetTypeName(), id));
        ptr->Clone(element.m_privateComponentData);
        ptr->m_started = false;
        m_privateComponents.push_back(ptr);
    }

    auto children = entity.GetChildren();
    for (auto &i : children)
    {
        m_children.push_back(AssetManager::CreateAsset<Prefab>(i.GetName()));
        m_children.back()->FromEntity(i);
    }
}
#else
void Prefab::ProcessNode(
    const std::string &directory,
    std::map<int, std::vector<Vertex>> &meshMaterials,
    const tinyobj::shape_t &shape,
    const tinyobj::attrib_t &attribute)
{
    std::unique_ptr<ModelNode> childNode = std::make_unique<ModelNode>();

    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
    {
        // per-face material
        int materialId = -1;
        if (shape.mesh.material_ids[f] != -1)
        {
            materialId = shape.mesh.material_ids[f];
            if (meshMaterials.find(materialId) == meshMaterials.end())
            {
                meshMaterials[materialId] = std::vector<Vertex>();
            }
        }
        const size_t fv = size_t(shape.mesh.num_face_vertices[f]);
        // Loop over vertices in the face.
        Vertex vertices[3];
        bool recalculateNormal = false;
        for (size_t v = 0; v < fv; v++)
        {
            Vertex &vertex = vertices[v];
            // access to vertex
            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            vertex.m_position.x = attribute.vertices[3 * size_t(idx.vertex_index) + 0];
            vertex.m_position.y = attribute.vertices[3 * size_t(idx.vertex_index) + 1];
            vertex.m_position.z = attribute.vertices[3 * size_t(idx.vertex_index) + 2];

            // Check if `normal_index` is zero or positive. negative = no normal data
            if (idx.normal_index >= 0)
            {
                vertex.m_normal.x = attribute.normals[3 * size_t(idx.normal_index) + 0];
                vertex.m_normal.y = attribute.normals[3 * size_t(idx.normal_index) + 1];
                vertex.m_normal.z = attribute.normals[3 * size_t(idx.normal_index) + 2];
            }
            else
            {
                recalculateNormal = true;
            }

            // Check if `texcoord_index` is zero or positive. negative = no texcoord data
            if (idx.texcoord_index >= 0)
            {
                vertex.m_texCoords.x = attribute.texcoords[2 * size_t(idx.texcoord_index) + 0];
                vertex.m_texCoords.y = attribute.texcoords[2 * size_t(idx.texcoord_index) + 1];
            }
            else
            {
                vertex.m_texCoords = glm::vec2(0.0f);
            }

            if (!attribute.colors.empty())
            {
                vertex.m_color.x = attribute.colors[3 * size_t(idx.vertex_index) + 0];
                vertex.m_color.y = attribute.colors[3 * size_t(idx.vertex_index) + 1];
                vertex.m_color.z = attribute.colors[3 * size_t(idx.vertex_index) + 2];
            }
            else
            {
                vertex.m_color = glm::vec4(1.0f);
            }
        }
        if (recalculateNormal)
        {
            vertices[0].m_normal = vertices[1].m_normal = vertices[2].m_normal = -glm::normalize(glm::cross(
                vertices[0].m_position - vertices[1].m_position, vertices[0].m_position - vertices[2].m_position));
        }
        meshMaterials[materialId].push_back(vertices[0]);
        meshMaterials[materialId].push_back(vertices[1]);
        meshMaterials[materialId].push_back(vertices[2]);
        index_offset += fv;
    }
}
#endif
#pragma endregion