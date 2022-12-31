#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Texture2D.hpp>
#include "MaterialProperties.hpp"
namespace UniEngine
{
struct DrawSettings{
    bool m_cullFace = true;
    float m_lineWidth = 1.0f;
    float m_pointSize = 1.0f;
    OpenGLCullFace m_cullFaceMode = OpenGLCullFace::Back;
    OpenGLPolygonMode m_polygonMode = OpenGLPolygonMode::Fill;

    bool m_blending = false;
    OpenGLBlendFactor m_blendingSrcFactor = OpenGLBlendFactor::SrcAlpha;
    OpenGLBlendFactor m_blendingDstFactor = OpenGLBlendFactor::OneMinusSrcAlpha;
    bool OnInspect();
    void ApplySettings() const;

    void Save(const std::string &name, YAML::Emitter &out);
    void Load(const std::string &name, const YAML::Node &in);
};

class UNIENGINE_API Material : public IAsset
{
  public:
    AssetRef m_albedoTexture;
    AssetRef m_normalTexture;
    AssetRef m_metallicTexture;
    AssetRef m_roughnessTexture;
    AssetRef m_aoTexture;
    AssetRef m_program;
    std::map<std::string, float> m_floatPropertyList;
    std::map<std::string, glm::mat4> m_float4X4PropertyList;
    DrawSettings m_drawSettings;

    MaterialProperties m_materialProperties;

    bool m_vertexColorOnly = false;
    unsigned m_version = 0;
    void OnCreate() override;
    void OnInspect() override;
    void SetTexture(const TextureType &type, std::shared_ptr<Texture2D> texture);
    void RemoveTexture(TextureType type);
    void SetProgram(std::shared_ptr<OpenGLUtils::GLProgram> program);


    void CollectAssetRef(std::vector<AssetRef> &list) override;

    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;

};
} // namespace UniEngine
