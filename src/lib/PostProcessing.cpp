#include <AssetManager.hpp>
#include <Core/FileIO.hpp>
#include <DefaultResources.hpp>
#include <Gui.hpp>
#include <PostProcessing.hpp>
#include <Texture2D.hpp>
using namespace UniEngine;

std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_separateProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_filterProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_combineProgram;

std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_geometryProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_blurProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_combineProgram;

void PostProcessing::PushLayer(std::unique_ptr<PostProcessingLayer> layer)
{
    if (!layer)
        return;
    layer->Init();
    layer->ResizeResolution(m_resolutionX, m_resolutionY);
    m_layers[layer->m_name] = std::move(layer);
}

void PostProcessing::RemoveLayer(const std::string &layerName)
{
    if (m_layers[layerName])
        m_layers.erase(layerName);
}

void PostProcessing::SetEnableLayer(const std::string &layerName, bool enabled)
{
    if (m_layers[layerName])
        m_layers[layerName]->m_enabled = enabled;
}

void PostProcessing::OnCreate()
{
    ResizeResolution(1, 1);

    PushLayer(std::make_unique<Bloom>());
    PushLayer(std::make_unique<SSAO>());

    SetEnabled(true);
}

void PostProcessing::Process()
{
    if(!GetOwner().HasPrivateComponent<Camera>()) return;
    auto &cameraComponent = GetOwner().GetPrivateComponent<Camera>();
    ResizeResolution(cameraComponent.m_resolutionX, cameraComponent.m_resolutionY);
    auto ltw = cameraComponent.GetOwner().GetDataComponent<GlobalTransform>();
    Camera::m_cameraInfoBlock.UpdateMatrices(
        cameraComponent, ltw.GetPosition(), ltw.GetRotation());
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent);

    if (m_layers["SSAO"] && m_layers["SSAO"]->m_enabled)
    {
        m_layers["SSAO"]->Process(cameraComponent, *this);
    }
    if (m_layers["Bloom"] && m_layers["Bloom"]->m_enabled)
    {
        m_layers["Bloom"]->Process(cameraComponent, *this);
    }

    if (m_layers["GreyScale"] && m_layers["GreyScale"]->m_enabled)
    {
        m_layers["GreyScale"]->Process(cameraComponent, *this);
    }
}

void PostProcessing::ResizeResolution(int x, int y)
{
    if (m_resolutionX == x && m_resolutionY == y)
        return;
    m_resolutionX = x;
    m_resolutionY = y;
    for (auto &layer : m_layers)
    {
        if (layer.second)
            layer.second->ResizeResolution(x, y);
    }
}

void PostProcessing::OnGui()
{
    auto &cameraComponent = GetOwner().GetPrivateComponent<Camera>();
    for (auto &layer : m_layers)
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

void Bloom::Init()
{
    m_name = "Bloom";
    m_graph = BezierCubic2D();
    m_graph.m_controlPoints[1] = glm::vec2(1, 0);
    m_graph.m_controlPoints[2] = glm::vec2(0.9, 1.0);
    m_brightColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_brightColor->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_brightColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_result = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_result->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_result->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_result->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_flatColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_flatColor->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_flatColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    m_enabled = true;
}

void Bloom::ResizeResolution(int x, int y)
{
    m_brightColor->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, x, y);
    m_result->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, x, y);
    m_flatColor->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, x, y);
}

void Bloom::Process(Camera &cameraComponent, RenderTarget &renderTarget) const
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
    cameraComponent.m_colorTexture->m_texture->Bind(0);
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
        m_graph.m_controlPoints[1][0],
        m_graph.m_controlPoints[1][1],
        m_graph.m_controlPoints[2][0],
        m_graph.m_controlPoints[2][1]);
    m_filterProgram->SetInt("diffusion", m_diffusion);
    m_filterProgram->SetFloat("clamp", m_clamp);
    m_filterProgram->SetFloat("intensity", m_intensity);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    renderTarget.AttachTexture(m_brightColor.get(), GL_COLOR_ATTACHMENT0);
    m_result->Bind(0);
    m_filterProgram->SetBool("horizontal", true);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_combineProgram->Bind();
    renderTarget.AttachTexture(cameraComponent.m_colorTexture->Texture().get(), GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_flatColor->Bind(0);
    m_brightColor->Bind(1);
    m_combineProgram->SetInt("flatColor", 0);
    m_combineProgram->SetInt("brightColor", 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Bloom::OnGui(Camera &cameraComponent)
{
    if (ImGui::TreeNode("Bloom Settings"))
    {
        ImGui::DragFloat("Intensity##Bloom", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##Bloom", &m_diffusion, 1.0f, 1, 64);
        ImGui::DragFloat("Threshold##Bloom", &m_threshold, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("Clamp##Bloom", &m_clamp, 0.01f, 0.0f, 5.0f);
        m_graph.Graph("Bezier##Bloom");
        if (ImGui::TreeNode("Debug##Bloom"))
        {
            ImGui::Image((ImTextureID)m_flatColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((ImTextureID)m_result->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((ImTextureID)m_brightColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

void SSAO::Init()
{
    m_name = "SSAO";
    m_graph = BezierCubic2D();
    m_graph.m_controlPoints[1] = glm::vec2(1, 0);
    m_graph.m_controlPoints[2] = glm::vec2(0.9, 1.0);
    m_originalColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_originalColor->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_originalColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_ssaoPosition = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_R16F, 1, 1, false);
    m_ssaoPosition->SetData(0, GL_R16F, GL_RED, GL_FLOAT, 0);
    m_ssaoPosition->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_ssaoPosition->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_ssaoPosition->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_ssaoPosition->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_blur = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_R16F, 1, 1, false);
    m_blur->SetData(0, GL_R16F, GL_RED, GL_FLOAT, 0);
    m_blur->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_blur->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    m_enabled = true;
}

void SSAO::ResizeResolution(int x, int y)
{
    m_originalColor->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, x, y);
    m_ssaoPosition->ReSize(0, GL_R16F, GL_RED, GL_FLOAT, 0, x, y);
    m_blur->ReSize(0, GL_R16F, GL_RED, GL_FLOAT, 0, x, y);
}

void SSAO::Process(Camera &cameraComponent, RenderTarget &renderTarget) const
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    unsigned int enums[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    DefaultResources::GLPrograms::ScreenVAO->Bind();

    m_geometryProgram->Bind();
    renderTarget.AttachTexture(m_originalColor.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.AttachTexture(m_ssaoPosition.get(), GL_COLOR_ATTACHMENT1);
    glDrawBuffers(2, enums);
    renderTarget.Bind();
    cameraComponent.m_colorTexture->Texture()->Bind(0);
    cameraComponent.m_gBufferNormal->Bind(1);
    cameraComponent.m_gBufferDepth->Bind(2);
    m_geometryProgram->SetInt("color", 0);
    m_geometryProgram->SetInt("gNormal", 1);
    m_geometryProgram->SetInt("gDepth", 2);
    m_geometryProgram->SetFloat("radius", m_kernelRadius);
    m_geometryProgram->SetFloat("bias", m_kernelBias);
    m_geometryProgram->SetFloat("noiseScale", m_scale);
    m_geometryProgram->SetInt("kernelSize", m_sampleSize);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_blurProgram->Bind();
    renderTarget.AttachTexture(m_blur.get(), GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_ssaoPosition->Bind(0);
    m_blurProgram->SetInt("image", 0);
    m_blurProgram->SetFloat("sampleScale", m_blurScale);
    m_blurProgram->SetBool("horizontal", false);
    m_blurProgram->SetFloat4(
        "bezier",
        m_graph.m_controlPoints[1][0],
        m_graph.m_controlPoints[1][1],
        m_graph.m_controlPoints[2][0],
        m_graph.m_controlPoints[2][1]);
    m_blurProgram->SetInt("diffusion", m_diffusion);
    m_blurProgram->SetFloat("intensity", m_intensity);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    renderTarget.AttachTexture(m_ssaoPosition.get(), GL_COLOR_ATTACHMENT0);
    m_blur->Bind(0);
    m_blurProgram->SetBool("horizontal", true);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_combineProgram->Bind();
    renderTarget.AttachTexture(cameraComponent.m_colorTexture->Texture().get(), GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_originalColor->Bind(0);
    m_ssaoPosition->Bind(1);
    m_combineProgram->SetInt("originalColor", 0);
    m_combineProgram->SetInt("ao", 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SSAO::OnGui(Camera &cameraComponent)
{
    if (ImGui::TreeNode("SSAO Settings"))
    {
        ImGui::DragFloat("Radius##SSAO", &m_kernelRadius, 0.01f, 0.1f, 5.0f);
        ImGui::DragFloat("Bias##SSAO", &m_kernelBias, 0.001f, 0.0f, 1.0f);
        ImGui::DragInt("Sample Size##SSAO", &m_sampleSize, 1, 1, 64);
        ImGui::DragFloat("Blur Scale##SSAO", &m_blurScale, 0.001f, 0.01f, 1.0f);
        ImGui::DragFloat("Intensity##SSAO", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##SSAO", &m_diffusion, 1.0f, 1, 64);
        m_graph.Graph("Bezier##SSAO");
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Debug##SSAO"))
    {
        ImGui::Text("Original Color");
        ImGui::Image((ImTextureID)m_originalColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Text("SSAO Proximity");
        ImGui::Image((ImTextureID)m_ssaoPosition->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Text("Blur");
        ImGui::Image((ImTextureID)m_blur->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::TreePop();
    }
}
