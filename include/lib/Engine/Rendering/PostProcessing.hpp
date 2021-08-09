#pragma once
#include <Camera.hpp>
namespace UniEngine
{
class UNIENGINE_API PostProcessingLayer
{
    friend class PostProcessing;

  protected:
    std::string m_name = "";

  public:
    bool m_enabled = false;
    virtual void Init() = 0;
    virtual void ResizeResolution(int x, int y) = 0;
    virtual void Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const = 0;
    virtual void OnGui(const std::shared_ptr<Camera> &cameraComponent) = 0;
};

class UNIENGINE_API PostProcessing final : public IPrivateComponent, public RenderTarget
{
    std::map<std::string, std::shared_ptr<PostProcessingLayer>> m_layers;

  public:
    template <typename T> std::weak_ptr<T> GetLayer();
    void PushLayer(const std::shared_ptr<PostProcessingLayer> &layer);
    void RemoveLayer(const std::string &layerName);
    void SetEnableLayer(const std::string &layerName, bool enabled);
    void OnCreate() override;
    void Process();
    void ResizeResolution(int x, int y);
    void OnGui() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

template <typename T> std::weak_ptr<T> PostProcessing::GetLayer()
{
    for (auto &layer : m_layers)
    {
        if (std::dynamic_pointer_cast<T>(layer.second))
        {
            return std::weak_ptr<T>(std::static_pointer_cast<T>(layer.second));
        }
    }
    throw 0;
}

#pragma region Effects
class UNIENGINE_API Bloom : public PostProcessingLayer
{
    friend class DefaultResources;

    std::unique_ptr<OpenGLUtils::GLTexture2D> m_result;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_brightColor;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_flatColor;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_separateProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_filterProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_combineProgram;

  public:
    float m_intensity = 0.1f;
    float m_threshold = 1.0f;
    float m_clamp = 0.0f;
    int m_diffusion = 8;
    BezierCubic2D m_graph;
    void Init() override;
    void ResizeResolution(int x, int y) override;
    void Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const override;
    void OnGui(const std::shared_ptr<Camera> &cameraComponent) override;
};
class UNIENGINE_API SSAO : public PostProcessingLayer
{
    friend class DefaultResources;

    std::unique_ptr<OpenGLUtils::GLTexture2D> m_originalColor;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_ssaoPosition;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_blur;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_geometryProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_blurProgram;
    static std::shared_ptr<OpenGLUtils::GLProgram> m_combineProgram;

  public:
    float m_intensity = 0.1f;
    int m_diffusion = 8;
    float m_blurScale = 1.0f;
    BezierCubic2D m_graph;
    float m_kernelRadius = 1.0f;
    float m_kernelBias = 0.01f;
    float m_scale = 1.0f;
    int m_sampleSize = 4;

    void Init() override;
    void ResizeResolution(int x, int y) override;
    void Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const override;
    void OnGui(const std::shared_ptr<Camera> &cameraComponent) override;
};
#pragma endregion

} // namespace UniEngine
