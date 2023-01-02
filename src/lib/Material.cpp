#include "DefaultResources.hpp"
#include "Editor.hpp"
#include <Material.hpp>

using namespace UniEngine;

void Material::OnCreate() {
    m_program = DefaultResources::GLPrograms::StandardProgram;
}

static const char *PolygonMode[]{"Point", "Line", "Fill"};
static const char *CullingMode[]{"Front", "Back", "FrontAndBack"};
static const char *BlendingFactor[]{"Zero", "One", "SrcColor", "OneMinusSrcColor", "DstColor", "OneMinusDstColor",
                                    "SrcAlpha", "OneMinusSrcAlpha",
                                    "DstAlpha", "OneMinusDstAlpha", "ConstantColor", "OneMinusConstantColor",
                                    "ConstantAlpha", "OneMinusConstantAlpha", "SrcAlphaSaturate",
                                    "Src1Color", "OneMinusSrc1Color", "Src1Alpha", "OneMinusSrc1Alpha"};

bool DrawSettings::OnInspect() {
    bool changed = false;
    int polygonMode = 0;
    switch (m_polygonMode)
    {
    case OpenGLPolygonMode::Point: polygonMode = 0; break;
    case OpenGLPolygonMode::Line: polygonMode = 1; break;
    case OpenGLPolygonMode::Fill: polygonMode = 2; break;
    }
    if (ImGui::Combo(
            "Polygon Mode",
            &polygonMode,
            PolygonMode,
            IM_ARRAYSIZE(PolygonMode))) {
        changed = true;
        switch (polygonMode)
        {
        case 0: m_polygonMode = OpenGLPolygonMode::Point; break;
        case 1: m_polygonMode = OpenGLPolygonMode::Line; break;
        case 2: m_polygonMode = OpenGLPolygonMode::Fill; break;
        }
    }
    if(m_polygonMode == OpenGLPolygonMode::Line)
    {
        ImGui::DragFloat("Line width", &m_lineWidth, 0.1f, 0.0f, 100.0f);
    }
    if (m_polygonMode == OpenGLPolygonMode::Point)
    {
        ImGui::DragFloat("Point size", &m_pointSize, 0.1f, 0.0f, 100.0f);
    }
    int cullFaceMode = 0;
    switch (m_cullFaceMode)
    {
    case OpenGLCullFace::Front: cullFaceMode = 0; break;
    case OpenGLCullFace::Back: cullFaceMode = 1; break;
    case OpenGLCullFace::FrontAndBack: cullFaceMode = 2; break;
    }
    if (ImGui::Checkbox("Cull Face", &m_cullFace)) changed = true;
    if (m_cullFace && ImGui::Combo(
            "Cull Face Mode",
            &cullFaceMode,
            CullingMode,
            IM_ARRAYSIZE(CullingMode))) {
        changed = true;
        switch (cullFaceMode)
        {
        case 0: m_cullFaceMode = OpenGLCullFace::Front; break;
        case 1: m_cullFaceMode = OpenGLCullFace::Back; break;
        case 2: m_cullFaceMode = OpenGLCullFace::FrontAndBack; break;
        }
    }

    if (ImGui::Checkbox("Blending", &m_blending)) changed = true;

    if (false && m_blending) {
        if (ImGui::Combo(
                "Blending Source Factor",
                reinterpret_cast<int *>(&m_blendingSrcFactor),
                BlendingFactor,
                IM_ARRAYSIZE(BlendingFactor))) {
            changed = true;
        }
        if (ImGui::Combo(
                "Blending Destination Factor",
                reinterpret_cast<int *>(&m_blendingDstFactor),
                BlendingFactor,
                IM_ARRAYSIZE(BlendingFactor))) {
            changed = true;
        }
    }
    return changed;
}

void DrawSettings::Save(const std::string &name, YAML::Emitter &out) {
    out << YAML::Key << name << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "m_cullFace" << YAML::Value << m_cullFace;
    out << YAML::Key << "m_lineWidth" << YAML::Value << m_lineWidth;
    out << YAML::Key << "m_pointSize" << YAML::Value << m_pointSize;
    out << YAML::Key << "m_cullFaceMode" << YAML::Value << (unsigned) m_cullFaceMode;
    out << YAML::Key << "m_polygonMode" << YAML::Value << (unsigned) m_polygonMode;
    out << YAML::Key << "m_blending" << YAML::Value << m_blending;
    out << YAML::Key << "m_blendingSrcFactor" << YAML::Value << (unsigned) m_blendingSrcFactor;
    out << YAML::Key << "m_blendingDstFactor" << YAML::Value << (unsigned) m_blendingDstFactor;
    out << YAML::EndMap;
}

void DrawSettings::Load(const std::string &name, const YAML::Node &in) {
    if (in[name]) {
        const auto &drawSettings = in[name];
        if(drawSettings["m_cullFace"]) m_cullFace = drawSettings["m_cullFace"].as<bool>();
        if (drawSettings["m_lineWidth"]) m_lineWidth = drawSettings["m_lineWidth"].as<float>();
        if (drawSettings["m_pointSize"]) m_pointSize = drawSettings["m_pointSize"].as<float>();
        if (drawSettings["m_cullFaceMode"]) m_cullFaceMode = (OpenGLCullFace) drawSettings["m_cullFaceMode"].as<unsigned>();
        if (drawSettings["m_polygonMode"]) m_polygonMode = (OpenGLPolygonMode) drawSettings["m_polygonMode"].as<unsigned>();

        if (drawSettings["m_blending"]) m_blending = drawSettings["m_blending"].as<bool>();
        if (drawSettings["m_blendingSrcFactor"]) m_blendingSrcFactor = (OpenGLBlendFactor) drawSettings["m_blendingSrcFactor"].as<unsigned>();
        if (drawSettings["m_blendingDstFactor"]) m_blendingDstFactor = (OpenGLBlendFactor) drawSettings["m_blendingDstFactor"].as<unsigned>();
    }
}

void Material::OnInspect() {
    bool changed = false;
    if (Editor::DragAndDropButton<OpenGLUtils::GLProgram>(m_program, "Program"))
        changed = true;
    if (ImGui::Checkbox("Vertex color only", &m_vertexColorOnly)) {
        changed = true;
    }

    ImGui::Separator();
    if (ImGui::TreeNodeEx("PBR##Material", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (ImGui::ColorEdit3("Albedo##Material", &m_materialProperties.m_albedoColor.x)) {
            changed = true;
        }
        if (ImGui::DragFloat("Subsurface##Material", &m_materialProperties.m_subsurfaceFactor, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (m_materialProperties.m_subsurfaceFactor > 0.0f) {
            if (ImGui::DragFloat3("Subsurface Radius##Material", &m_materialProperties.m_subsurfaceRadius.x, 0.01f, 0.0f, 999.0f)) {
                changed = true;
            }
            if (ImGui::ColorEdit3("Subsurface Color##Material", &m_materialProperties.m_subsurfaceColor.x)) {
                changed = true;
            }
        }
        if (ImGui::DragFloat("Metallic##Material", &m_materialProperties.m_metallic, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }

        if (ImGui::DragFloat("Specular##Material", &m_materialProperties.m_specular, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Specular Tint##Material", &m_materialProperties.m_specularTint, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Roughness##Material", &m_materialProperties.m_roughness, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Sheen##Material", &m_materialProperties.m_sheen, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Sheen Tint##Material", &m_materialProperties.m_sheenTint, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Clear Coat##Material", &m_materialProperties.m_clearCoat, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Clear Coat Roughness##Material", &m_materialProperties.m_clearCoatRoughness, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("IOR##Material", &m_materialProperties.m_IOR, 0.01f, 0.0f, 5.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Transmission##Material", &m_materialProperties.m_transmission, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Transmission Roughness##Material", &m_materialProperties.m_transmissionRoughness, 0.01f, 0.0f, 1.0f)) {
            changed = true;
        }
        if (ImGui::DragFloat("Emission##Material", &m_materialProperties.m_emission, 0.01f, 0.0f, 10.0f)) {
            changed = true;
        }


        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Others##Material")) {
        if (m_drawSettings.OnInspect()) changed = true;
        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Textures##Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (Editor::DragAndDropButton<Texture2D>(m_albedoTexture, "Albedo Tex")) {
            changed = true;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_normalTexture, "Normal Tex")) {
            changed = true;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_metallicTexture, "Metallic Tex")) {
            changed = true;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_roughnessTexture, "Roughness Tex")) {
            changed = true;
        }
        if (Editor::DragAndDropButton<Texture2D>(m_aoTexture, "AO Tex")) {
            changed = true;
        }
        ImGui::TreePop();
    }
    if(changed){
        m_saved = false;
        m_version++;
    }
}

void Material::SetTexture(const TextureType &type, std::shared_ptr<Texture2D> texture) {
    switch (type) {
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
    m_saved = false;
    m_version++;
}

void Material::RemoveTexture(TextureType type) {
    switch (type) {
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
    m_saved = false;
    m_version++;
}

void Material::SetProgram(std::shared_ptr<OpenGLUtils::GLProgram> program) {
    m_program = std::move(program);
    m_saved = false;
    m_version++;
}

void Material::Serialize(YAML::Emitter &out) {
    m_albedoTexture.Save("m_albedoTexture", out);
    m_normalTexture.Save("m_normalTexture", out);
    m_metallicTexture.Save("m_metallicTexture", out);
    m_roughnessTexture.Save("m_roughnessTexture", out);
    m_aoTexture.Save("m_aoTexture", out);
    m_program.Save("m_program", out);

    m_drawSettings.Save("m_drawSettings", out);

    out << YAML::Key << "m_materialProperties.m_albedoColor" << YAML::Value << m_materialProperties.m_albedoColor;
    out << YAML::Key << "m_materialProperties.m_subsurfaceColor" << YAML::Value << m_materialProperties.m_subsurfaceColor;
    out << YAML::Key << "m_materialProperties.m_subsurfaceFactor" << YAML::Value << m_materialProperties.m_subsurfaceFactor;
    out << YAML::Key << "m_materialProperties.m_subsurfaceRadius" << YAML::Value << m_materialProperties.m_subsurfaceRadius;

    out << YAML::Key << "m_materialProperties.m_metallic" << YAML::Value << m_materialProperties.m_metallic;
    out << YAML::Key << "m_materialProperties.m_specular" << YAML::Value << m_materialProperties.m_specular;
    out << YAML::Key << "m_materialProperties.m_specularTint" << YAML::Value << m_materialProperties.m_specularTint;
    out << YAML::Key << "m_materialProperties.m_roughness" << YAML::Value << m_materialProperties.m_roughness;
    out << YAML::Key << "m_materialProperties.m_sheen" << YAML::Value << m_materialProperties.m_sheen;
    out << YAML::Key << "m_materialProperties.m_sheenTint" << YAML::Value << m_materialProperties.m_sheenTint;
    out << YAML::Key << "m_materialProperties.m_clearCoat" << YAML::Value << m_materialProperties.m_clearCoat;
    out << YAML::Key << "m_materialProperties.m_clearCoatRoughness" << YAML::Value << m_materialProperties.m_clearCoatRoughness;
    out << YAML::Key << "m_materialProperties.m_IOR" << YAML::Value << m_materialProperties.m_IOR;
    out << YAML::Key << "m_materialProperties.m_transmission" << YAML::Value << m_materialProperties.m_transmission;
    out << YAML::Key << "m_materialProperties.m_transmissionRoughness" << YAML::Value << m_materialProperties.m_transmissionRoughness;
    out << YAML::Key << "m_materialProperties.m_emission" << YAML::Value << m_materialProperties.m_emission;

    out << YAML::Key << "m_vertexColorOnly" << YAML::Value << m_vertexColorOnly;
}

void Material::Deserialize(const YAML::Node &in) {
    m_albedoTexture.Load("m_albedoTexture", in);
    m_normalTexture.Load("m_normalTexture", in);
    m_metallicTexture.Load("m_metallicTexture", in);
    m_roughnessTexture.Load("m_roughnessTexture", in);
    m_aoTexture.Load("m_aoTexture", in);
    m_program.Load("m_program", in);

    m_drawSettings.Load("m_drawSettings", in);
    if (in["m_materialProperties.m_albedoColor"])
        m_materialProperties.m_albedoColor = in["m_materialProperties.m_albedoColor"].as<glm::vec3>();
    if (in["m_materialProperties.m_subsurfaceColor"])
        m_materialProperties.m_subsurfaceColor = in["m_materialProperties.m_subsurfaceColor"].as<glm::vec3>();
    if (in["m_materialProperties.m_subsurfaceFactor"])
        m_materialProperties.m_subsurfaceFactor = in["m_materialProperties.m_subsurfaceFactor"].as<float>();
    if (in["m_materialProperties.m_subsurfaceRadius"])
        m_materialProperties.m_subsurfaceRadius = in["m_materialProperties.m_subsurfaceRadius"].as<glm::vec3>();
    if (in["m_materialProperties.m_metallic"])
        m_materialProperties.m_metallic = in["m_materialProperties.m_metallic"].as<float>();
    if (in["m_materialProperties.m_specular"])
        m_materialProperties.m_specular = in["m_materialProperties.m_specular"].as<float>();
    if (in["m_materialProperties.m_specularTint"])
        m_materialProperties.m_specularTint = in["m_materialProperties.m_specularTint"].as<float>();
    if (in["m_materialProperties.m_roughness"])
        m_materialProperties.m_roughness = in["m_materialProperties.m_roughness"].as<float>();
    if (in["m_materialProperties.m_sheen"])
        m_materialProperties.m_sheen = in["m_materialProperties.m_sheen"].as<float>();
    if (in["m_materialProperties.m_sheenTint"])
        m_materialProperties.m_sheenTint = in["m_materialProperties.m_sheenTint"].as<float>();
    if (in["m_materialProperties.m_clearCoat"])
        m_materialProperties.m_clearCoat = in["m_materialProperties.m_clearCoat"].as<float>();
    if (in["m_materialProperties.m_clearCoatRoughness"])
        m_materialProperties.m_clearCoatRoughness = in["m_materialProperties.m_clearCoatRoughness"].as<float>();
    if (in["m_materialProperties.m_IOR"])
        m_materialProperties.m_IOR = in["m_materialProperties.m_IOR"].as<float>();
    if (in["m_materialProperties.m_transmission"])
        m_materialProperties.m_transmission = in["m_materialProperties.m_transmission"].as<float>();
    if (in["m_materialProperties.m_transmissionRoughness"])
        m_materialProperties.m_transmissionRoughness = in["m_materialProperties.m_transmissionRoughness"].as<float>();
    if (in["m_materialProperties.m_emission"])
        m_materialProperties.m_emission = in["m_materialProperties.m_emission"].as<float>();

    if (in["m_vertexColorOnly"])
        m_vertexColorOnly = in["m_vertexColorOnly"].as<bool>();
    m_version = 0;
}

void Material::CollectAssetRef(std::vector<AssetRef> &list) {
    list.push_back(m_albedoTexture);
    list.push_back(m_normalTexture);
    list.push_back(m_metallicTexture);
    list.push_back(m_roughnessTexture);
    list.push_back(m_aoTexture);
    list.push_back(m_program);
}
