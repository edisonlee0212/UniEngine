#include <Animation.hpp>
using namespace UniEngine;

void Bone::Animate(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &parentTransform,
    const glm::mat4 &rootTransform,
    std::vector<glm::mat4> &results)
{
    glm::mat4 globalTransform = parentTransform;

    const auto search = m_animations.find(name);
    if (search != m_animations.end())
    {
        const auto translation = m_animations[name].InterpolatePosition(animationTime);
        const auto rotation = m_animations[name].InterpolateRotation(animationTime);
        const auto scale = m_animations[name].InterpolateScaling(animationTime);
        globalTransform *= translation * rotation * scale;
    }

    results[m_index] = globalTransform;
    for (auto &i : m_children)
    {
        i->Animate(name, animationTime, globalTransform, rootTransform, results);
    }
}

void Bone::OnInspect()
{
    if (ImGui::TreeNode((m_name + "##" + std::to_string(m_index)).c_str()))
    {
        ImGui::Text("Controller: ");
        ImGui::SameLine();
        for (auto &i : m_children)
        {
            i->OnInspect();
        }
        ImGui::TreePop();
    }
}
void Bone::Serialize(YAML::Emitter &out) const
{
    out << YAML::Key << "m_name" << YAML::Value << m_name;
    out << YAML::Key << "m_offsetMatrix" << YAML::Value << m_offsetMatrix.m_value;
    out << YAML::Key << "m_index" << YAML::Value << m_index;
    if (!m_animations.empty())
    {
        out << YAML::Key << "m_animations" << YAML::Value << YAML::BeginSeq;
        for (const auto &i : m_animations)
        {
            out << YAML::BeginMap;
            {
                out << YAML::Key << "Name" << YAML::Value << i.first;
                out << YAML::Key << "BoneKeyFrames" << YAML::Value << YAML::BeginMap;
                i.second.Serialize(out);
                out << YAML::EndMap;
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
    if (!m_children.empty())
    {
        out << YAML::Key << "m_children" << YAML::Value << YAML::BeginSeq;
        for (const auto &i : m_children)
        {
            out << YAML::BeginMap;
            i->Serialize(out);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}
void Bone::Deserialize(const YAML::Node &in)
{
    m_name = in["m_name"].as<std::string>();
    m_offsetMatrix.m_value = in["m_offsetMatrix"].as<glm::mat4>();
    m_index = in["m_index"].as<size_t>();
    m_animations.clear();
    m_children.clear();
    auto inAnimations = in["m_animations"];
    if (inAnimations)
    {
        for (const auto &i : inAnimations)
        {
            BoneKeyFrames keyFrames;
            keyFrames.Deserialize(i["BoneKeyFrames"]);
            m_animations.insert({i["Name"].as<std::string>(), std::move(keyFrames)});
        }
    }

    auto inChildren = in["m_children"];
    if (inChildren)
    {
        for (const auto &i : inChildren)
        {
            m_children.push_back(std::make_shared<Bone>());
            m_children.back()->Deserialize(i);
        }
    }
}

int BoneKeyFrames::GetPositionIndex(const float &animationTime)
{
    const int size = m_positions.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_positions[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

int BoneKeyFrames::GetRotationIndex(const float &animationTime)
{
    const int size = m_rotations.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_rotations[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

int BoneKeyFrames::GetScaleIndex(const float &animationTime)
{
    const int size = m_scales.size();
    for (int index = 0; index < size - 1; ++index)
    {
        if (animationTime < m_scales[index + 1].m_timeStamp)
            return index;
    }
    return size - 2;
}

float BoneKeyFrames::GetScaleFactor(const float &lastTimeStamp, const float &nextTimeStamp, const float &animationTime)
{
    const float midWayLength = animationTime - lastTimeStamp;
    const float framesDiff = nextTimeStamp - lastTimeStamp;
    if (framesDiff == 0.0f)
        return 0.0f;
    return glm::clamp(midWayLength / framesDiff, 0.0f, 1.0f);
}

glm::mat4 BoneKeyFrames::InterpolatePosition(const float &animationTime)
{
    if (1 == m_positions.size())
        return glm::translate(glm::mat4(1.0f), m_positions[0].m_value);

    const int p0Index = GetPositionIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_positions[p0Index].m_timeStamp, m_positions[p1Index].m_timeStamp, animationTime);
    const glm::vec3 finalPosition = glm::mix(m_positions[p0Index].m_value, m_positions[p1Index].m_value, scaleFactor);
    return glm::translate(finalPosition);
}

glm::mat4 BoneKeyFrames::InterpolateRotation(const float &animationTime)
{
    if (1 == m_rotations.size())
    {
        const auto rotation = glm::normalize(m_rotations[0].m_value);
        return glm::mat4_cast(rotation);
    }

    const int p0Index = GetRotationIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_rotations[p0Index].m_timeStamp, m_rotations[p1Index].m_timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].m_value, m_rotations[p1Index].m_value, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::mat4_cast(finalRotation);
}

glm::mat4 BoneKeyFrames::InterpolateScaling(const float &animationTime)
{
    if (1 == m_scales.size())
        return glm::scale(m_scales[0].m_value);

    const int p0Index = GetScaleIndex(animationTime);
    const int p1Index = p0Index + 1;
    const float scaleFactor =
        GetScaleFactor(m_scales[p0Index].m_timeStamp, m_scales[p1Index].m_timeStamp, animationTime);
    const glm::vec3 finalScale = glm::mix(m_scales[p0Index].m_value, m_scales[p1Index].m_value, scaleFactor);
    return glm::scale(finalScale);
}
void BoneKeyFrames::Serialize(YAML::Emitter &out) const
{
    out << YAML::Key << "m_maxTimeStamp" << YAML::Value << m_maxTimeStamp;
    if (!m_positions.empty())
    {
        out << YAML::Key << "m_positions" << YAML::Value
            << YAML::Binary((const unsigned char *)m_positions.data(), m_positions.size() * sizeof(BonePosition));
    }
    if (!m_rotations.empty())
    {
        out << YAML::Key << "m_rotations" << YAML::Value
            << YAML::Binary((const unsigned char *)m_rotations.data(), m_rotations.size() * sizeof(BoneRotation));
    }
    if (!m_scales.empty())
    {
        out << YAML::Key << "m_scales" << YAML::Value
            << YAML::Binary((const unsigned char *)m_scales.data(), m_scales.size() * sizeof(BoneScale));
    }
}
void BoneKeyFrames::Deserialize(const YAML::Node &in)
{
    m_maxTimeStamp = in["m_maxTimeStamp"].as<float>();
    if (in["m_positions"])
    {
        YAML::Binary positions = in["m_positions"].as<YAML::Binary>();
        m_positions.resize(positions.size() / sizeof(BonePosition));
        std::memcpy(m_positions.data(), positions.data(), positions.size());
    }
    if (in["m_rotations"])
    {
        YAML::Binary rotations = in["m_rotations"].as<YAML::Binary>();
        m_rotations.resize(rotations.size() / sizeof(BoneRotation));
        std::memcpy(m_rotations.data(), rotations.data(), rotations.size());
    }
    if (in["m_scales"])
    {
        YAML::Binary scales = in["m_scales"].as<YAML::Binary>();
        m_scales.resize(scales.size() / sizeof(BoneScale));
        std::memcpy(m_scales.data(), scales.data(), scales.size());
    }
}

std::shared_ptr<Bone> &Animation::UnsafeGetRootBone()
{
    return m_rootBone;
}

void Animation::OnInspect()
{
    if (!m_rootBone)
        return;
    ImGui::Text(("Bone size: " + std::to_string(m_boneSize)).c_str());
    m_rootBone->OnInspect();
}

void Animation::Animate(
    const std::string &name,
    const float &animationTime,
    const glm::mat4 &rootTransform,
    std::vector<glm::mat4> &results)
{
    if (m_animationNameAndLength.find(name) == m_animationNameAndLength.end() || !m_rootBone)
    {
        return;
    }
    m_rootBone->Animate(name, animationTime, rootTransform, rootTransform, results);
}
void Animation::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_boneSize" << YAML::Value << m_boneSize;

    if (!m_animationNameAndLength.empty())
    {
        out << YAML::Key << "m_animationNameAndLength" << YAML::Value << YAML::BeginSeq;
        for (const auto &i : m_animationNameAndLength)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << i.first;
            out << YAML::Key << "Length" << YAML::Value << i.second;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
    if (m_rootBone)
    {
        out << YAML::Key << "m_rootBone" << YAML::Value << YAML::BeginMap;
        m_rootBone->Serialize(out);
        out << YAML::EndMap;
    }
}
void Animation::Deserialize(const YAML::Node &in)
{
    m_boneSize = in["m_boneSize"].as<size_t>();
    auto inAnimationNameAndLength = in["m_animationNameAndLength"];
    m_animationNameAndLength.clear();
    if (inAnimationNameAndLength)
    {
        for (const auto &i : inAnimationNameAndLength)
        {
            m_animationNameAndLength.insert({i["Name"].as<std::string>(), i["Length"].as<float>()});
        }
    }
    if (in["m_rootBone"])
    {
        m_rootBone = std::make_shared<Bone>();
        m_rootBone->Deserialize(in["m_rootBone"]);
    }
}
