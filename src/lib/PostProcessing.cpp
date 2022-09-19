#include <ProjectManager.hpp>
#include <DefaultResources.hpp>
#include <PostProcessing.hpp>
#include <Texture2D.hpp>
#include <Utilities.hpp>
using namespace UniEngine;

std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_separateProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_filterProgram;
std::shared_ptr<OpenGLUtils::GLProgram> Bloom::m_combineProgram;

std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_geometryProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_blurProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSAO::m_combineProgram;

std::shared_ptr<OpenGLUtils::GLProgram> SSR::m_reflectProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSR::m_blurProgram;
std::shared_ptr<OpenGLUtils::GLProgram> SSR::m_combineProgram;

void PostProcessing::PushLayer(const std::shared_ptr<PostProcessingLayer> &layer)
{
    if (!layer)
        return;
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
    PushLayer(std::make_shared<Bloom>());
    PushLayer(std::make_shared<SSAO>());
    PushLayer(std::make_shared<SSR>());
    ResizeResolution(1, 1);
    SetEnabled(true);
}

void PostProcessing::Process()
{
    auto scene = GetScene();
    auto owner = GetOwner();
    if (!scene->HasPrivateComponent<Camera>(owner))
        return;
    auto cameraComponent = scene->GetOrSetPrivateComponent<Camera>(owner).lock();
    ResizeResolution(cameraComponent->m_resolutionX, cameraComponent->m_resolutionY);
    auto ltw = scene->GetDataComponent<GlobalTransform>(owner);
    Camera::m_cameraInfoBlock.UploadMatrices(cameraComponent, ltw.GetPosition(), ltw.GetRotation());

    if (m_layers["SSAO"] && m_layers["SSAO"]->m_enabled)
    {
        m_layers["SSAO"]->Process(cameraComponent, *this);
    }
    if (m_layers["Bloom"] && m_layers["Bloom"]->m_enabled)
    {
        m_layers["Bloom"]->Process(cameraComponent, *this);
    }
    if (m_layers["SSR"] && m_layers["SSR"]->m_enabled)
    {
        m_layers["SSR"]->Process(cameraComponent, *this);
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

void PostProcessing::OnInspect()
{
    auto scene = GetScene();
    auto owner = GetOwner();
    if(!scene->HasPrivateComponent<Camera>(owner)){
        ImGui::Text("Camera not present!");
        return;
    }
    auto cameraComponent = scene->GetOrSetPrivateComponent<Camera>(owner).lock();
    for (auto &layer : m_layers)
    {
        if (layer.second)
        {
            ImGui::Checkbox(layer.second->m_name.c_str(), &layer.second->m_enabled);
            if (layer.second->m_enabled)
                layer.second->OnInspect(cameraComponent);
        }
    }
}

void PostProcessing::Serialize(YAML::Emitter &out)
{
}

void PostProcessing::Deserialize(const YAML::Node &in)
{
}
void PostProcessing::PostCloneAction(const std::shared_ptr<IPrivateComponent> &target)
{
}
PostProcessing &PostProcessing::operator=(const PostProcessing &source)
{
    PushLayer(std::make_shared<Bloom>());
    PushLayer(std::make_shared<SSAO>());
    PushLayer(std::make_shared<SSR>());
    ResizeResolution(1, 1);
    SetEnabled(source.IsEnabled());
    return *this;
}
void PostProcessing::OnDestroy()
{
    m_layers.clear();
}

Bloom::Bloom()
{
    m_name = "Bloom";
    m_graph = Bezier2D();
    m_graph.m_controlPoints[1] = glm::vec2(1, 0);
    m_graph.m_controlPoints[2] = glm::vec2(0.9, 1.0);

    m_brightColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_brightColor->SetData(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0);
    m_brightColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_brightColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_result = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_result->SetData(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0);
    m_result->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_result->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_result->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_flatColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_flatColor->SetData(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0);
    m_flatColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_flatColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_enabled = true;
}

void Bloom::ResizeResolution(int x, int y)
{
    m_brightColor->ReSize(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0, x, y);
    m_result->ReSize(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0, x, y);
    m_flatColor->ReSize(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0, x, y);
}

void Bloom::Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const
{
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);

    DefaultResources::ScreenVAO->Bind();

    m_separateProgram->Bind();

    renderTarget.AttachTexture(m_flatColor.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.AttachTexture(m_brightColor.get(), GL_COLOR_ATTACHMENT1);
    renderTarget.Bind();
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    cameraComponent->m_colorTexture->m_texture->Bind(0);
    m_separateProgram->SetInt("image", 0);
    m_separateProgram->SetFloat("threshold", m_threshold);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_filterProgram->Bind();
    renderTarget.AttachTexture(m_result.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_brightColor->Bind(0);
    m_filterProgram->SetInt("image", 0);
    m_filterProgram->SetBool("horizontal", false);
    m_filterProgram->SetInt("sampleStep", m_sampleStep);
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
    renderTarget.AttachTexture(cameraComponent->m_colorTexture->UnsafeGetGLTexture().get(), GL_COLOR_ATTACHMENT0);
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_flatColor->Bind(0);
    m_brightColor->Bind(1);
    m_combineProgram->SetInt("flatColor", 0);
    m_combineProgram->SetInt("brightColor", 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Bloom::OnInspect(const std::shared_ptr<Camera> &cameraComponent)
{
    if (ImGui::TreeNode("Bloom Settings"))
    {
        ImGui::DragFloat("Threshold##Bloom", &m_threshold, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("Clamp##Bloom", &m_clamp, 0.01f, 0.0f, 5.0f);

        ImGui::Text("Blur settings:");
        ImGui::DragInt("Blur Step##Bloom", &m_sampleStep, 1, 1, 64);
        ImGui::DragFloat("Intensity##Bloom", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##Bloom", &m_diffusion, 1.0f, 1, 64);
        m_graph.DrawGraph("Bezier##Bloom");
        if (ImGui::TreeNode("Debug##Bloom"))
        {
            ImGui::Image((ImTextureID)m_brightColor->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

SSAO::SSAO()
{
    m_graph = Bezier2D();
    m_graph.m_controlPoints[1] = glm::vec2(1, 0);
    m_graph.m_controlPoints[2] = glm::vec2(0.9, 1.0);
    m_graph.m_fixed = false;
    m_name = "SSAO";

    m_originalColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_originalColor->SetData(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0);
    m_originalColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_ssaoPosition = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_R16F, 1, 1, false);
    m_ssaoPosition->SetData(0, GL_R16F, GL_RED, GL_HALF_FLOAT, 0);
    m_ssaoPosition->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_ssaoPosition->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_ssaoPosition->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_ssaoPosition->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_blur = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_R16F, 1, 1, false);
    m_blur->SetData(0, GL_R16F, GL_RED, GL_HALF_FLOAT, 0);
    m_blur->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_blur->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_enabled = true;
}

void SSAO::ResizeResolution(int x, int y)
{
    m_originalColor->ReSize(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0, x, y);
    m_ssaoPosition->ReSize(0, GL_R16F, GL_RED, GL_HALF_FLOAT, 0, x, y);
    m_blur->ReSize(0, GL_R16F, GL_RED, GL_HALF_FLOAT, 0, x, y);
}

void SSAO::Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const
{
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    DefaultResources::ScreenVAO->Bind();

    m_geometryProgram->Bind();

    renderTarget.AttachTexture(m_originalColor.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.AttachTexture(m_ssaoPosition.get(), GL_COLOR_ATTACHMENT1);
    renderTarget.Bind();
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});

    cameraComponent->m_colorTexture->UnsafeGetGLTexture()->Bind(0);
    cameraComponent->m_gBufferNormal->Bind(1);
    cameraComponent->m_gBufferDepth->Bind(2);
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
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_ssaoPosition->Bind(0);
    m_blurProgram->SetInt("image", 0);
    m_blurProgram->SetInt("sampleStep", m_sampleStep);
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
    renderTarget.AttachTexture(cameraComponent->m_colorTexture->UnsafeGetGLTexture().get(), GL_COLOR_ATTACHMENT0);
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_originalColor->Bind(0);
    m_ssaoPosition->Bind(1);
    m_combineProgram->SetInt("originalColor", 0);
    m_combineProgram->SetInt("ao", 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SSAO::OnInspect(const std::shared_ptr<Camera> &cameraComponent)
{
    if (ImGui::TreeNode("SSAO Settings"))
    {
        ImGui::DragFloat("Radius##SSAO", &m_kernelRadius, 0.01f, 0.1f, 5.0f);
        ImGui::DragFloat("Bias##SSAO", &m_kernelBias, 0.001f, 0.0f, 1.0f);
        ImGui::DragInt("Sample Size##SSAO", &m_sampleSize, 1, 1, 64);
        ImGui::Text("Blur settings:");
        ImGui::DragInt("Blur Step##SSAO", &m_sampleStep, 1, 1, 64);
        ImGui::DragFloat("Intensity##SSAO", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##SSAO", &m_diffusion, 1.0f, 1, 64);
        m_graph.DrawGraph("Bezier##SSAO");
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Debug##SSAO"))
    {
        ImGui::Text("SSAO Proximity");
        ImGui::Image((ImTextureID)m_ssaoPosition->Id(), ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::TreePop();
    }
}

SSR::SSR()
{
    m_graph = Bezier2D();
    m_graph.m_controlPoints[1] = glm::vec2(1, 0);
    m_graph.m_controlPoints[2] = glm::vec2(0.9, 1.0);
    m_graph.m_fixed = false;
    m_name = "SSR";

    m_originalColor = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, 1, 1, false);
    m_originalColor->SetData(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0);
    m_originalColor->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_originalColor->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_reflectedColorVisibility = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGBA16F, 1, 1, false);
    m_reflectedColorVisibility->SetData(0, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 0);
    m_reflectedColorVisibility->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_reflectedColorVisibility->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_reflectedColorVisibility->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_reflectedColorVisibility->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_blur = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGBA16F, 1, 1, false);
    m_blur->SetData(0, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 0);
    m_blur->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_blur->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_blur->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_enabled = true;
}
void SSR::ResizeResolution(int x, int y)
{
    m_originalColor->ReSize(0, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 0, x, y);
    m_reflectedColorVisibility->ReSize(0, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 0, x, y);
    m_blur->ReSize(0, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 0, x, y);
}
void SSR::Process(const std::shared_ptr<Camera> &cameraComponent, RenderTarget &renderTarget) const
{
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    OpenGLUtils::SetEnable(OpenGLCapability::Blend, false);
    OpenGLUtils::SetEnable(OpenGLCapability::CullFace, false);
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);

    DefaultResources::ScreenVAO->Bind();

    m_reflectProgram->Bind();

    renderTarget.AttachTexture(m_originalColor.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.AttachTexture(m_reflectedColorVisibility.get(), GL_COLOR_ATTACHMENT1);
    renderTarget.Bind();
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});

    cameraComponent->m_colorTexture->UnsafeGetGLTexture()->Bind(0);
    cameraComponent->m_gBufferMetallicRoughnessEmissionAmbient->Bind(1);
    cameraComponent->m_gBufferNormal->Bind(2);
    cameraComponent->m_gBufferDepth->Bind(3);

    m_reflectProgram->SetInt("colorTexture", 0);
    m_reflectProgram->SetInt("gBufferMetallicRoughnessEmissionAmbient", 1);
    m_reflectProgram->SetInt("gBufferNormal", 2);
    m_reflectProgram->SetInt("gBufferDepth", 3);

    m_reflectProgram->SetInt("numBinarySearchSteps", m_numBinarySearchSteps);
    m_reflectProgram->SetFloat("step", m_step);
    m_reflectProgram->SetFloat("minRayStep", m_minRayStep);
    m_reflectProgram->SetInt("maxSteps", m_maxSteps);
    m_reflectProgram->SetFloat("reflectionSpecularFalloffExponent", m_reflectionSpecularFalloffExponent);


    glDrawArrays(GL_TRIANGLES, 0, 6);


    m_blurProgram->Bind();
    renderTarget.AttachTexture(m_blur.get(), GL_COLOR_ATTACHMENT0);
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_reflectedColorVisibility->Bind(0);
    m_blurProgram->SetInt("image", 0);
    m_blurProgram->SetInt("sampleStep", m_sampleStep);
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

    renderTarget.AttachTexture(m_reflectedColorVisibility.get(), GL_COLOR_ATTACHMENT0);
    m_blur->Bind(0);
    m_blurProgram->SetBool("horizontal", true);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    m_combineProgram->Bind();
    renderTarget.AttachTexture(cameraComponent->m_colorTexture->UnsafeGetGLTexture().get(), GL_COLOR_ATTACHMENT0);
    renderTarget.GetFrameBuffer()->DrawBuffers({GL_COLOR_ATTACHMENT0});
    m_originalColor->Bind(0);
    m_reflectedColorVisibility->Bind(1);
    m_combineProgram->SetInt("originalColor", 0);
    m_combineProgram->SetInt("reflectedColorVisibility", 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void SSR::OnInspect(const std::shared_ptr<Camera> &cameraComponent)
{
    if (ImGui::TreeNode("SSR Settings"))
    {
        ImGui::DragFloat("Step##SSR", &m_step, 0.01f, 0.01f, 1.0f);
        ImGui::DragFloat("Min ray step##SSR", &m_minRayStep, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Max step##SSR", &m_maxSteps, 1, 1, 100.0f);
        ImGui::DragFloat("Falloff##SSR", &m_reflectionSpecularFalloffExponent, 0.1f, 0.001f, 10.0f);
        ImGui::DragInt("Binary search step##SSR", &m_numBinarySearchSteps, 1.0f, 1, 64);

        ImGui::Text("Blur settings:");
        ImGui::DragInt("Blur Step##SSR", &m_sampleStep, 1, 1, 64);
        ImGui::DragFloat("Intensity##SSR", &m_intensity, 0.001f, 0.001f, 1.0f);
        ImGui::DragInt("Diffusion##SSR", &m_diffusion, 1.0f, 1, 64);
        m_graph.DrawGraph("Bezier##SSR");

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Debug##SSR"))
    {
        static float debugSacle = 0.25f;
        ImGui::DragFloat("Scale", &debugSacle, 0.01f, 0.1f, 1.0f);
        debugSacle = glm::clamp(debugSacle, 0.1f, 1.0f);
        ImGui::Image(
            (ImTextureID)m_reflectedColorVisibility->Id(),
            ImVec2(cameraComponent->m_resolutionX * debugSacle, cameraComponent->m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::Image(
            (ImTextureID)m_blur->Id(),
            ImVec2(cameraComponent->m_resolutionX * debugSacle, cameraComponent->m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::TreePop();
    }
}
