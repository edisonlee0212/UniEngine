#pragma once
#include <Core/OpenGLUtils.hpp>
#include <Texture2D.hpp>
namespace UniEngine
{
enum class UNIENGINE_API MaterialPolygonMode
{
    Fill,
    Line,
    Point
};

enum class UNIENGINE_API MaterialCullingMode
{
    Back,
    Front,
    Off
};

enum class UNIENGINE_API MaterialBlendingMode
{
    Off,
    OneMinusSrcAlpha,
};

struct MaterialFloatProperty
{
    std::string m_name;
    float m_value;
    MaterialFloatProperty(const std::string &name, const float &value);
};
struct MaterialMat4Property
{
    std::string m_name;
    glm::mat4 m_value;
    MaterialMat4Property(const std::string &name, const glm::mat4 &value);
};
class UNIENGINE_API Material : public IAsset
{
  public:
    std::map<TextureType, std::shared_ptr<Texture2D>> m_textures;
    std::shared_ptr<OpenGLUtils::GLProgram> m_program;
    std::vector<MaterialFloatProperty> m_floatPropertyList;
    std::vector<MaterialMat4Property> m_float4X4PropertyList;
    MaterialPolygonMode m_polygonMode = MaterialPolygonMode::Fill;
    MaterialCullingMode m_cullingMode = MaterialCullingMode::Back;
    MaterialBlendingMode m_blendingMode = MaterialBlendingMode::Off;
    float m_metallic = 0.3f;
    float m_roughness = 0.3f;
    float m_ambient = 1.0f;
    float m_emission = 0.0f;
    glm::vec3 m_albedoColor = glm::vec3(1.0f);
    bool m_alphaDiscardEnabled = true;
    float m_alphaDiscardOffset = 0.1f;
    void OnCreate() override;
    void OnGui();
    void SetMaterialProperty(const std::string &name, const float &value);
    void SetMaterialProperty(const std::string &name, const glm::mat4 &value);
    void SetTexture(const TextureType &type, std::shared_ptr<Texture2D> texture);
    void RemoveTexture(TextureType type);
    void SetProgram(std::shared_ptr<OpenGLUtils::GLProgram> program);
};
} // namespace UniEngine
