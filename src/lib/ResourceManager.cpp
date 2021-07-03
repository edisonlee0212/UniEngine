#include <Application.hpp>
#include <DefaultResources.hpp>
#include <EditorManager.hpp>
#include <FileIO.hpp>
#include <Gui.hpp>
#include <MeshRenderer.hpp>
#include <SkinnedMeshRenderer.hpp>
#include <RenderManager.hpp>
#include <ResourceManager.hpp>
#include <SerializationManager.hpp>
using namespace UniEngine;

void ResourceManager::Remove(size_t id, size_t hashCode)
{
	GetInstance().m_resources[id].second.erase(hashCode);
}

std::shared_ptr<Model> ResourceManager::LoadModel(
	const bool &addResource,
	std::string const &path,
	std::shared_ptr<OpenGLUtils::GLProgram> glProgram,
	const unsigned &flags,
	const bool &optimize,
	const float &gamma)
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
	auto retVal = CreateResource<Model>();
	retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
	std::map<unsigned, std::shared_ptr<Material>> loadedMaterials;
	std::map<std::string, std::shared_ptr<Bone>> bonesMap;

	
	std::shared_ptr<AssimpNode> rootAssimpNode = std::make_shared<AssimpNode>(scene->mRootNode);
	if (!ProcessNode(
			directory,
			glProgram,
			retVal->RootNode(),
			loadedMaterials,
			texture2DsLoaded,
			bonesMap,
			scene->mRootNode,
			rootAssimpNode,
			scene,
			gamma))
	{
		UNIENGINE_ERROR("Model is empty!");
		return nullptr;
	}
    if (!bonesMap.empty() || scene->HasAnimations())
    {
        retVal->m_animator = std::make_shared<Animator>();
        rootAssimpNode->NecessaryWalker(bonesMap);
        rootAssimpNode->AttachToAnimation(retVal->m_animator);
        ReadAnimations(scene, retVal->m_animator, bonesMap);
    }
	
	if (addResource)
		Push(retVal);
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
	retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
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
						material->SetProgram(glProgram);
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
				childNode->m_localToParent.m_value = glm::translate(glm::vec3(0.0f)) *
													 glm::mat4_cast(glm::quat(glm::vec3(0.0f))) *
													 glm::scale(glm::vec3(1.0f));
				childNode->m_meshMaterials.emplace_back(material, mesh);
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
			childNode->m_localToParent.m_value = glm::translate(glm::vec3(0.0f)) *
												 glm::mat4_cast(glm::quat(glm::vec3(0.0f))) *
												 glm::scale(glm::vec3(1.0f));
			childNode->m_meshMaterials.emplace_back(material, mesh);
			retVal->RootNode()->m_children.push_back(std::move(childNode));
		}
	}
	if (addResource)
		Push(retVal);
	return retVal;
#endif
}

Entity ResourceManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model)
{
	const Entity entity = EntityManager::CreateEntity(archetype);
	entity.SetName(model->m_name);
	std::shared_ptr<ModelNode> &modelNode = model->RootNode();
    if (model->m_animator)
    {
        auto animator = std::make_unique<Animator>();
        animator->m_animationNames = model->m_animator->m_animationNames;
        animator->m_rootBone = model->m_animator->m_rootBone;
        entity.SetPrivateComponent(std::move(animator));
    }
	if (modelNode->m_mesh)
	{
		auto mmc = std::make_unique<MeshRenderer>();
		mmc->m_mesh = modelNode->m_mesh;
		mmc->m_material = modelNode->m_material;
		EntityManager::SetPrivateComponent<MeshRenderer>(entity, std::move(mmc));
	}
	else if (modelNode->m_skinnedMesh)
	{
		auto smmc = std::make_unique<SkinnedMeshRenderer>();
		smmc->m_skinnedMesh = modelNode->m_skinnedMesh;
		smmc->m_material = modelNode->m_material;
		EntityManager::SetPrivateComponent<SkinnedMeshRenderer>(entity, std::move(smmc));
	}
	int index = 0;
	for (auto &i : modelNode->m_children)
	{
		AttachChildren(archetype, i, entity, model->m_name + "_" + std::to_string(index));
		index++;
	}
	return entity;
}

Entity ResourceManager::ToEntity(EntityArchetype archetype, std::shared_ptr<Texture2D> texture)
{
	const Entity entity = EntityManager::CreateEntity(archetype);
	entity.SetName(texture->m_name);
	auto mmc = std::make_unique<MeshRenderer>();
	mmc->m_material = LoadMaterial(false, DefaultResources::GLPrograms::StandardProgram);
	mmc->m_material->SetTexture(TextureType::Albedo, texture);
	mmc->m_mesh = DefaultResources::Primitives::Quad;
	entity.SetPrivateComponent(std::move(mmc));
	return entity;
}
#ifdef USE_ASSIMP

AssimpNode::AssimpNode(aiNode *node)
{
	m_correspondingNode = node;
}
void AssimpNode::AttachToAnimation(std::shared_ptr<Animator> &animation)
{
    animation->m_rootBone = m_bone;
    for (auto& i : m_children)
    {
        i->AttachChild(m_bone);
    }
}

void AssimpNode::AttachChild(std::shared_ptr<Bone> &parent)
{
    parent->m_children.push_back(m_bone);
    for (auto &i : m_children)
    {
        i->AttachChild(m_bone);
    }
}

bool AssimpNode::NecessaryWalker(std::map<std::string, std::shared_ptr<Bone>> &boneMap)
{
	bool necessary = false;
	for (int i = 0; i < m_children.size(); i++)
	{
		if(!m_children[i]->NecessaryWalker(boneMap))
		{
			m_children.erase(m_children.begin() + i);
			i--;
		}else
		{
			necessary = true;
		}
	}
	auto search = boneMap.find(m_name);
	if (search != boneMap.end())
	{
        m_bone = search->second;
        necessary = true;
    }
    else if (necessary)
    {
        m_bone = std::make_shared<Bone>();
        m_bone->m_name = m_name;
        m_bone->m_isEntity = true;
    }
    
	return necessary;
}

void ResourceManager::ReadKeyFrame(BoneAnimation& boneAnimation, const aiNodeAnim *channel)
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
    }

    const auto numRotations = channel->mNumRotationKeys;
    boneAnimation.m_rotations.resize(numRotations);
    for (int rotationIndex = 0; rotationIndex < numRotations; ++rotationIndex)
    {
        const aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        const float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        BoneRotation data;
        data.m_value = glm::quat(aiOrientation.x, aiOrientation.y, aiOrientation.z, aiOrientation.w);
        data.m_timeStamp = timeStamp;
        boneAnimation.m_rotations.push_back(data);
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
    }
}
void ResourceManager::ReadAnimations(const aiScene* importerScene,
    std::shared_ptr<Animator> &animator, std::map<std::string, std::shared_ptr<Bone>> &bonesMap)
{
    for (int i = 0; i < importerScene->mNumAnimations; i++)
    {
        aiAnimation *importerAnimation = importerScene->mAnimations[i];
        const std::string animationName = importerAnimation->mName.C_Str();
        animator->m_animationNames.push_back(animationName);
        for (int j = 0; j < importerAnimation->mNumChannels; j++)
        {
            aiNodeAnim *importerNodeAmination = importerAnimation->mChannels[j];
            const std::string nodeName = importerNodeAmination->mNodeName.C_Str();
            const auto search = bonesMap.find(nodeName);
            if (search != bonesMap.end())
            {
                auto &bone = search->second;
                bone->m_animations[animationName] = BoneAnimation();
                ReadKeyFrame(bone->m_animations[animationName], importerNodeAmination);
            }
        }
    }
}

std::shared_ptr<Material> ResourceManager::ReadMaterial(
	const std::string &directory,
	const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
	std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
	aiMaterial *importerMaterial,
	const float &gamma)
{
	auto targetMaterial = LoadMaterial(false, glProgram);

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

bool ResourceManager::ProcessNode(
	const std::string &directory,
	const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
	std::shared_ptr<ModelNode> &modelNode,
	std::map<unsigned, std::shared_ptr<Material>> &loadedMaterials,
	std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
	std::map<std::string, std::shared_ptr<Bone>> &boneMaps, 
	aiNode *importerNode,
	std::shared_ptr<AssimpNode> assimpNode,
	const aiScene *importerScene,
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
		if (search == loadedMaterials.end())
		{
			aiMaterial *importerMaterial = importerScene->mMaterials[importerMesh->mMaterialIndex];
			childNode->m_material =
				ReadMaterial(directory, glProgram, texture2DsLoaded, importerMaterial, gamma);
		}
		else
		{
			childNode->m_material = search->second;
		}
		
		if (importerMesh->HasBones())
		{
			const auto skinnedMesh = ReadSkinnedMesh(boneMaps, importerMesh);
			if (!skinnedMesh)
				continue;
			addedMeshRenderer = true;
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
		childNode->m_localToParent.m_value = glm::mat4(
			importerNode->mTransformation.a1,
			importerNode->mTransformation.a2,
			importerNode->mTransformation.a3,
			importerNode->mTransformation.a4,
			importerNode->mTransformation.b1,
			importerNode->mTransformation.b2,
			importerNode->mTransformation.b3,
			importerNode->mTransformation.b4,
			importerNode->mTransformation.c1,
			importerNode->mTransformation.c2,
			importerNode->mTransformation.c3,
			importerNode->mTransformation.c4,
			importerNode->mTransformation.d1,
			importerNode->mTransformation.d2,
			importerNode->mTransformation.d3,
			importerNode->mTransformation.d4);
		if (!importerNode->mParent)
			childNode->m_localToParent = Transform();
		childNode->m_parent = modelNode;
		modelNode->m_children.push_back(std::move(childNode));
	}

	for (unsigned i = 0; i < importerNode->mNumChildren; i++)
	{
		auto childNode = std::make_shared<ModelNode>();
		auto childAssimpNode = std::make_shared<AssimpNode>(importerNode->mChildren[i]);
		childAssimpNode->m_parent = assimpNode;
		childAssimpNode->m_name = importerNode->mChildren[i]->mName.C_Str();
		const bool childAdd = ProcessNode(
			directory,
			glProgram,
			childNode,
			loadedMaterials,
			texture2DsLoaded,
			boneMaps,
			importerNode->mChildren[i],
			childAssimpNode,
			importerScene,
			gamma);
		if (childAdd)
		{
			childNode->m_parent = modelNode;
			modelNode->m_children.push_back(std::move(childNode));
		}
		addedMeshRenderer = addedMeshRenderer | childAdd;

		childAssimpNode->m_localToParent.m_value = glm::mat4(
			importerNode->mTransformation.a1,
			importerNode->mTransformation.a2,
			importerNode->mTransformation.a3,
			importerNode->mTransformation.a4,
			importerNode->mTransformation.b1,
			importerNode->mTransformation.b2,
			importerNode->mTransformation.b3,
			importerNode->mTransformation.b4,
			importerNode->mTransformation.c1,
			importerNode->mTransformation.c2,
			importerNode->mTransformation.c3,
			importerNode->mTransformation.c4,
			importerNode->mTransformation.d1,
			importerNode->mTransformation.d2,
			importerNode->mTransformation.d3,
			importerNode->mTransformation.d4);
		if (!importerNode->mParent)
			childAssimpNode->m_localToParent = Transform();
		assimpNode->m_children.push_back(std::move(childAssimpNode));
	}
	return addedMeshRenderer;
}

std::shared_ptr<Mesh> ResourceManager::ReadMesh(aiMesh *importerMesh)
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
	auto mesh = CreateResource<Mesh>();
	mesh->SetVertices(mask, vertices, indices);
	return mesh;
}

std::shared_ptr<SkinnedMesh> ResourceManager::ReadSkinnedMesh(
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
	auto skinnedMesh = std::make_shared<SkinnedMesh>();
#pragma region Read bones
	std::vector<std::vector<std::pair<int, float>>> verticesBoneIdWeights;
	verticesBoneIdWeights.resize(vertices.size());
	for (unsigned i = 0; i < importerMesh->mNumBones; i++)
	{
		aiBone *importerBone = importerMesh->mBones[i];
		auto name = importerBone->mName.C_Str();
		if (const auto search = bonesMap.find(name);
			search == bonesMap.end()) // If we can't find this bone
		{
			std::shared_ptr<Bone> bone = std::make_shared<Bone>();
			bone->m_name = name;
			bone->m_offsetMatrix.m_value = glm::mat4(
				importerBone->mOffsetMatrix.a1,
				importerBone->mOffsetMatrix.a2,
				importerBone->mOffsetMatrix.a3,
				importerBone->mOffsetMatrix.a4,
				importerBone->mOffsetMatrix.b1,
				importerBone->mOffsetMatrix.b2,
				importerBone->mOffsetMatrix.b3,
				importerBone->mOffsetMatrix.b4,
				importerBone->mOffsetMatrix.c1,
				importerBone->mOffsetMatrix.c2,
				importerBone->mOffsetMatrix.c3,
				importerBone->mOffsetMatrix.c4,
				importerBone->mOffsetMatrix.d1,
				importerBone->mOffsetMatrix.d2,
				importerBone->mOffsetMatrix.d3,
				importerBone->mOffsetMatrix.d4);
			bonesMap[name] = bone;
			skinnedMesh->m_bones.push_back(bone);
		}else
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
		for (unsigned j = 0; j < 4; j++)
		{
			if (j < verticesBoneIdWeights[i].size())
			{
				ids[j] = verticesBoneIdWeights[i][j].first;
				weights[j] = verticesBoneIdWeights[i][j].second;
			}
		}
		vertices[i].m_bondId = ids;
		vertices[i].m_weight = weights;
	}
#pragma endregion
	skinnedMesh->SetVertices(mask, vertices, indices);
	return skinnedMesh;
}
#else
void ResourceManager::ProcessNode(
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



std::shared_ptr<Texture2D> ResourceManager::CollectTexture(
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
	auto texture2D = LoadTexture(false, directory + "/" + path, gamma);
	loadedTextures[fileName] = texture2D;
	return texture2D;
}




void UniEngine::ResourceManager::AttachChildren(
	EntityArchetype archetype, std::shared_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName)
{
	Entity entity = EntityManager::CreateEntity(archetype);
	entity.SetName(parentName);
	EntityManager::SetParent(entity, parentEntity);
	entity.SetComponentData(modelNode->m_localToParent);
	GlobalTransform globalTransform;
	globalTransform.m_value =
		parentEntity.GetComponentData<GlobalTransform>().m_value * modelNode->m_localToParent.m_value;
	entity.SetComponentData(globalTransform);

	if (modelNode->m_mesh)
	{
		auto mmc = std::make_unique<MeshRenderer>();
		mmc->m_mesh = modelNode->m_mesh;
		mmc->m_material = modelNode->m_material;
		EntityManager::SetPrivateComponent<MeshRenderer>(entity, std::move(mmc));
	}
	else if (modelNode->m_skinnedMesh)
	{
		auto smmc = std::make_unique<SkinnedMeshRenderer>();
		smmc->m_skinnedMesh = modelNode->m_skinnedMesh;
		smmc->m_material = modelNode->m_material;
		EntityManager::SetPrivateComponent<SkinnedMeshRenderer>(entity, std::move(smmc));
	}

	int index = 0;
	for (auto &i : modelNode->m_children)
	{
		AttachChildren(archetype, i, entity, (parentName + "_" + std::to_string(index)));
		index++;
	}
}

std::string ResourceManager::GetTypeName(size_t id)
{
	auto &resourceManager = GetInstance();
	if (resourceManager.m_resources.find(id) != resourceManager.m_resources.end())
	{
		return resourceManager.m_resources[id].first;
	}
	UNIENGINE_ERROR("Resource type not registered!");
	throw 0;
}

std::string ResourceManager::GetTypeName(const std::shared_ptr<ResourceBehaviour> &resource)
{
	auto &resourceManager = GetInstance();
	const auto id = resource->m_typeId;
	if (resourceManager.m_resources.find(id) != resourceManager.m_resources.end())
	{
		return resourceManager.m_resources[id].first;
	}
	UNIENGINE_ERROR("Resource type not registered!");
	throw 0;
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(
	const bool &addResource, const std::string &path, const float &gamma)
{
	stbi_set_flip_vertically_on_load(true);
	auto retVal = CreateResource<Texture2D>();
	const std::string filename = path;
	retVal->m_path = filename;
	int width, height, nrComponents;
	if (gamma == 0.0f)
	{
		if (path.substr(path.find_last_of(".") + 1) == "hdr")
		{
			stbi_hdr_to_ldr_gamma(2.2f);
			stbi_ldr_to_hdr_gamma(2.2f);
		}
		else
		{
			stbi_hdr_to_ldr_gamma(1.0f);
			stbi_ldr_to_hdr_gamma(1.0f);
		}
	}
	else
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_ldr_to_hdr_gamma(gamma);
	}

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
	if (addResource)
		Push(retVal);
	return retVal;
}

std::shared_ptr<Cubemap> ResourceManager::LoadCubemap(
	const bool &addResource, const std::string &path, const float &gamma)
{
	auto &manager = GetInstance();
	stbi_set_flip_vertically_on_load(true);
	auto texture2D = CreateResource<Texture2D>();
	const std::string filename = path;
	texture2D->m_path = filename;
	int width, height, nrComponents;
	if (gamma == 0.0f)
	{
		if (path.substr(path.find_last_of(".") + 1) == "hdr")
		{
			stbi_hdr_to_ldr_gamma(2.2f);
			stbi_ldr_to_hdr_gamma(2.2f);
		}
		else
		{
			stbi_hdr_to_ldr_gamma(1.0f);
			stbi_ldr_to_hdr_gamma(1.0f);
		}
	}
	else
	{
		stbi_hdr_to_ldr_gamma(gamma);
		stbi_ldr_to_hdr_gamma(gamma);
	}
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
	auto retVal = CreateResource<Cubemap>();
	retVal->m_texture = std::move(envCubemap);
	retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
	if (addResource)
		Push(retVal);
	return retVal;
}

std::shared_ptr<LightProbe> ResourceManager::LoadLightProbe(
	const bool &addResource, const std::string &path, const float &gamma)
{
	auto retVal = CreateResource<LightProbe>();
	retVal->ConstructFromCubemap(LoadCubemap(false, path, gamma));
	retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
	if (addResource)
		Push(retVal);
	return retVal;
}

std::shared_ptr<ReflectionProbe> ResourceManager::LoadReflectionProbe(
	const bool &addResource, const std::string &path, const float &gamma)
{
	auto retVal = CreateResource<ReflectionProbe>();
	retVal->ConstructFromCubemap(LoadCubemap(false, path, gamma));
	retVal->m_name = path.substr(path.find_last_of("/\\") + 1);
	if (addResource)
		Push(retVal);
	return retVal;
}

std::shared_ptr<Cubemap> ResourceManager::LoadCubemap(
	const bool &addResource, const std::vector<std::string> &paths, const float &gamma)
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
	for (int i = 0; i < size; i++)
	{
		if (gamma == 0.0f)
		{
			if (paths[i].substr(paths[i].find_last_of(".") + 1) == "hdr")
			{
				stbi_hdr_to_ldr_gamma(2.2f);
				stbi_ldr_to_hdr_gamma(2.2f);
			}
			else
			{
				stbi_hdr_to_ldr_gamma(1.0f);
				stbi_ldr_to_hdr_gamma(1.0f);
			}
		}
		else
		{
			stbi_hdr_to_ldr_gamma(gamma);
			stbi_ldr_to_hdr_gamma(gamma);
		}
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
	auto retVal = CreateResource<Cubemap>();
	retVal->m_texture = std::move(texture);
	retVal->m_name = paths[0].substr(paths[0].find_last_of("/\\") + 1);
	if (addResource)
		Push(retVal);
	return retVal;
}

std::shared_ptr<Material> ResourceManager::LoadMaterial(
	const bool &addResource, const std::shared_ptr<OpenGLUtils::GLProgram> &program)
{
	auto retVal = CreateResource<Material>();
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
	auto retVal = CreateResource<OpenGLUtils::GLProgram>();
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
	auto retVal = CreateResource<OpenGLUtils::GLProgram>();
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
	auto &resourceManager = GetInstance();
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
#ifdef USE_ASSIMP
				std::string modelFormat = ".obj,.gltf,.glb,.blend,.ply,.fbx";
#else
				std::string modelFormat = ".obj";
#endif
				FileIO::OpenFile("Load Model", modelFormat, [](const std::string &filePath) {
					LoadModel(true, filePath, DefaultResources::GLPrograms::StandardProgram);
					UNIENGINE_LOG("Loaded model from \"" + filePath);
				});

				FileIO::OpenFile("Load Texture", ".png,.jpg,.jpeg,.tga", [](const std::string &filePath) {
					LoadTexture(true, filePath);
					UNIENGINE_LOG("Loaded texture from \"" + filePath);
				});

				FileIO::OpenFile("Load Cubemap", ".png,.jpg,.jpeg,.tga,.hdr", [](const std::string &filePath) {
					LoadCubemap(true, filePath);
					UNIENGINE_LOG("Loaded texture from \"" + filePath);
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
			for (auto &collection : resourceManager.m_resources)
			{
				if (ImGui::CollapsingHeader(collection.second.first.c_str()))
				{
					if (ImGui::BeginDragDropTarget())
					{
						const std::string hash = collection.second.first;
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
						const size_t hashCode = i.second->GetHashCode();
						if (EditorManager::Draggable(collection.first, i.second))
						{
							Remove(collection.first, hashCode);
							break;
						}
					}
				}
			}
		}
		ImGui::EndTabBar();
		ImGui::End();
	}
}
