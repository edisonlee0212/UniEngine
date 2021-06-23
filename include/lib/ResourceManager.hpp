#pragma once
#include <Cubemap.hpp>
#include <EntityManager.hpp>
#include <Model.hpp>
#include <OpenGLUtils.hpp>
namespace UniEngine
{
class UNIENGINE_API ResourceManager : public ISingleton<ResourceManager>
{
    bool m_enableAssetMenu = true;
    std::map<size_t, std::pair<std::string, std::map<size_t, std::shared_ptr<ResourceBehaviour>>>> m_resources;
    std::unique_ptr<OpenGLUtils::GLProgram> m_2DToCubemapProgram;
    friend class DefaultResources;
    static std::shared_ptr<Texture2D> CollectTexture(
        const std::string &directory,
        const std::string &path,
        std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures,
        const TextureType &textureType
    );

    static void ProcessNode(
        const std::string &directory,
        std::map<int, std::vector<Vertex>> &meshMaterials,
        const tinyobj::shape_t& shape,
        const tinyobj::attrib_t& attribute);
    static void AttachChildren(
        EntityArchetype archetype, std::unique_ptr<ModelNode> &modelNode, Entity parentEntity, std::string parentName);

  public:
    template <typename T> static void Push(std::shared_ptr<T> resource);
    template <typename T> static std::shared_ptr<T> Get(size_t hashCode);
    template <typename T> static std::shared_ptr<T> Find(std::string objectName);
    template <typename T> static void Remove(size_t hashCode);
    static void Remove(size_t id, size_t hashCode);
    static std::shared_ptr<Model> LoadModel(const bool &addResource,
                                            std::string const &path,
                                            std::shared_ptr<OpenGLUtils::GLProgram> glProgram,
                                            const bool& optimize = false
        );
    static std::shared_ptr<Texture2D> LoadTexture(
        const bool &addResource,
        const std::string &path,
        TextureType type = TextureType::Albedo,
        const float &gamma = 1.0f);
    static std::shared_ptr<Cubemap> LoadCubemap(
        const bool &addResource,
        const std::string &path,
        const float &gamma = 1.0f);
    static std::shared_ptr<Cubemap> LoadCubemap(
        const bool &addResource,
        const std::vector<std::string> &paths,
        const float &gamma = 1.0f);
    static std::shared_ptr<Material> LoadMaterial(
        const bool &addResource, const std::shared_ptr<OpenGLUtils::GLProgram> &program);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
        const bool &addResource,
        const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
        const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
    static std::shared_ptr<OpenGLUtils::GLProgram> LoadProgram(
        const bool &addResource,
        const std::shared_ptr<OpenGLUtils::GLShader> &vertex,
        const std::shared_ptr<OpenGLUtils::GLShader> &geometry,
        const std::shared_ptr<OpenGLUtils::GLShader> &fragment);
    static void OnGui();
    static Entity ToEntity(EntityArchetype archetype, std::shared_ptr<Model> model);
};

template <typename T> void ResourceManager::Push(std::shared_ptr<T> resource)
{
    GetInstance().m_resources[typeid(T).hash_code()].first = std::string(typeid(T).name());
    GetInstance()
        .m_resources[typeid(T).hash_code()]
        .second[std::dynamic_pointer_cast<ResourceBehaviour>(resource)->GetHashCode()] = resource;
}

template <typename T> std::shared_ptr<T> ResourceManager::Get(size_t hashCode)
{
    return std::dynamic_pointer_cast<ResourceBehaviour>(
        GetInstance().m_resources[typeid(T).hash_code()].second[hashCode]);
}

template <typename T> std::shared_ptr<T> ResourceManager::Find(std::string objectName)
{
    for (const auto &i : GetInstance().m_resources[typeid(T).hash_code()].second)
    {
        if (i.second->m_name.compare(objectName) == 0)
            return std::dynamic_pointer_cast<ResourceBehaviour>(i.second);
    }
    return nullptr;
}

template <typename T> void ResourceManager::Remove(size_t hashCode)
{
    GetInstance().m_resources[typeid(T).hash_code()].second.erase(hashCode);
}
} // namespace UniEngine
