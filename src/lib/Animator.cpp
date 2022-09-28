#include "Application.hpp"
#include "Graphics.hpp"
#include <Animator.hpp>
#include "Editor.hpp"
#include "ClassRegistry.hpp"
using namespace UniEngine;
PrivateComponentRegistration<Animator> AnimatorRegistry("Animator");

bool Animator::AnimatedCurrentFrame() const
{
    return m_animatedCurrentFrame;
}
void Animator::Setup()
{
    auto animation = m_animation.Get<Animation>();
    if (animation)
    {
        m_boneSize = animation->m_boneSize;
        if (animation->m_rootBone && m_boneSize != 0)
        {
            m_transformChain.resize(m_boneSize);
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
void Animator::OnDestroy()
{
    m_transformChain.clear();
    m_offsetMatrices.clear();
    m_names.clear();
    m_animation.Clear();
    m_bones.clear();
}
void Animator::Setup(const std::shared_ptr<Animation> &targetAnimation)
{
    m_animation.Set<Animation>(targetAnimation);
    m_needAnimationSetup = true;
}

void Animator::OnInspect()
{
    auto animation = m_animation.Get<Animation>();
    Animation *previous = animation.get();
    Editor::DragAndDropButton<Animation>(m_animation, "Animation");
    if (previous != animation.get() && animation)
    {
        Setup(animation);
        animation = m_animation.Get<Animation>();
    }
    if (animation)
    {
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
            if(!Application::IsPlaying()) ImGui::Checkbox("AutoPlay", &m_autoPlay);
            if (ImGui::SliderFloat(
                    "Animation time",
                    &m_currentAnimationTime,
                    0.0f,
                    animation->m_animationNameAndLength[m_currentActivatedAnimation]))
            {
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
}
float Animator::CurrentAnimationTime()
{
    return m_currentAnimationTime;
}

std::string Animator::CurrentAnimationName()
{
    return m_currentActivatedAnimation;
}

void Animator::Animate(const std::string& animationName, float time)
{
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    auto search = animation->m_animationNameAndLength.find(animationName);
    if(search == animation->m_animationNameAndLength.end()){
        UNIENGINE_ERROR("Animation not found!");
        return;
    }
    m_currentActivatedAnimation = animationName;
    m_currentAnimationTime =
        glm::mod(time, animation->m_animationNameAndLength[m_currentActivatedAnimation]);
    m_needAnimate = true;
}
void Animator::SetAutoPlay(bool value)
{
    m_autoPlay = value;
}
void Animator::Animate(float time)
{
    auto animation = m_animation.Get<Animation>();
    if (!animation)
        return;
    m_currentAnimationTime =
        glm::mod(time, animation->m_animationNameAndLength[m_currentActivatedAnimation]);
    m_needAnimate = true;
}
void Animator::Apply()
{
    if (!m_needAnimate)
        return;
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
        if (owner.GetIndex() != 0)
        {
            animation->Animate(m_currentActivatedAnimation, m_currentAnimationTime, glm::mat4(1.0f), m_transformChain);
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

void Animator::Setup(std::vector<std::string> &name, std::vector<glm::mat4> &offsetMatrices)
{
    m_bones.clear();
    m_boneSize = 0;
    m_transformChain.resize(offsetMatrices.size());
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

glm::mat4 Animator::GetReverseTransform(const int &index, const Entity& entity)
{
    if (m_needAnimationSetup)
        Setup();
    return m_transformChain[index] * glm::inverse(m_bones[index]->m_offsetMatrix.m_value);
}
void Animator::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
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
    m_animatedCurrentFrame = false;
    m_needAnimate = true;
}

std::shared_ptr<Animation> Animator::GetAnimation()
{
    return m_animation.Get<Animation>();
}