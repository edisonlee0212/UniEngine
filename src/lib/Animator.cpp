#include "Application.hpp"
#include "RenderManager.hpp"
#include <Animator.hpp>
#include <EditorManager.hpp>
using namespace UniEngine;

void Animator::Setup()
{
    auto animation = m_animation.Get<Animation>();
    if (animation)
    {
        m_boneSize = animation->m_boneSize;
        if (animation->m_rootBone && m_boneSize != 0)
        {
            m_transformChain.resize(m_boneSize);
            m_boundEntities.resize(m_boneSize);
            m_names.resize(m_boneSize);
            m_bones.resize(m_boneSize);
            BoneSetter(animation->m_rootBone);
            m_offsetMatrices.resize(m_boneSize);
            for (auto &i : m_bones)
                m_offsetMatrices[i->m_index] = i->m_offsetMatrix.m_value;
            if (!animation->m_animationNameAndLength.empty())
                m_currentActivatedAnimation = animation->m_animationNameAndLength.begin()->first;
        }
    }
    m_needAnimationSetup = false;
}

void Animator::Setup(const std::shared_ptr<Animation> &targetAnimation)
{
    m_animation.Set<Animation>(targetAnimation);
    m_needAnimationSetup = true;
}

void Animator::OnInspect()
{
    ImGui::Text("Animation:");
    ImGui::SameLine();
    auto animation = m_animation.Get<Animation>();
    Animation *previous = animation.get();

    EditorManager::DragAndDropButton<Animation>(m_animation, "Animation");
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
                if (EditorManager::DragAndDropButton(m_boundEntities[i], "Bone: " + m_names[i]))
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
                        m_needAnimate = true;
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
            if(ImGui::SliderFloat(
                "Animation time",
                &m_currentAnimationTime,
                0.0f,
                animation->m_animationNameAndLength[m_currentActivatedAnimation])){
                m_needAnimate = true;
            }
        }
    }
}
void Animator::AutoPlay()
{
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    if (m_needAnimationSetup)
        Setup();
    m_currentAnimationTime += Application::Time().DeltaTime() * 1000.0f;
    if (m_currentAnimationTime > animation->m_animationNameAndLength[m_currentActivatedAnimation])
        m_currentAnimationTime =
            glm::mod(m_currentAnimationTime, animation->m_animationNameAndLength[m_currentActivatedAnimation]);
    m_needAnimate = true;
    Animate();
}

bool Animator::AnimatedCurrentFrame()
{
    return m_animatedCurrentFrame;
}

void Animator::Animate()
{
    if (m_boneSize == 0 && !m_transformChain.empty())
    {
        for (int i = 0; i < m_transformChain.size(); i++)
        {
            auto entity = m_boundEntities[i].Get();
            if(!entity.IsNull())
            {
                m_transformChain[i] = entity.GetDataComponent<GlobalTransform>().m_value;
            }
        }
        ApplyOffsetMatrices();
        m_needAnimate = false;
        m_animatedCurrentFrame = true;
        return;
    }
    if(!m_needAnimate) return;
    auto animation = m_animation.Get<Animation>();
    if (animation)
    {
        if (m_needAnimationSetup)
            Setup();

        if (animation->m_animationNameAndLength.find(m_currentActivatedAnimation) ==
            animation->m_animationNameAndLength.end())
        {
            m_currentActivatedAnimation = animation->m_animationNameAndLength.begin()->first;
            m_currentAnimationTime = 0.0f;
        }
        auto owner = GetOwner();
        if(!owner.IsNull())
        {
            animation->Animate(
                m_currentActivatedAnimation,
                m_currentAnimationTime,
                owner.GetDataComponent<GlobalTransform>().m_value,
                m_boundEntities,
                m_transformChain);
            ApplyOffsetMatrices();
        }
    }
    m_needAnimate = false;
    m_animatedCurrentFrame = true;
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
    m_boundEntities.resize(boundEntities.size());
    for (int i = 0; i < boundEntities.size(); i++)
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
    m_needAnimate = true;
}

void Animator::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_animation);
}

void Animator::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_autoPlay" << YAML::Value << m_autoPlay;

    if (m_animation.Get<Animation>())
    {
        m_animation.Save("m_animation", out);
        out << YAML::Key << "m_currentActivatedAnimation" << YAML::Value << m_currentActivatedAnimation;
        out << YAML::Key << "m_currentAnimationTime" << YAML::Value << m_currentAnimationTime;
    }
    if (!m_boundEntities.empty())
    {
        out << YAML::Key << "m_boundEntities" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < m_boundEntities.size(); i++)
        {
            out << YAML::BeginMap;
            m_boundEntities[i].Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        if (!m_transformChain.empty())
        {
            out << YAML::Key << "m_transformChain" << YAML::Value
                << YAML::Binary(
                       (const unsigned char *)m_transformChain.data(), m_transformChain.size() * sizeof(glm::mat4));
        }
        if (!m_offsetMatrices.empty())
        {
            out << YAML::Key << "m_offsetMatrices" << YAML::Value
                << YAML::Binary(
                       (const unsigned char *)m_offsetMatrices.data(), m_offsetMatrices.size() * sizeof(glm::mat4));
        }
        if (!m_names.empty())
        {
            out << YAML::Key << "m_names" << YAML::Value << YAML::BeginSeq;
            for (int i = 0; i < m_names.size(); i++)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << m_names[i];
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }
    }
}

void Animator::Deserialize(const YAML::Node &in)
{
    m_autoPlay = in["m_autoPlay"].as<bool>();
    m_animation.Load("m_animation", in);
    if (m_animation.Get<Animation>())
    {
        m_needAnimationSetup = true;
        m_currentActivatedAnimation = in["m_currentActivatedAnimation"].as<std::string>();
        m_currentAnimationTime = in["m_currentAnimationTime"].as<float>();
        Setup();
    }
    auto inBoundEntities = in["m_boundEntities"];
    if (inBoundEntities)
    {
        for (const auto &i : inBoundEntities)
        {
            EntityRef ref;
            ref.Deserialize(i);
            m_boundEntities.push_back(ref);
        }
        if (in["m_transformChain"])
        {
            YAML::Binary chains = in["m_transformChain"].as<YAML::Binary>();
            m_transformChain.resize(chains.size() / sizeof(glm::mat4));
            std::memcpy(m_transformChain.data(), chains.data(), chains.size());
        }
        if (in["m_offsetMatrices"])
        {
            YAML::Binary matrices = in["m_offsetMatrices"].as<YAML::Binary>();
            m_offsetMatrices.resize(matrices.size() / sizeof(glm::mat4));
            std::memcpy(m_offsetMatrices.data(), matrices.data(), matrices.size());
        }
        if (in["m_names"])
        {
            for (const auto &i : in["m_names"])
            {
                m_names.push_back(i["Name"].as<std::string>());
            }
        }
    }
    m_needAnimate = true;
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