#include <EditorManager.hpp>
#include <Gui.hpp>
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
}

void Material::OnGui()
{
    ImGui::Text("Name: %s", m_name.c_str());
    if (ImGui::BeginPopupContextItem(m_name.c_str()))
    {
        if (ImGui::BeginMenu("Rename##Material"))
        {
            static char newName[256];
            ImGui::InputText("New name##Material", newName, 256);
            if (ImGui::Button("Confirm##Material"))
                m_name = std::string(newName);
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    EditorManager::DragAndDrop<OpenGLUtils::GLProgram>(m_program, "Program");
    ImGui::Separator();
    if (ImGui::TreeNodeEx("PBR##Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!m_albedoTexture.Get())
            ImGui::ColorEdit3("Albedo##Material", &m_albedoColor.x);
        if (!m_metallicTexture.Get())
            ImGui::DragFloat("Metallic##Material", &m_metallic, 0.01f, 0.0f, 1.0f);
        if (!m_roughnessTexture.Get())
            ImGui::DragFloat("Roughness##Material", &m_roughness, 0.01f, 0.0f, 1.0f);
        if (!m_aoTexture.Get())
            ImGui::DragFloat("AO##Material", &m_ambient, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Emission##Material", &m_emission, 0.01f, 0.0f, 10.0f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Others##Material"))
    {
        ImGui::Checkbox("Enable alpha discard##Material", &m_alphaDiscardEnabled);
        if (m_alphaDiscardEnabled)
            ImGui::DragFloat("Alpha discard offset##Material", &m_alphaDiscardOffset, 0.01f, 0.0f, 0.99f);
        ImGui::Combo(
            "Polygon Mode##Material",
            reinterpret_cast<int *>(&m_polygonMode),
            MatPolygonMode,
            IM_ARRAYSIZE(MatPolygonMode));
        ImGui::Combo(
            "Culling Mode##Material",
            reinterpret_cast<int *>(&m_cullingMode),
            MatCullingMode,
            IM_ARRAYSIZE(MatCullingMode));
        ImGui::Combo(
            "Blending Mode##Material",
            reinterpret_cast<int *>(&m_blendingMode),
            MatBlendingMode,
            IM_ARRAYSIZE(MatBlendingMode));
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(("Textures##Material" + std::to_string(std::hash<std::string>{}(m_name))).c_str()))
    {
        EditorManager::DragAndDrop<Texture2D>(m_albedoTexture, "Albedo Tex");
        EditorManager::DragAndDrop<Texture2D>(m_normalTexture, "Normal Tex");
        EditorManager::DragAndDrop<Texture2D>(m_metallicTexture, "Metallic Tex");
        EditorManager::DragAndDrop<Texture2D>(m_roughnessTexture, "Roughness Tex");
        EditorManager::DragAndDrop<Texture2D>(m_aoTexture, "AO Tex");
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

    out << YAML::Key << "m_metallic" << YAML::Value << m_metallic;
    out << YAML::Key << "m_roughness" << YAML::Value << m_roughness;
    out << YAML::Key << "m_ambient" << YAML::Value << m_ambient;
    out << YAML::Key << "m_emission" << YAML::Value << m_emission;
    out << YAML::Key << "m_albedoColor" << YAML::Value << m_albedoColor;
    out << YAML::Key << "m_alphaDiscardEnabled" << YAML::Value << m_alphaDiscardEnabled;
    out << YAML::Key << "m_alphaDiscardOffset" << YAML::Value << m_alphaDiscardOffset;
}
void Material::Deserialize(const YAML::Node &in)
{
    m_albedoTexture.Load("m_albedoTexture", in);
    m_normalTexture.Load("m_normalTexture", in);
    m_metallicTexture.Load("m_metallicTexture", in);
    m_roughnessTexture.Load("m_roughnessTexture", in);
    m_aoTexture.Load("m_aoTexture", in);
    m_program.Load("m_program", in);

    m_metallic = in["m_metallic"].as<float>();
    m_roughness = in["m_roughness"].as<float>();
    m_ambient = in["m_ambient"].as<float>();
    m_emission = in["m_emission"].as<float>();
    m_albedoColor = in["m_albedoColor"].as<glm::vec3>();
    m_alphaDiscardEnabled = in["m_alphaDiscardEnabled"].as<bool>();
    m_alphaDiscardOffset = in["m_alphaDiscardOffset"].as<float>();
}
