#include "DefaultResources.hpp"
#include "Editor.hpp"
#include <Material.hpp>
using namespace UniEngine;
static const char *MatPolygonMode[]{"Fill", "Line", "Point"};
static const char *MatCullingMode[]{"BACK", "FRONT", "OFF"};
static const char *MatBlendingMode[]{"OFF", "ONE_MINUS_SRC_ALPHA"};

MaterialFloatProperty::MaterialFloatProperty(const std::string &name, const float &value)
{
    m_name = name;
    m_value = value;
}

MaterialMat4Property::MaterialMat4Property(const std::string &name, const glm::mat4 &value)
{
    m_name = name;
    m_value = value;
}

void Material::OnCreate()
{
    m_name = "New material";
    m_program = DefaultResources::GLPrograms::StandardProgram;
}

void Material::OnInspect()
{
    ImGui::Text("Name: %s", m_name.c_str());
    if (ImGui::BeginPopupContextItem(m_name.c_str()))
    {
        if (ImGui::BeginMenu("Rename##Material"))
        {
            static char newName[256];
            ImGui::InputText("New name##Material", newName, 256);
            if (ImGui::Button("Confirm##Material"))
            {
                m_saved = false;
                m_name = std::string(newName);
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    if (Editor::DragAndDropButton<OpenGLUtils::GLProgram>(m_program, "Program"))
        m_saved = false;
    ImGui::Separator();
    if (ImGui::TreeNodeEx("PBR##Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!m_albedoTexture.Get())
        {
            if (ImGui::ColorEdit3("Albedo##Material", &m_albedoColor.x))
            {
                m_saved = false;
            }
            if(m_blendingMode != MaterialBlendingMode::Off && ImGui::DragFloat("Transparency", &m_transparency, 0.01f, 0.0f, 1.0f)){
                m_saved = false;
            }
        }
        if (!m_metallicTexture.Get())
            if (ImGui::DragFloat("Metallic##Material", &m_metallic, 0.01f, 0.0f, 1.0f))
            {
                m_saved = false;
            }
        if (!m_roughnessTexture.Get())
            if (ImGui::DragFloat("Roughness##Material", &m_roughness, 0.01f, 0.0f, 1.0f))
            {
                m_saved = false;
            }
        if (!m_aoTexture.Get())
            if (ImGui::DragFloat("AO##Material", &m_ambient, 0.01f, 0.0f, 1.0f))
            {
                m_saved = false;
            }
        if (ImGui::DragFloat("Emission##Material", &m_emission, 0.01f, 0.0f, 10.0f))
        {
            m_saved = false;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Others##Material"))
    {
        if (ImGui::Combo(
                "Polygon Mode##Material",
                reinterpret_cast<int *>(&m_polygonMode),
                MatPolygonMode,
                IM_ARRAYSIZE(MatPolygonMode)))
        {
            m_saved = false;
        }
        if (ImGui::Combo(
                "Culling Mode##Material",
                reinterpret_cast<int *>(&m_cullingMode),
                MatCullingMode,
                IM_ARRAYSIZE(MatCullingMode)))
        {
            m_saved = false;
        }
        if (ImGui::Combo(
                "Blending Mode##Material",
                reinterpret_cast<int *>(&m_blendingMode),
                MatBlendingMode,
                IM_ARRAYSIZE(MatBlendingMode)))
        {
            m_saved = false;
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(("Textures##Material" + std::to_string(std::hash<std::string>{}(m_name))).c_str()))
    {
        if (Editor::DragAndDropButton<Texture2D>(m_albedoTexture, "Albedo Tex"))
        {
            m_saved = false;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_normalTexture, "Normal Tex"))
        {
            m_saved = false;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_metallicTexture, "Metallic Tex"))
        {
            m_saved = false;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_roughnessTexture, "Roughness Tex"))
        {
            m_saved = false;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_aoTexture, "AO Tex"))
        {
            m_saved = false;
        }
        ImGui::TreePop();
    }
}

void UniEngine::Material::SetMaterialProperty(const std::string &name, const float &value)
{
    for (auto &property : m_floatPropertyList)
    {
        if (property.m_name.compare(name) == 0)
        {
            property.m_value = value;
            return;
        }
    }
    m_floatPropertyList.emplace_back(name, value);
}

void UniEngine::Material::SetMaterialProperty(const std::string &name, const glm::mat4 &value)
{
    for (auto &property : m_float4X4PropertyList)
    {
        if (property.m_name.compare(name) == 0)
        {
            property.m_value = value;
            return;
        }
    }
    m_float4X4PropertyList.emplace_back(name, value);
}

void Material::SetTexture(const TextureType &type, std::shared_ptr<Texture2D> texture)
{
    switch (type)
    {
    case TextureType::Albedo:
        m_albedoTexture = texture;
        break;
    case TextureType::Normal:
        m_normalTexture = texture;
        break;
    case TextureType::Metallic:
        m_metallicTexture = texture;
        break;
    case TextureType::Roughness:
        m_roughnessTexture = texture;
        break;
    case TextureType::AO:
        m_aoTexture = texture;
        break;
    }
}

void Material::RemoveTexture(TextureType type)
{
    switch (type)
    {
    case TextureType::Albedo:
        m_albedoTexture.Clear();
        break;
    case TextureType::Normal:
        m_normalTexture.Clear();
        break;
    case TextureType::Metallic:
        m_metallicTexture.Clear();
        break;
    case TextureType::Roughness:
        m_roughnessTexture.Clear();
        break;
    case TextureType::AO:
        m_aoTexture.Clear();
        break;
    }
}

void Material::SetProgram(std::shared_ptr<OpenGLUtils::GLProgram> program)
{
    m_program = std::move(program);
}
void Material::Serialize(YAML::Emitter &out)
{
    m_albedoTexture.Save("m_albedoTexture", out);
    m_normalTexture.Save("m_normalTexture", out);
    m_metallicTexture.Save("m_metallicTexture", out);
    m_roughnessTexture.Save("m_roughnessTexture", out);
    m_aoTexture.Save("m_aoTexture", out);
    m_program.Save("m_program", out);

    out << YAML::Key << "m_polygonMode" << YAML::Value << (unsigned)m_polygonMode;
    out << YAML::Key << "m_cullingMode" << YAML::Value << (unsigned)m_cullingMode;
    out << YAML::Key << "m_blendingMode" << YAML::Value << (unsigned)m_blendingMode;

    out << YAML::Key << "m_metallic" << YAML::Value << m_metallic;
    out << YAML::Key << "m_roughness" << YAML::Value << m_roughness;
    out << YAML::Key << "m_ambient" << YAML::Value << m_ambient;
    out << YAML::Key << "m_emission" << YAML::Value << m_emission;
    out << YAML::Key << "m_albedoColor" << YAML::Value << m_albedoColor;
    out << YAML::Key << "m_transparency" << YAML::Value << m_transparency;
    out << YAML::Key << "m_subsurfaceColor" << YAML::Value << m_subsurfaceColor;
    out << YAML::Key << "m_subsurfaceRadius" << YAML::Value << m_subsurfaceRadius;
}
void Material::Deserialize(const YAML::Node &in)
{
    m_albedoTexture.Load("m_albedoTexture", in);
    m_normalTexture.Load("m_normalTexture", in);
    m_metallicTexture.Load("m_metallicTexture", in);
    m_roughnessTexture.Load("m_roughnessTexture", in);
    m_aoTexture.Load("m_aoTexture", in);
    m_program.Load("m_program", in);

    if (in["m_polygonMode"])
        m_polygonMode = (MaterialPolygonMode)in["m_polygonMode"].as<unsigned>();
    if (in["m_cullingMode"])
        m_cullingMode = (MaterialCullingMode)in["m_cullingMode"].as<unsigned>();
    if (in["m_blendingMode"])
        m_blendingMode = (MaterialBlendingMode)in["m_blendingMode"].as<unsigned>();

    if (in["m_metallic"])
        m_metallic = in["m_metallic"].as<float>();
    if (in["m_transparency"])
        m_transparency = in["m_transparency"].as<float>();
    if (in["m_roughness"])
        m_roughness = in["m_roughness"].as<float>();
    if (in["m_ambient"])
        m_ambient = in["m_ambient"].as<float>();
    if (in["m_emission"])
        m_emission = in["m_emission"].as<float>();
    if (in["m_albedoColor"])
        m_albedoColor = in["m_albedoColor"].as<glm::vec3>();
    if (in["m_subsurfaceColor"])
        m_subsurfaceColor = in["m_subsurfaceColor"].as<glm::vec3>();
    if (in["m_subsurfaceRadius"])
        m_subsurfaceRadius = in["m_subsurfaceRadius"].as<float>();
}
void Material::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_albedoTexture);
    list.push_back(m_normalTexture);
    list.push_back(m_metallicTexture);
    list.push_back(m_roughnessTexture);
    list.push_back(m_aoTexture);
    list.push_back(m_program);
}
