#include <Application.hpp>
#include <AssetManager.hpp>
#include <Core/FileSystem.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <MeshRenderer.hpp>
#include <RenderManager.hpp>
#include <SerializationManager.hpp>
#include <SkinnedMeshRenderer.hpp>
using namespace UniEngine;

void AssetManager::RemoveFromShared(const std::string &typeName, const AssetHandle &handle)
{
    if(handle < DefaultResources::GetInstance().m_currentHandle)
    {
        UNIENGINE_WARNING("Not allowed to remove internal assets!");
        return;
    }
    GetInstance().m_sharedAssets[typeName].erase(handle);
}

std::shared_ptr<Model> AssetManager::LoadModel(
    std::string const &path, const unsigned &flags, const bool &optimize, const float &gamma)
{
#ifdef USE_ASSIMP

    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, flags);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        UNIENGINE_LOG("Assimp: " + std::string(importer.GetErrorString()));
        return nullptr;
    }
    // retrieve the directory path of the filepath
    const std::string directory = path.substr(0, path.find_last_of('/'));
    std::map<std::string, std::shared_ptr<Texture2D>> texture2DsLoaded;
    auto retVal = CreateAsset<Model>();
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    std::map<unsigned, std::shared_ptr<Material>> loadedMaterials;
    std::map<std::string, std::shared_ptr<Bone>> bonesMap;
    if (!bonesMap.empty() || scene->HasAnimations())
    {
        retVal->m_animation = CreateAsset<Animation>(true);
        retVal->m_animation->m_name = path.substr(path.find_last_of("/\\") + 1);
    }
    std::shared_ptr<AssimpNode> rootAssimpNode = std::make_shared<AssimpNode>(scene->mRootNode);
    if (!ProcessNode(
            directory,
            retVal->RootNode(),
            loadedMaterials,
            texture2DsLoaded,
            bonesMap,
            scene->mRootNode,
            rootAssimpNode,
            scene,
            retVal->m_animation,
            gamma))
    {
        UNIENGINE_ERROR("Model is empty!");
        return nullptr;
    }
    if (!bonesMap.empty() || scene->HasAnimations())
    {
        rootAssimpNode->NecessaryWalker(bonesMap);
        size_t index = 0;
        rootAssimpNode->AttachToAnimator(retVal->m_animation, index);
        retVal->m_animation->m_boneSize = index + 1;
        ReadAnimations(scene, retVal->m_animation, bonesMap);
        ApplyBoneIndices(retVal->m_rootNode);
    }
    return retVal;
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
                material->m_albedoColor =
                    glm::vec3(importedMaterial.diffuse[0], importedMaterial.diffuse[1], importedMaterial.diffuse[2]);
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
                    const auto normal = CollectTexture(directory, importedMaterial.bump_texname, loadedTextures, gamma);
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
                    const auto roughness =
                        CollectTexture(directory, importedMaterial.specular_highlight_texname, loadedTextures, gamma);
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
                    const auto ao = CollectTexture(directory, importedMaterial.ambient_texname, loadedTextures, gamma);
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

Entity AssetManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model)
{
    const Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(model->m_name);
    std::shared_ptr<ModelNode> &modelNode = model->RootNode();

    if (modelNode->m_mesh)
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        mmc->m_mesh = modelNode->m_mesh;
        mmc->m_material = modelNode->m_material;
    }
    else if (modelNode->m_skinnedMesh)
    {
        auto smmc = entity.GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
        smmc->m_skinnedMesh = modelNode->m_skinnedMesh;
        smmc->m_material = modelNode->m_material;
    }
    int index = 0;

    for (auto &i : modelNode->m_children)
    {
        AttachChildren(archetype, i, entity, model->m_name + "_" + std::to_string(index));
        index++;
    }
    if (model->m_animation)
    {
        auto animator = entity.GetOrSetPrivateComponent<Animator>().lock();
        animator->Setup(model->m_animation);
        animator->Animate();
        AttachAnimator(entity, entity);
    }
    return entity;
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

void AssetManager::AttachAnimator(const Entity &parent, const Entity &animator)
{
    if (parent.HasPrivateComponent<SkinnedMeshRenderer>())
    {
        parent.GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock()->AttachAnimator(animator);
    }
    parent.ForEachChild([&](Entity child) { AttachAnimator(child, animator); });
}
#ifdef USE_ASSIMP

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

void AssetManager::ReadKeyFrame(BoneKeyFrames &boneAnimation, const aiNodeAnim *channel)
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
void AssetManager::ReadAnimations(
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

std::shared_ptr<Material> AssetManager::ReadMaterial(
    const std::string &directory,
    const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
    std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
    aiMaterial *importerMaterial,
    const float &gamma)
{
    auto targetMaterial = LoadMaterial(glProgram);

    // PBR
    if (importerMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Albedo, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Albedo, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Normal, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_METALNESS, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Metallic, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Roughness, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
        targetMaterial->SetTexture(TextureType::AO, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    if (importerMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        aiString str;
        importerMaterial->GetTexture(aiTextureType_HEIGHT, 0, &str);
        targetMaterial->SetTexture(
            TextureType::Normal, CollectTexture(directory, str.C_Str(), texture2DsLoaded, gamma));
    }
    return targetMaterial;
}

bool AssetManager::ProcessNode(
    const std::string &directory,
    std::shared_ptr<ModelNode> &modelNode,
    std::map<unsigned, std::shared_ptr<Material>> &loadedMaterials,
    std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
    std::map<std::string, std::shared_ptr<Bone>> &boneMaps,
    aiNode *importerNode,
    std::shared_ptr<AssimpNode> assimpNode,
    const aiScene *importerScene,
    const std::shared_ptr<Animation> &animator,
    const float &gamma)
{
    bool addedMeshRenderer = false;
    for (unsigned i = 0; i < importerNode->mNumMeshes; i++)
    {
        // the modelNode object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, modelNode is just to keep stuff organized (like relations between nodes).
        aiMesh *importerMesh = importerScene->mMeshes[importerNode->mMeshes[i]];
        if (!importerMesh)
            continue;
        auto childNode = std::make_shared<ModelNode>();
        const auto search = loadedMaterials.find(importerMesh->mMaterialIndex);

        if (importerMesh->HasBones())
        {
            const auto skinnedMesh = ReadSkinnedMesh(boneMaps, importerMesh);
            if (!skinnedMesh)
                continue;
            addedMeshRenderer = true;
            skinnedMesh->m_animation = animator;
            childNode->m_skinnedMesh = skinnedMesh;
            childNode->m_type = ModelNodeType::SkinnedMesh;
        }
        else
        {
            const auto mesh = ReadMesh(importerMesh);
            if (!mesh)
                continue;
            addedMeshRenderer = true;
            childNode->m_mesh = mesh;
            childNode->m_type = ModelNodeType::Mesh;
        }

        if (search == loadedMaterials.end())
        {
            aiMaterial *importerMaterial = importerScene->mMaterials[importerMesh->mMaterialIndex];
            childNode->m_material = ReadMaterial(
                directory,
                childNode->m_type == ModelNodeType::SkinnedMesh ? DefaultResources::GLPrograms::StandardSkinnedProgram
                                                                : DefaultResources::GLPrograms::StandardProgram,
                texture2DsLoaded,
                importerMaterial,
                gamma);
        }
        else
        {
            childNode->m_material = search->second;
        }

        childNode->m_localTransform.m_value = mat4_cast(importerNode->mTransformation);
        if (!importerNode->mParent)
            childNode->m_localTransform = Transform();
        childNode->m_parent = modelNode;
        modelNode->m_children.push_back(std::move(childNode));
    }

    for (unsigned i = 0; i < importerNode->mNumChildren; i++)
    {
        auto childNode = std::make_shared<ModelNode>();
        auto childAssimpNode = std::make_shared<AssimpNode>(importerNode->mChildren[i]);
        childAssimpNode->m_parent = assimpNode;
        const bool childAdd = ProcessNode(
            directory,
            childNode,
            loadedMaterials,
            texture2DsLoaded,
            boneMaps,
            importerNode->mChildren[i],
            childAssimpNode,
            importerScene,
            animator,
            gamma);
        if (childAdd)
        {
            childNode->m_parent = modelNode;
            modelNode->m_children.push_back(std::move(childNode));
        }
        addedMeshRenderer = addedMeshRenderer | childAdd;
        assimpNode->m_children.push_back(std::move(childAssimpNode));
    }
    return addedMeshRenderer;
}

std::shared_ptr<Mesh> AssetManager::ReadMesh(aiMesh *importerMesh)
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
    auto mesh = CreateAsset<Mesh>();
    mesh->SetVertices(mask, vertices, indices);
    return mesh;
}

std::shared_ptr<SkinnedMesh> AssetManager::ReadSkinnedMesh(
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
    auto skinnedMesh = AssetManager::CreateAsset<SkinnedMesh>();
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
#else
void AssetManager::ProcessNode(
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

std::shared_ptr<Texture2D> AssetManager::CollectTexture(
    const std::string &directory,
    const std::string &path,
    std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures,
    const float &gamma)
{
    const std::string fileName = directory + "/" + path;
    if (const auto search = loadedTextures.find(fileName); search != loadedTextures.end())
    {
        return search->second;
    }
    auto texture2D = LoadTexture(directory + "/" + path, gamma);
    loadedTextures[fileName] = texture2D;
    return texture2D;
}

void AssetManager::ApplyBoneIndices(std::shared_ptr<ModelNode> &node)
{
    if (node->m_skinnedMesh)
        node->m_skinnedMesh->FetchIndices();
    for (auto &i : node->m_children)
    {
        ApplyBoneIndices(i);
    }
}

void UniEngine::AssetManager::AttachChildren(
    EntityArchetype archetype, std::shared_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName)
{
    Entity entity = EntityManager::CreateEntity(archetype);
    entity.SetName(parentName);
    entity.SetParent(parentEntity);
    entity.SetDataComponent(modelNode->m_localTransform);
    if (modelNode->m_mesh)
    {
        auto mmc = entity.GetOrSetPrivateComponent<MeshRenderer>().lock();
        mmc->m_mesh = modelNode->m_mesh;
        mmc->m_material = modelNode->m_material;
    }
    else if (modelNode->m_skinnedMesh)
    {
        auto smmc = entity.GetOrSetPrivateComponent<SkinnedMeshRenderer>().lock();
        smmc->m_skinnedMesh = modelNode->m_skinnedMesh;
        smmc->m_material = modelNode->m_material;
    }

    int index = 0;
    for (auto &i : modelNode->m_children)
    {
        AttachChildren(archetype, i, entity, (parentName + "_" + std::to_string(index)));
        index++;
    }
}

std::shared_ptr<Texture2D> AssetManager::LoadTexture(const std::string &path, const float &gamma)
{
    stbi_set_flip_vertically_on_load(true);
    auto retVal = CreateAsset<Texture2D>();
    const std::string filename = path;
    retVal->m_path = filename;
    int width, height, nrComponents;
    float actualGamma = gamma;
    if (gamma == 0.0f)
    {
        if (path.substr(path.find_last_of(".") + 1) == "hdr")
        {
            actualGamma = 2.2f;
        }
        else
        {
            actualGamma = 1.0f;
        }
    }
    stbi_hdr_to_ldr_gamma(actualGamma);
    stbi_ldr_to_hdr_gamma(actualGamma);

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
        GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(width, height))) + 1;
        retVal->m_texture = std::make_shared<OpenGLUtils::GLTexture2D>(mipmap, GL_RGBA32F, width, height, true);
        retVal->m_texture->SetData(0, format, GL_FLOAT, data);
        retVal->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_REPEAT);
        retVal->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_REPEAT);
        retVal->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        retVal->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    retVal->m_gamma = actualGamma;
    return retVal;
}

std::shared_ptr<Cubemap> AssetManager::LoadCubemap(const std::string &path, const float &gamma)
{
    auto &manager = GetInstance();
    stbi_set_flip_vertically_on_load(true);
    auto texture2D = CreateAsset<Texture2D>();
    const std::string filename = path;
    texture2D->m_path = filename;
    int width, height, nrComponents;
    float actualGamma = gamma;
    if (gamma == 0.0f)
    {
        if (path.substr(path.find_last_of(".") + 1) == "hdr")
        {
            actualGamma = 2.2f;
        }
        else
        {
            actualGamma = 1.0f;
        }
    }
    stbi_hdr_to_ldr_gamma(actualGamma);
    stbi_ldr_to_hdr_gamma(actualGamma);
    float *data = stbi_loadf(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        texture2D->m_texture = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RGB32F, width, height, true);
        texture2D->m_texture->SetData(0, GL_RGB, GL_FLOAT, data);
        texture2D->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        texture2D->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture2D->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture2D->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        UNIENGINE_LOG("Texture failed to load at path: " + filename);
        stbi_image_free(data);
        return nullptr;
    }

#pragma region Conversion
    // pbr: setup framebuffer
    // ----------------------
    size_t resolution = 1024;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(resolution, resolution))) + 1;
    auto envCubemap = std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGB32F, resolution, resolution, true);
    envCubemap->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    envCubemap->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    envCubemap->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    envCubemap->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    envCubemap->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    manager.m_2DToCubemapProgram->Bind();
    manager.m_2DToCubemapProgram->SetInt("equirectangularMap", 0);
    manager.m_2DToCubemapProgram->SetFloat4x4("projection", captureProjection);
    texture2D->m_texture->Bind(0);
    renderTarget->GetFrameBuffer()->ViewPort(resolution, resolution);
    for (unsigned int i = 0; i < 6; ++i)
    {
        manager.m_2DToCubemapProgram->SetFloat4x4("view", captureViews[i]);
        renderTarget->AttachTexture2D(envCubemap.get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        renderTarget->Clear();
        RenderManager::RenderCube();
    }
    OpenGLUtils::GLFrameBuffer::BindDefault();
    envCubemap->GenerateMipMap();
#pragma endregion
    auto retVal = CreateAsset<Cubemap>();
    retVal->m_texture = std::move(envCubemap);
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    retVal->m_gamma = actualGamma;
    return retVal;
}

std::shared_ptr<EnvironmentalMap> AssetManager::LoadEnvironmentalMap(const std::string &path, const float &gamma)
{
    auto retVal = CreateAsset<EnvironmentalMap>();
    retVal->Construct(LoadCubemap(path, gamma));
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    return retVal;
}

std::shared_ptr<LightProbe> AssetManager::LoadLightProbe(const std::string &path, const float &gamma)
{
    auto retVal = CreateAsset<LightProbe>();
    retVal->ConstructFromCubemap(LoadCubemap(path, gamma));
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    return retVal;
}

std::shared_ptr<ReflectionProbe> AssetManager::LoadReflectionProbe(const std::string &path, const float &gamma)
{
    auto retVal = CreateAsset<ReflectionProbe>();
    retVal->ConstructFromCubemap(LoadCubemap(path, gamma));
    retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
    return retVal;
}

std::shared_ptr<Cubemap> AssetManager::LoadCubemap(const std::vector<std::string> &paths, const float &gamma)
{
    int width, height, nrComponents;
    auto size = paths.size();
    if (size != 6)
    {
        UNIENGINE_ERROR("Texture::LoadCubeMap: Size error.");
        return nullptr;
    }

    float *temp = stbi_loadf(paths[0].c_str(), &width, &height, &nrComponents, 0);
    stbi_image_free(temp);
    GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(width, height))) + 1;
    auto texture = std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGBA32F, width, height, true);
    float actualGamma = gamma;
    for (int i = 0; i < size; i++)
    {
        if (gamma == 0.0f)
        {
            if (paths[i].substr(paths[i].find_last_of(".") + 1) == "hdr")
            {
                actualGamma = 2.2f;
            }
            else
            {
                actualGamma = 1.0f;
            }
        }
        stbi_hdr_to_ldr_gamma(actualGamma);
        stbi_ldr_to_hdr_gamma(actualGamma);
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

    texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    texture->GenerateMipMap();
    auto retVal = CreateAsset<Cubemap>();
    retVal->m_texture = std::move(texture);
    retVal->m_name = paths[0].substr(paths[0].find_last_of("/\\") + 1);
    retVal->m_gamma = actualGamma;
    return retVal;
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
            if (ImGui::BeginMenu("Load..."))
            {
                FileSystem::OpenFile("Scene##Load", ".uescene", [](const std::string &filePath) {
                    std::shared_ptr<Scene> scene = AssetManager::CreateAsset<Scene>();
                    bool successful = true;
                    try
                    {
                        SerializableFactory::Deserialize(filePath, scene);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        successful = false;
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                    if (successful)
                        Share(scene);
                });

#ifdef USE_ASSIMP
                std::string modelFormat = ".obj,.gltf,.glb,.blend,.ply,.fbx,.dae";
#else
                std::string modelFormat = ".obj";
#endif
                FileSystem::OpenFile("Model##Load", modelFormat, [](const std::string &filePath) {
                    bool successful = true;
                    std::shared_ptr<Model> model;
                    try
                    {
                        model = LoadModel(filePath);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        successful = false;
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                    if (successful)
                        Share(model);
                });

                FileSystem::OpenFile("Texture2D##Load", ".png,.jpg,.jpeg,.tga,.hdr", [](const std::string &filePath) {
                    bool successful = true;
                    std::shared_ptr<Texture2D> texture2D;
                    try
                    {
                        texture2D = LoadTexture(filePath);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        successful = false;
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                    if (successful)
                        Share(texture2D);
                });

                FileSystem::OpenFile("Cubemap##Load", ".png,.jpg,.jpeg,.tga,.hdr", [](const std::string &filePath) {
                    bool successful = true;
                    std::shared_ptr<Cubemap> cubeMap;
                    try
                    {
                        cubeMap = LoadCubemap(filePath);
                        UNIENGINE_LOG("Loaded from " + filePath);
                    }
                    catch (std::exception &e)
                    {
                        successful = false;
                        UNIENGINE_ERROR("Failed to load from " + filePath);
                    }
                    if (successful)
                        Share(cubeMap);
                });
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Save..."))
            {
                FileSystem::SaveFile("Scene##Load", ".uescene", [](const std::string &filePath) {
                  try
                  {
                      SerializableFactory::Serialize(filePath, EntityManager::GetCurrentScene());
                      UNIENGINE_LOG("Saved to " + filePath);
                  }
                  catch (std::exception &e)
                  {
                      UNIENGINE_ERROR("Failed to save to " + filePath);
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
        ImGui::Begin("Resource Manager");
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
                                Share(payload_n);
                            }
                            ImGui::EndDragDropTarget();
                        }
                        for (auto &i : collection.second)
                        {
                            if (EditorManager::Draggable(collection.first, i.second))
                            {
                                RemoveFromShared(collection.first, i.second->GetHandle());
                                break;
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
        AssetHandle assetHandle(inAssetRecord["Handle"].as<uint64_t>());
        AssetRecord assetRecord;
        assetRecord.m_filePath = inAssetRecord["FilePath"].as<std::string>();
        assetRecord.m_typeName = inAssetRecord["TypeName"].as<std::string>();
        m_assetRecords.insert({assetHandle, assetRecord});
    }
}


std::string AssetManager::GetAssetRootPath()
{
    std::string assetRootFolder = GetProjectPath() + "Assets/";
    if (!std::filesystem::exists(assetRootFolder))
    {
        std::filesystem::create_directory(assetRootFolder);
    }
    return assetRootFolder;
}

void AssetManager::SetProjectPath(const std::string &path)
{
    GetInstance().m_projectPath = path;
    std::string assetRootFolder = GetAssetRootPath();
    for(const auto& i : SerializableFactory::GetInstance().m_serializableNames){
        auto assetFolderPath = assetRootFolder + i.second + "/";
        if (!std::filesystem::exists(assetFolderPath))
        {
            std::filesystem::create_directory(assetFolderPath);
        }
    }
}

std::string AssetManager::GetProjectPath()
{
    auto &path = GetInstance().m_projectPath;
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }
    return path;
}

void AssetManager::SetResourcePath(const std::string &path)
{
    GetInstance().m_resourceRootPath = path;
}

std::string AssetManager::GetResourcePath()
{
    return GetInstance().m_resourceRootPath;
}
