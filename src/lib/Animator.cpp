#include "Application.hpp"
#include "RenderManager.hpp"
#include <Animator.hpp>
#include <EditorManager.hpp>
using namespace UniEngine;

void Animator::Setup()
{
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    m_boneSize = animation->m_boneSize;
    m_transformChain.resize(m_boneSize);
    m_boundEntities.resize(m_boneSize);
    m_names.resize(m_boneSize);
    m_bones.resize(m_boneSize);
    if (animation->m_rootBone)
        BoneSetter(animation->m_rootBone);
    m_offsetMatrices.resize(m_boneSize);
    for (auto &i : m_bones)
        m_offsetMatrices[i->m_index] = i->m_offsetMatrix.m_value;
    if (!animation->m_animationNameAndLength.empty())
        m_currentActivatedAnimation = animation->m_animationNameAndLength.begin()->first;

    m_needAnimationSetup = false;
}

void Animator::Setup(const std::shared_ptr<Animation> &targetAnimation)
{
    m_animation.Set<Animation>(targetAnimation);
    m_needAnimationSetup = true;
}

void Animator::OnGui()
{
    ImGui::Text("Animation:");
    ImGui::SameLine();
    auto animation = m_animation.Get<Animation>();
    Animation *previous = animation.get();

    EditorManager::DragAndDrop<Animation>(m_animation, "Animation");
    if (previous != animation.get() && animation)
    {
        Setup(animation);
        animation = m_animation.Get<Animation>();
    }

    if (animation)
    {
        static bool debugRenderBones = true;
        static float debugRenderBonesSize = 0.5f;
        static glm::vec4 debugRenderBonesColor = glm::vec4(1, 1, 0, 0.5);
        ImGui::Checkbox("Display bones", &debugRenderBones);
        if (debugRenderBones)
        {
            ImGui::DragFloat("Size", &debugRenderBonesSize, 0.01f, 0.01f, 3.0f);
            ImGui::ColorEdit4("Color", &debugRenderBonesColor.x);
            DebugBoneRender(debugRenderBonesColor, debugRenderBonesSize);
        }
        animation->OnGui();
        if (ImGui::TreeNode("RagDoll"))
        {
            for (int i = 0; i < m_boundEntities.size(); i++)
            {
                if (EditorManager::DragAndDrop(m_boundEntities[i], "Bone: " + m_names[i]))
                {
                    if (m_boundEntities[i].Get().IsValid())
                    {
                        ResetTransform(i);
                    }
                }
            }
            ImGui::TreePop();
        }
        if (m_boneSize != 0)
        {
            if (animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
                animation->m_animationNameAndLength.end())
            {
                m_currentActivatedAnimation = animation->m_animationNameAndLength.begin()->first;
                m_currentAnimationTime = 0.0f;
            }
            if (ImGui::BeginCombo(
                    "Animations##Animator",
                    m_currentActivatedAnimation
                        .c_str())) // The second parameter is the label previewed before opening the combo.
            {
                for (auto &i : animation->m_animationNameAndLength)
                {
                    const bool selected =
                        m_currentActivatedAnimation ==
                        i.first; // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(i.first.c_str(), selected))
                    {
                        m_currentActivatedAnimation = i.first;
                        m_currentAnimationTime = 0.0f;
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling
                                                      // + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("AutoPlay", &m_autoPlay);
            ImGui::SliderFloat(
                "Animation time",
                &m_currentAnimationTime,
                0.0f,
                animation->m_animationNameAndLength[m_currentActivatedAnimation]);
        }
    }
}
void Animator::AutoPlay()
{
    if (m_needAnimationSetup)
        Setup();
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    m_currentAnimationTime += Application::Time().DeltaTime() * 1000.0f;
    if (m_currentAnimationTime > animation->m_animationNameAndLength[m_currentActivatedAnimation])
        m_currentAnimationTime =
            glm::mod(m_currentAnimationTime, animation->m_animationNameAndLength[m_currentActivatedAnimation]);
}
void Animator::Animate()
{
    if (m_needAnimationSetup)
        Setup();
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    if (m_boneSize == 0)
    {
        for (int i = 0; i < m_transformChain.size(); i++)
        {
            m_transformChain[i] = m_boundEntities[i].Get().GetDataComponent<GlobalTransform>().m_value;
        }
    }
    else
    {
        auto owner = GetOwner();
        if (animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
            animation->m_animationNameAndLength.end())
        {
            m_currentActivatedAnimation = animation->m_animationNameAndLength.begin()->first;
            m_currentAnimationTime = 0.0f;
        }
        animation->Animate(
            m_currentActivatedAnimation,
            m_currentAnimationTime,
            owner.GetDataComponent<GlobalTransform>().m_value,
            m_boundEntities,
            m_transformChain);
    }
    ApplyOffsetMatrices();
}

void Animator::BoneSetter(const std::shared_ptr<Bone> &boneWalker)
{
    m_names[boneWalker->m_index] = boneWalker->m_name;
    m_bones[boneWalker->m_index] = boneWalker;
    for (auto &i : boneWalker->m_children)
    {
        BoneSetter(i);
    }
}

void Animator::Setup(
    std::vector<Entity> &boundEntities, std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices)
{
    m_bones.clear();
    assert(boundEntities.size() == name.size() && boundEntities.size() == offsetMatrices.size());
    m_transformChain.resize(boundEntities.size());
    for (int i = 0; i < m_boneSize; i++)
    {
        m_boundEntities[i] = boundEntities[i];
    }
    m_names = name;
    m_offsetMatrices = offsetMatrices;
}

void Animator::ApplyOffsetMatrices()
{
    for (int i = 0; i < m_transformChain.size(); i++)
    {
        m_transformChain[i] *= m_offsetMatrices[i];
    }
}

void Animator::DebugBoneRender(const glm::vec4 &color, const float &size)
{
    if (m_needAnimationSetup)
        Setup();
    const auto selfScale = GetOwner().GetDataComponent<GlobalTransform>().GetScale();

    std::vector<glm::mat4> debugRenderingMatrices = m_transformChain;
    for (int index = 0; index < m_transformChain.size(); index++)
    {
        debugRenderingMatrices[index] =
            m_transformChain[index] * glm::inverse(m_offsetMatrices[index]) * glm::inverse(glm::scale(selfScale));
    }
    RenderManager::DrawGizmoMeshInstanced(
        DefaultResources::Primitives::Sphere, color, debugRenderingMatrices, Transform().m_value, size);
}

void Animator::ResetTransform(const int &index)
{
    if (m_needAnimationSetup)
        Setup();
    GlobalTransform globalTransform;
    auto entity = m_boundEntities[index].Get();
    globalTransform.m_value = m_transformChain[index] * glm::inverse(m_bones[index]->m_offsetMatrix.m_value);
    auto parent = m_boundEntities[index].Get().GetParent();
    Transform localTransform;
    if (parent.IsValid())
    {
        const auto parentGlobalTransform = parent.GetDataComponent<GlobalTransform>();
        localTransform.m_value = glm::inverse(parentGlobalTransform.m_value) * globalTransform.m_value;
    }
    else
    {
        localTransform.m_value = globalTransform.m_value;
    }
    entity.SetDataComponent(localTransform);
}
void Animator::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    *this = *std::static_pointer_cast<Animator>(target);
}
void Animator::Deserialize(const YAML::Node &in)
{
    m_autoPlay = in["m_autoPlay"].as<bool>();
    m_animation.Load("m_animation", in);
    m_needAnimationSetup = true;
    m_currentActivatedAnimation = in["m_currentActivatedAnimation"].as<std::string>();
    m_currentAnimationTime = in["m_currentAnimationTime"].as<float>();

    auto inBoundEntities = in["m_boundEntities"];
    auto size = inBoundEntities.size();
    for (int i = 0; i < size; i++)
    {
        EntityRef ref;
        ref.Load(std::to_string(i), inBoundEntities);
        m_boundEntities.push_back(ref);
        i++;
    }
}

void Animator::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_animation);
}

void Animator::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_autoPlay" << YAML::Value << m_autoPlay;
    out << YAML::Key << "m_currentActivatedAnimation" << YAML::Value << m_currentActivatedAnimation;
    out << YAML::Key << "m_currentAnimationTime" << YAML::Value << m_currentAnimationTime;
    m_animation.Save("m_animation", out);

    out << YAML::Key << "m_boundEntities" << YAML::Value << YAML::BeginMap;
    int index = 0;
    for (auto &i : m_boundEntities)
    {
        i.Save(std::to_string(index), out);
        index++;
    }
    out << YAML::EndMap;
}
std::shared_ptr<Animation> Animator::GetAnimation()
{
    return m_animation.Get<Animation>();
}
void Animator::Relink(const std::unordered_map<Handle, Handle> &map)
{
    for (auto &i : m_boundEntities)
    {
        i.Relink(map);
    }
}