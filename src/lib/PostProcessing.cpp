#include <DefaultResources.hpp>
#include <FileIO.hpp>
#include <PostProcessing.hpp>
#include <Texture2D.hpp>
using namespace UniEngine;

void PostProcessing::PushLayer(std::unique_ptr<PostProcessingLayer> layer)
{
    if (!layer)
        return;
    layer->Init();
    layer->ResizeResolution(m_resolutionX, m_resolutionY);
    _Layers[layer->m_name] = std::move(layer);
}

void PostProcessing::RemoveLayer(const std::string &layerName)
{
    if (_Layers[layerName])
        _Layers.erase(layerName);
}

void PostProcessing::SetEnableLayer(const std::string &layerName, bool enabled)
{
    if (_Layers[layerName])
        _Layers[layerName]->m_enabled = enabled;
}

PostProcessing::PostProcessing()
{
    ResizeResolution(1, 1);
    SetEnabled(true);
}

void PostProcessing::Process()
{
    auto &cameraComponent = GetOwner().GetPrivateComponent<CameraComponent>();
    if (_Layers["SSAO"] && _Layers["SSAO"]->m_enabled)
    {
        _Layers["SSAO"]->Process(cameraComponent, *this);
    }
    if (_Layers["Bloom"] && _Layers["Bloom"]->m_enabled)
    {
        _Layers["Bloom"]->Process(cameraComponent, *this);
    }

    if (_Layers["GreyScale"] && _Layers["GreyScale"]->m_enabled)
    {
        _Layers["GreyScale"]->Process(cameraComponent, *this);
    }
}

void PostProcessing::ResizeResolution(int x, int y)
{
    if (m_resolutionX == x && m_resolutionY == y)
        return;
    m_resolutionX = x;
    m_resolutionY = y;
    for (auto &layer : _Layers)
    {
        if (layer.second)
            layer.second->ResizeResolution(x, y);
    }
}

void PostProcessing::OnGui()
{
    auto &cameraComponent = GetOwner().GetPrivateComponent<CameraComponent>();
    for (auto &layer : _Layers)
    {
        if (layer.second)
        {
            ImGui::Checkbox(layer.second->m_name.c_str(), &layer.second->m_enabled);
            if (layer.second->m_enabled)
                layer.second->OnGui(cameraComponent);
        }
    }
}

void PostProcessing::Serialize(YAML::Emitter &out)
{
}

void PostProcessing::Deserialize(const YAML::Node &in)
{
}

void UniEngine::Bloom::Init()
{
    m_name = "Bloom";
    m_bezierGraph = BezierCubic2D();
    m_bezierGraph.m_controlPoints[1] = glm::vec2(1, 0);
    m_bezierGraph.m_controlPoints[2] = glm::vec2(0.9, 1.0);
    m_brightColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB32F, 1, 1, false);
    m_brightColor->SetData(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0);
    m_brightColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_result = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB32F, 1, 1, false);
    m_result->SetData(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0);
    m_result->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_result->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_flatColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB32F, 1, 1, false);
    m_flatColor->SetData(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0);
    m_flatColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_separateProgram = std::make_unique<OpenGLUtils::GLProgram>(
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Vertex,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Fragment,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BloomSeparator.frag"))));

    m_filterProgram = std::make_unique<OpenGLUtils::GLProgram>(
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Vertex,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Fragment,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BlurFilter.frag"))));
    m_combineProgram = std::make_unique<OpenGLUtils::GLProgram>(
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Vertex,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Vertex/TexturePassThrough.vert"))),
        std::make_shared<OpenGLUtils::GLShader>(
            OpenGLUtils::ShaderType::Fragment,
            std::string("#version 460 core\n") +
                FileIO::LoadFileAsString(FileIO::GetResourcePath("Shaders/Fragment/BloomCombine.frag"))));
}

void UniEngine::Bloom::ResizeResolution(int x, int y)
{
    m_brightColor->ReSize(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0, x, y);
    m_result->ReSize(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0, x, y);
    m_flatColor->ReSize(0, GL_RGB32F, GL_RGB, GL_FLOAT, 0, x, y);
}

void UniEngine::Bloom::Process(std::unique_ptr<CameraComponent> &cameraComponent, RenderTarget &renderTarget) const
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    unsigned int enums[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

    DefaultResources::GLPrograms::ScreenVAO->Bind();

    m_separateProgram->Bind();

    renderTarget.AttachTexture(m_flatColor.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.AttachTexture(m_brightColor.get(), GL_COLOR_ATTACHMENT1);
    renderTarget.Bind();
    glDrawBuffers(2, enums);
    cameraComponent->m_colorTexture->m_texture->Bind(0);
    m_separateProgram->SetInt("image", 0);
    m_separateProgram->SetFloat("threshold", m_threshold);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_filterProgram->Bind();
    renderTarget.AttachTexture(m_result.get(), GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_brightColor->Bind(0);
    m_filterProgram->SetInt("image", 0);
    m_filterProgram->SetBool("horizontal", false);
    m_filterProgram->SetFloat("sampleScale", 1.0f);
    m_filterProgram->SetFloat4(
        "bezier",
        m_bezierGraph.m_controlPoints[1][0],
        m_bezierGraph.m_controlPoints[1][1],
        m_bezierGraph.m_controlPoints[2][0],
        m_bezierGraph.m_controlPoints[2][1]);
    m_filterProgram->SetInt("diffusion", m_diffusion);
    m_filterProgram->SetFloat("clamp", m_clamp);
    m_filterProgram->SetFloat("intensity", m_intensity);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    renderTarget.AttachTexture(m_brightColor.get(), GL_COLOR_ATTACHMENT0);
    m_result->Bind(0);
    m_filterProgram->SetBool("horizontal", true);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_combineProgram->Bind();
    renderTarget.AttachTexture(cameraComponent->m_colorTexture->Texture().get(), GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_flatColor->Bind(0);
    m_brightColor->Bind(1);
    m_combineProgram->SetInt("flatColor", 0);
    m_combineProgram->SetInt("brightColor", 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UniEngine::Bloom::OnGui(std::unique_ptr<CameraComponent> &cameraComponent)
{
    if (ImGui::TreeNode("Bloom Settings"))
    {
        ImGui::DragFloat("Intensity##Bloom", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##Bloom", &m_diffusion, 1.0f, 1, 64);
        ImGui::DragFloat("Threshold##Bloom", &m_threshold, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("Clamp##Bloom", &m_clamp, 0.01f, 0.0f, 5.0f);
        m_bezierGraph.Graph("Bezier##Bloom");
        if (ImGui::TreeNode("Debug##Bloom"))
        {
            ImGui::Image((ImTextureID)m_flatColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((ImTextureID)m_result->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((ImTextureID)m_brightColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::TreePop();
    }
}
