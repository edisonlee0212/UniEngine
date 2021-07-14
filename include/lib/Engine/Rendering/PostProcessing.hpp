#pragma once
#include <CameraComponent.hpp>
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
    virtual void Process(CameraComponent &cameraComponent, RenderTarget &renderTarget) const = 0;
    virtual void OnGui(CameraComponent &cameraComponent) = 0;
};

class UNIENGINE_API PostProcessing final : public PrivateComponentBase, public RenderTarget
{
    std::map<std::string, std::unique_ptr<PostProcessingLayer>> _Layers;

  public:
    template <typename T> T *GetLayer();
    void PushLayer(std::unique_ptr<PostProcessingLayer> layer);
    void RemoveLayer(const std::string &layerName);
    void SetEnableLayer(const std::string &layerName, bool enabled);
    PostProcessing();
    void Process();
    void ResizeResolution(int x, int y);
    void OnGui() override;
    void Serialize(YAML::Emitter &out) override;
    void Deserialize(const YAML::Node &in) override;
};

template <typename T> T *PostProcessing::GetLayer()
{
    for (auto &layer : _Layers)
    {
        if (dynamic_cast<T *>(layer.second.get()))
        {
            return dynamic_cast<T *>(layer.second.get());
        }
    }
    return nullptr;
}

#pragma region Effects
class UNIENGINE_API Bloom : public PostProcessingLayer
{
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_result;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_brightColor;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_flatColor;
    std::shared_ptr<OpenGLUtils::GLProgram> m_separateProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_filterProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_combineProgram;

  public:
    float m_intensity = 0.05f;
    float m_threshold = 1.0f;
    float m_clamp = 0.0f;
    int m_diffusion = 8;
    BezierCubic2D m_graph;
    void Init() override;
    void ResizeResolution(int x, int y) override;
    void Process(CameraComponent &cameraComponent, RenderTarget &renderTarget) const override;
    void OnGui(CameraComponent &cameraComponent) override;
};
class UNIENGINE_API SSAO : public PostProcessingLayer
{
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_originalColor;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_position;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_ssaoPosition;
    std::unique_ptr<OpenGLUtils::GLTexture2D> m_blur;
    std::shared_ptr<OpenGLUtils::GLProgram> m_positionReconstructProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_geometryProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_blurProgram;
    std::shared_ptr<OpenGLUtils::GLProgram> m_combineProgram;

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
    void Process(CameraComponent &cameraComponent, RenderTarget &renderTarget) const override;
    void OnGui(CameraComponent &cameraComponent) override;
};
#pragma endregion

} // namespace UniEngine
