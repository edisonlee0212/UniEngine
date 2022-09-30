#include "DefaultResources.hpp"
#include "Editor.hpp"
#include <Material.hpp>
using namespace UniEngine;

void Material::OnCreate()
{
    m_program = DefaultResources::GLPrograms::StandardProgram;
}
static const char *PolygonMode[]{"Point", "Line", "Fill"};
static const char *CullingMode[]{"Front", "Back", "FrontAndBack"};
static const char *BlendingFactor[]{"Zero", "One", "SrcColor", "OneMinusSrcColor", "DstColor", "OneMinusDstColor", "SrcAlpha", "OneMinusSrcAlpha",
                                    "DstAlpha", "OneMinusDstAlpha", "ConstantColor", "OneMinusConstantColor", "ConstantAlpha", "OneMinusConstantAlpha", "SrcAlphaSaturate",
                                    "Src1Color", "OneMinusSrc1Color", "Src1Alpha", "OneMinusSrc1Alpha"};
bool DrawSettings::OnInspect()
{
    bool changed = false;
    if (ImGui::Combo(
            "Polygon Mode",
            reinterpret_cast<int *>(&m_polygonMode),
            PolygonMode,
            IM_ARRAYSIZE(PolygonMode)))
    {
        changed = true;
    }
    if(ImGui::Checkbox("Cull Face", &m_cullFace)) changed = true;
    if (m_cullFace && ImGui::Combo(
            "Cull Face Mode",
            reinterpret_cast<int *>(&m_cullFaceMode),
                          CullingMode,
            IM_ARRAYSIZE(CullingMode)))
    {
        changed = true;
    }

    if(ImGui::Checkbox("Blending", &m_blending)) changed = true;

    if(m_blending){
        if (ImGui::Combo(
                "Blending Source Factor",
                reinterpret_cast<int *>(&m_blendingSrcFactor),
                BlendingFactor,
                IM_ARRAYSIZE(BlendingFactor)))
        {
            changed = true;
        }
        if (ImGui::Combo(
                "Blending Destination Factor",
                reinterpret_cast<int *>(&m_blendingDstFactor),
                BlendingFactor,
                IM_ARRAYSIZE(BlendingFactor)))
        {
            changed = true;
        }
    }
    return changed;
}
void DrawSettings::Save(const std::string &name, YAML::Emitter &out)
{
    out << YAML::Key << name << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "m_cullFace" << YAML::Value << m_cullFace;
    out << YAML::Key << "m_cullFaceMode" << YAML::Value << (unsigned)m_cullFaceMode;
    out << YAML::Key << "m_polygonMode" << YAML::Value << (unsigned)m_polygonMode;
    out << YAML::Key << "m_blending" << YAML::Value << m_blending;
    out << YAML::Key << "m_blendingSrcFactor" << YAML::Value << (unsigned)m_blendingSrcFactor;
    out << YAML::Key << "m_blendingDstFactor" << YAML::Value << (unsigned)m_blendingDstFactor;
    out << YAML::EndMap;
}
void DrawSettings::Load(const std::string &name, const YAML::Node &in)
{
    if (in[name])
    {
        const auto& drawSettings = in[name];
        m_cullFace = drawSettings["m_cullFace"].as<bool>();
        m_cullFaceMode = (OpenGLCullFace)drawSettings["m_cullFaceMode"].as<unsigned>();
        m_polygonMode = (OpenGLPolygonMode)drawSettings["m_polygonMode"].as<unsigned>();

        m_blending = drawSettings["m_blending"].as<bool>();
        m_blendingSrcFactor = (OpenGLBlendFactor)drawSettings["m_blendingSrcFactor"].as<unsigned>();
        m_blendingDstFactor = (OpenGLBlendFactor)drawSettings["m_blendingDstFactor"].as<unsigned>();
    }
}

void Material::OnInspect()
{
    if (Editor::DragAndDropButton<OpenGLUtils::GLProgram>(m_program, "Program"))
        m_saved = false;
    if(ImGui::Checkbox("Vertex color only", &m_vertexColorOnly)){
        m_saved = false;
    }

    ImGui::Separator();
    if (ImGui::TreeNodeEx("PBR##Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!m_albedoTexture.Get())
        {
            if (ImGui::ColorEdit3("Albedo##Material", &m_albedoColor.x))
            {
                m_saved = false;
            }
            if(!m_drawSettings.m_blending && ImGui::DragFloat("Transparency##Material", &m_transparency, 0.01f, 0.0f, 1.0f)){
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
        if(ImGui::DragFloat("Subsurface Factor##Material", &m_subsurfaceFactor, 0.01f, 0.0f, 1.0f)){
            m_saved = false;
        }
        if(m_subsurfaceFactor > 0.0f) {
            if (ImGui::DragFloat("Subsurface Radius##Material", &m_subsurfaceRadius, 0.01f, 0.0f, 999.0f)) {
                m_saved = false;
            }
            if (ImGui::ColorEdit3("Subsurface Color##Material", &m_subsurfaceColor.x)) {
                m_saved = false;
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Others##Material"))
    {
        if(m_drawSettings.OnInspect()) m_saved = false;
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Textures##Material"))
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

    m_drawSettings.Save("m_drawSettings", out);

    out << YAML::Key << "m_metallic" << YAML::Value << m_metallic;
    out << YAML::Key << "m_roughness" << YAML::Value << m_roughness;
    out << YAML::Key << "m_ambient" << YAML::Value << m_ambient;
    out << YAML::Key << "m_emission" << YAML::Value << m_emission;
    out << YAML::Key << "m_albedoColor" << YAML::Value << m_albedoColor;
    out << YAML::Key << "m_transparency" << YAML::Value << m_transparency;
    out << YAML::Key << "m_subsurfaceColor" << YAML::Value << m_subsurfaceColor;
    out << YAML::Key << "m_subsurfaceFactor" << YAML::Value << m_subsurfaceFactor;
    out << YAML::Key << "m_subsurfaceRadius" << YAML::Value << m_subsurfaceRadius;

    out << YAML::Key << "m_vertexColorOnly" << YAML::Value << m_vertexColorOnly;
}
void Material::Deserialize(const YAML::Node &in)
{
    m_albedoTexture.Load("m_albedoTexture", in);
    m_normalTexture.Load("m_normalTexture", in);
    m_metallicTexture.Load("m_metallicTexture", in);
    m_roughnessTexture.Load("m_roughnessTexture", in);
    m_aoTexture.Load("m_aoTexture", in);
    m_program.Load("m_program", in);

    m_drawSettings.Load("m_drawSettings", in);

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
    if (in["m_subsurfaceFactor"])
        m_subsurfaceFactor = in["m_subsurfaceFactor"].as<float>();
    if (in["m_subsurfaceRadius"])
        m_subsurfaceRadius = in["m_subsurfaceRadius"].as<float>();

    if (in["m_vertexColorOnly"])
        m_vertexColorOnly = in["m_vertexColorOnly"].as<bool>();
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
