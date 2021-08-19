#pragma once
#include <Animator.hpp>
#include <IAsset.hpp>
#include <IPrivateComponent.hpp>
#include <ISystem.hpp>
#include <Material.hpp>
#include <Mesh.hpp>
#include <SkinnedMesh.hpp>
#include <Transform.hpp>

namespace UniEngine
{
#ifdef USE_ASSIMP
struct UNIENGINE_API AssimpNode
{
    aiNode *m_correspondingNode = nullptr;
    std::string m_name;
    Transform m_localTransform;
    AssimpNode(aiNode *node);
    std::shared_ptr<AssimpNode> m_parent;
    std::vector<std::shared_ptr<AssimpNode>> m_children;
    std::shared_ptr<Bone> m_bone;
    bool m_hasMesh;

    bool NecessaryWalker(std::map<std::string, std::shared_ptr<Bone>> &boneMap);
    void AttachToAnimator(std::shared_ptr<Animation> &animation, size_t &index);
    void AttachChild(std::shared_ptr<Bone> &parent, size_t &index);
};
#endif
struct UNIENGINE_API DataComponentHolder
{
    DataComponentType m_type;
    std::shared_ptr<IDataComponent> m_data;

    void Serialize(YAML::Emitter &out);
    void Deserialize(const YAML::Node &in);
};

struct UNIENGINE_API PrivateComponentHolder
{
    bool m_enabled;
    std::shared_ptr<IPrivateComponent> m_data;

    void Serialize(YAML::Emitter &out);
    void Deserialize(const YAML::Node &in);
};

class UNIENGINE_API Prefab : public IAsset
{
    bool m_enabled = true;

#pragma region Model Loading
    void AttachAnimator(Prefab *parent, const Handle &animatorEntityHandle);

    std::shared_ptr<Texture2D> CollectTexture(
        const std::string &directory,
        const std::string &path,
        std::map<std::string, std::shared_ptr<Texture2D>> &loadedTextures);
#ifdef USE_ASSIMP
    void ApplyBoneIndices(Prefab *node);
    void ReadAnimations(
        const aiScene *importerScene,
        std::shared_ptr<Animation> &animator,
        std::map<std::string, std::shared_ptr<Bone>> &bonesMap);
    void ReadKeyFrame(BoneKeyFrames &boneAnimation, const aiNodeAnim *channel);
    std::shared_ptr<Material> ReadMaterial(
        const std::string &directory,
        const std::shared_ptr<OpenGLUtils::GLProgram> &glProgram,
        std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
        aiMaterial *importerMaterial);
    bool ProcessNode(
        const std::string &directory,
        Prefab *modelNode,
        std::map<unsigned, std::shared_ptr<Material>> &loadedMaterials,
        std::map<std::string, std::shared_ptr<Texture2D>> &texture2DsLoaded,
        std::map<std::string, std::shared_ptr<Bone>> &bonesMap,
        aiNode *importerNode,
        std::shared_ptr<AssimpNode> assimpNode,
        const aiScene *importerScene,
        const std::shared_ptr<Animation> &animation);
    std::shared_ptr<Mesh> ReadMesh(aiMesh *importerMesh);
    std::shared_ptr<SkinnedMesh> ReadSkinnedMesh(
        std::map<std::string, std::shared_ptr<Bone>> &bonesMap, aiMesh *importerMesh);
#else
    static void ProcessNode(
        const std::string &directory,
        std::map<int, std::vector<Vertex>> &meshMaterials,
        const tinyobj::shape_t &shape,
        const tinyobj::attrib_t &attribute);
#endif
    void AttachChildren(
        const std::shared_ptr<Prefab> &modelNode,
        Entity parentEntity,
        const std::string &parentName,
        std::unordered_map<Handle, Handle> &map) const;

    void AttachChildrenPrivateComponent(
        const std::shared_ptr<Prefab> &modelNode,
        const Entity &parentEntity,
        const std::unordered_map<Handle, Handle> &map) const;
    void RelinkChildren(const Entity &parentEntity, const std::unordered_map<Handle, Handle> &map) const;
#pragma endregion
  public:
    Handle m_entityHandle = Handle();
    std::vector<DataComponentHolder> m_dataComponents;
    std::vector<PrivateComponentHolder> m_privateComponents;
    std::vector<std::shared_ptr<Prefab>> m_children;
    template <typename T = IPrivateComponent> std::shared_ptr<T> GetPrivateComponent();
    void OnCreate() override;
    void Load(const std::filesystem::path &path) override;
    void Save(const std::filesystem::path &path) override;
    [[nodiscard]] Entity ToEntity() const;

    void FromEntity(const Entity &entity);
    void CollectAssets(std::unordered_map<Handle, std::shared_ptr<IAsset>> &map);
    void Serialize(YAML::Emitter &out);
    void Deserialize(const YAML::Node &in);
};

template <typename T> std::shared_ptr<T> Prefab::GetPrivateComponent()
{
    auto typeName = SerializationManager::GetSerializableTypeName<T>();
    for (auto &i : m_privateComponents)
    {
        if (i.m_data->GetTypeName() == typeName)
        {
            return std::static_pointer_cast<T>(i.m_data);
        }
    }
    return nullptr;
}
} // namespace UniEngine