#include "AssetManager.hpp"

#include <Utilities.hpp>

#include <Camera.hpp>
#include <Cubemap.hpp>
#include <EditorManager.hpp>
#include <Gui.hpp>
#include <PostProcessing.hpp>
#include <RenderManager.hpp>
#include <SerializationManager.hpp>
#include <Texture2D.hpp>
using namespace UniEngine;

CameraInfoBlock Camera::m_cameraInfoBlock;
std::unique_ptr<OpenGLUtils::GLUBO> Camera::m_cameraUniformBufferBlock;

Plane::Plane() : m_a(0), m_b(0), m_c(0), m_d(0)
{
}

void Plane::Normalize()
{
    const float mag = glm::sqrt(m_a * m_a + m_b * m_b + m_c * m_c);
    m_a /= mag;
    m_b /= mag;
    m_c /= mag;
    m_d /= mag;
}

void Camera::CalculatePlanes(std::vector<Plane> &planes, glm::mat4 projection, glm::mat4 view)
{
    glm::mat4 comboMatrix = projection * glm::transpose(view);
    planes[0].m_a = comboMatrix[3][0] + comboMatrix[0][0];
    planes[0].m_b = comboMatrix[3][1] + comboMatrix[0][1];
    planes[0].m_c = comboMatrix[3][2] + comboMatrix[0][2];
    planes[0].m_d = comboMatrix[3][3] + comboMatrix[0][3];

    planes[1].m_a = comboMatrix[3][0] - comboMatrix[0][0];
    planes[1].m_b = comboMatrix[3][1] - comboMatrix[0][1];
    planes[1].m_c = comboMatrix[3][2] - comboMatrix[0][2];
    planes[1].m_d = comboMatrix[3][3] - comboMatrix[0][3];

    planes[2].m_a = comboMatrix[3][0] - comboMatrix[1][0];
    planes[2].m_b = comboMatrix[3][1] - comboMatrix[1][1];
    planes[2].m_c = comboMatrix[3][2] - comboMatrix[1][2];
    planes[2].m_d = comboMatrix[3][3] - comboMatrix[1][3];

    planes[3].m_a = comboMatrix[3][0] + comboMatrix[1][0];
    planes[3].m_b = comboMatrix[3][1] + comboMatrix[1][1];
    planes[3].m_c = comboMatrix[3][2] + comboMatrix[1][2];
    planes[3].m_d = comboMatrix[3][3] + comboMatrix[1][3];

    planes[4].m_a = comboMatrix[3][0] + comboMatrix[2][0];
    planes[4].m_b = comboMatrix[3][1] + comboMatrix[2][1];
    planes[4].m_c = comboMatrix[3][2] + comboMatrix[2][2];
    planes[4].m_d = comboMatrix[3][3] + comboMatrix[2][3];

    planes[5].m_a = comboMatrix[3][0] - comboMatrix[2][0];
    planes[5].m_b = comboMatrix[3][1] - comboMatrix[2][1];
    planes[5].m_c = comboMatrix[3][2] - comboMatrix[2][2];
    planes[5].m_d = comboMatrix[3][3] - comboMatrix[2][3];

    planes[0].Normalize();
    planes[1].Normalize();
    planes[2].Normalize();
    planes[3].Normalize();
    planes[4].Normalize();
    planes[5].Normalize();
}

void Camera::CalculateFrustumPoints(
    const std::shared_ptr<Camera> &cameraComponrnt,
    float nearPlane,
    float farPlane,
    glm::vec3 cameraPos,
    glm::quat cameraRot,
    glm::vec3 *points)
{
    const glm::vec3 front = cameraRot * glm::vec3(0, 0, -1);
    const glm::vec3 right = cameraRot * glm::vec3(1, 0, 0);
    const glm::vec3 up = cameraRot * glm::vec3(0, 1, 0);
    const glm::vec3 nearCenter = front * nearPlane;
    const glm::vec3 farCenter = front * farPlane;

    const float e = tanf(glm::radians(cameraComponrnt->m_fov * 0.5f));
    const float near_ext_y = e * nearPlane;
    const float near_ext_x = near_ext_y * cameraComponrnt->GetResolutionRatio();
    const float far_ext_y = e * farPlane;
    const float far_ext_x = far_ext_y * cameraComponrnt->GetResolutionRatio();

    points[0] = cameraPos + nearCenter - right * near_ext_x - up * near_ext_y;
    points[1] = cameraPos + nearCenter - right * near_ext_x + up * near_ext_y;
    points[2] = cameraPos + nearCenter + right * near_ext_x + up * near_ext_y;
    points[3] = cameraPos + nearCenter + right * near_ext_x - up * near_ext_y;
    points[4] = cameraPos + farCenter - right * far_ext_x - up * far_ext_y;
    points[5] = cameraPos + farCenter - right * far_ext_x + up * far_ext_y;
    points[6] = cameraPos + farCenter + right * far_ext_x + up * far_ext_y;
    points[7] = cameraPos + farCenter + right * far_ext_x - up * far_ext_y;
}

glm::quat Camera::ProcessMouseMovement(float yawAngle, float pitchAngle, bool constrainPitch)
{
    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitchAngle > 89.0f)
            pitchAngle = 89.0f;
        if (pitchAngle < -89.0f)
            pitchAngle = -89.0f;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(yawAngle)) * cos(glm::radians(pitchAngle));
    front.y = sin(glm::radians(pitchAngle));
    front.z = sin(glm::radians(yawAngle)) * cos(glm::radians(pitchAngle));
    front = glm::normalize(front);
    const glm::vec3 right = glm::normalize(glm::cross(
        front, glm::vec3(0.0f, 1.0f, 0.0f))); // Normalize the vectors, because their length gets closer to 0 the more
                                              // you look up or down which results in slower movement.
    const glm::vec3 up = glm::normalize(glm::cross(right, front));
    return glm::quatLookAt(front, up);
}

void Camera::ReverseAngle(const glm::quat &rotation, float &pitchAngle, float &yawAngle, const bool &constrainPitch)
{
    const auto angle = glm::degrees(glm::eulerAngles(rotation));
    pitchAngle = angle.x;
    yawAngle = glm::abs(angle.z) > 90.0f ? 90.0f - angle.y : -90.0f - angle.y;
    if (constrainPitch)
    {
        if (pitchAngle > 89.0f)
            pitchAngle = 89.0f;
        if (pitchAngle < -89.0f)
            pitchAngle = -89.0f;
    }
}

std::shared_ptr<Texture2D> Camera::GetTexture() const
{
    return m_colorTexture;
}

std::unique_ptr<OpenGLUtils::GLTexture2D> &Camera::UnsafeGetGBufferDepth()
{
    return m_gBufferDepth;
}
std::unique_ptr<OpenGLUtils::GLTexture2D> &Camera::UnsafeGetGBufferNormal()
{
    return m_gBufferNormal;
}
std::unique_ptr<OpenGLUtils::GLTexture2D> &Camera::UnsafeGetGBufferAlbedo()
{
    return m_gBufferAlbedo;
}
std::unique_ptr<OpenGLUtils::GLTexture2D> &Camera::UnsafeGetGBufferMetallicRoughnessEmissionAmbient()
{
    return m_gBufferMetallicRoughnessEmissionAmbient;
}

glm::mat4 Camera::GetProjection() const
{
    return glm::perspective(glm::radians(m_fov * 0.5f), GetResolutionRatio(), m_nearDistance, m_farDistance);
}

glm::vec3 Camera::Project(GlobalTransform &ltw, glm::vec3 position)
{
    return m_cameraInfoBlock.m_projection * m_cameraInfoBlock.m_view * glm::vec4(position, 1.0f);
}

glm::vec3 Camera::UnProject(GlobalTransform &ltw, glm::vec3 position) const
{
    glm::mat4 inversed = glm::inverse(m_cameraInfoBlock.m_projection * m_cameraInfoBlock.m_view);
    glm::vec4 start = glm::vec4(position, 1.0f);
    start = inversed * start;
    return start / start.w;
}

glm::vec3 Camera::GetMouseWorldPoint(GlobalTransform &ltw, glm::vec2 mousePosition) const
{
    const float halfX = static_cast<float>(m_resolutionX) / 2.0f;
    const float halfY = static_cast<float>(m_resolutionY) / 2.0f;
    const glm::vec4 start =
        glm::vec4((mousePosition.x - halfX) / halfX, -1 * (mousePosition.y - halfY) / halfY, 0.0f, 1.0f);
    return start / start.w;
}

void Camera::SetClearColor(glm::vec3 color) const
{
    m_frameBuffer->ClearColor(glm::vec4(color.x, color.y, color.z, 0.0f));
    m_frameBuffer->Clear();
    m_frameBuffer->ClearColor(glm::vec4(0.0f));
}

Ray Camera::ScreenPointToRay(GlobalTransform &ltw, glm::vec2 mousePosition) const
{
    const auto position = ltw.GetPosition();
    const auto rotation = ltw.GetRotation();
    const glm::vec3 front = rotation * glm::vec3(0, 0, -1);
    const glm::vec3 up = rotation * glm::vec3(0, 1, 0);
    const auto projection =
        glm::perspective(glm::radians(m_fov * 0.5f), GetResolutionRatio(), m_nearDistance, m_farDistance);
    const auto view = glm::lookAt(position, position + front, up);
    const glm::mat4 inv = glm::inverse(projection * view);
    const float halfX = static_cast<float>(m_resolutionX) / 2.0f;
    const float halfY = static_cast<float>(m_resolutionY) / 2.0f;
    const auto realX = (mousePosition.x + halfX) / halfX;
    const auto realY = (mousePosition.y - halfY) / halfY;
    if (glm::abs(realX) > 1.0f || glm::abs(realY) > 1.0f)
        return {glm::vec3(FLT_MAX), glm::vec3(FLT_MAX)};
    glm::vec4 start = glm::vec4(realX, -1 * realY, -1, 1.0);
    glm::vec4 end = glm::vec4(realX, -1.0f * realY, 1.0f, 1.0f);
    start = inv * start;
    end = inv * end;
    start /= start.w;
    end /= end.w;
    const glm::vec3 dir = glm::normalize(glm::vec3(end - start));
    return {glm::vec3(ltw.m_value[3]) + m_nearDistance * dir, glm::vec3(ltw.m_value[3]) + m_farDistance * dir};
}

void Camera::GenerateMatrices()
{
    m_cameraUniformBufferBlock = std::make_unique<OpenGLUtils::GLUBO>();
    m_cameraUniformBufferBlock->SetData(sizeof(CameraInfoBlock), nullptr, GL_STREAM_DRAW);
    m_cameraUniformBufferBlock->SetBase(0);
}

void Camera::ResizeResolution(int x, int y)
{
    if (m_resolutionX == x && m_resolutionY == y)
        return;
    m_resolutionX = x > 0 ? x : 1;
    m_resolutionY = y > 0 ? y : 1;
    m_gBuffer->SetResolution(m_resolutionX, m_resolutionY);
    m_gBufferDepth->ReSize(0, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 0, m_resolutionX, m_resolutionY);
    m_gBufferNormal->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, m_resolutionX, m_resolutionY);
    m_gBufferAlbedo->ReSize(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0, m_resolutionX, m_resolutionY);
    m_gBufferMetallicRoughnessEmissionAmbient->ReSize(
        0, GL_RGBA16F, GL_RGBA, GL_FLOAT, 0, m_resolutionX, m_resolutionY);

    m_colorTexture->m_texture->ReSize(0, GL_RGBA16F, GL_RGBA, GL_FLOAT, 0, m_resolutionX, m_resolutionY);
    m_depthStencilTexture->m_texture->ReSize(
        0, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0, m_resolutionX, m_resolutionY);
}

void Camera::OnCreate()
{
    m_resolutionX = 1;
    m_resolutionY = 1;

    m_colorTexture = AssetManager::CreateAsset<Texture2D>();
    m_colorTexture->m_name = "CameraTexture";
    m_colorTexture->m_texture =
        std::make_shared<OpenGLUtils::GLTexture2D>(0, GL_RGBA16F, m_resolutionX, m_resolutionY, false);
    m_colorTexture->m_texture->SetData(0, GL_RGBA16F, GL_RGBA, GL_FLOAT, 0);
    m_colorTexture->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_colorTexture->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_colorTexture->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_colorTexture->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    AttachTexture(m_colorTexture->m_texture.get(), GL_COLOR_ATTACHMENT0);

    m_depthStencilTexture = AssetManager::CreateAsset<Texture2D>();
    m_depthStencilTexture->m_texture =
        std::make_shared<OpenGLUtils::GLTexture2D>(0, GL_DEPTH32F_STENCIL8, m_resolutionX, m_resolutionY, false);
    m_depthStencilTexture->m_texture->SetData(0, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
    m_depthStencilTexture->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_depthStencilTexture->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_depthStencilTexture->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthStencilTexture->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    AttachTexture(m_depthStencilTexture->m_texture.get(), GL_DEPTH_STENCIL_ATTACHMENT);

    m_gBuffer = std::make_unique<RenderTarget>(m_resolutionX, m_resolutionY);

    m_gBufferDepth =
        std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_DEPTH_COMPONENT32F, m_resolutionX, m_resolutionY, false);
    m_gBufferDepth->SetData(0, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    m_gBufferDepth->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gBufferDepth->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gBufferDepth->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gBufferDepth->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gBuffer->AttachTexture(m_gBufferDepth.get(), GL_DEPTH_ATTACHMENT);

    m_gBufferNormal = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, m_resolutionX, m_resolutionY, false);
    m_gBufferNormal->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_gBufferNormal->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gBufferNormal->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gBufferNormal->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gBufferNormal->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gBuffer->AttachTexture(m_gBufferNormal.get(), GL_COLOR_ATTACHMENT0);

    m_gBufferAlbedo = std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGB16F, m_resolutionX, m_resolutionY, false);
    m_gBufferAlbedo->SetData(0, GL_RGB16F, GL_RGB, GL_FLOAT, 0);
    m_gBufferAlbedo->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gBufferAlbedo->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gBufferAlbedo->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gBufferAlbedo->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gBuffer->AttachTexture(m_gBufferAlbedo.get(), GL_COLOR_ATTACHMENT1);

    m_gBufferMetallicRoughnessEmissionAmbient =
        std::make_unique<OpenGLUtils::GLTexture2D>(0, GL_RGBA16F, m_resolutionX, m_resolutionY, false);
    m_gBufferMetallicRoughnessEmissionAmbient->SetData(0, GL_RGBA16F, GL_RGBA, GL_FLOAT, 0);
    m_gBufferMetallicRoughnessEmissionAmbient->SetInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gBufferMetallicRoughnessEmissionAmbient->SetInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gBufferMetallicRoughnessEmissionAmbient->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gBufferMetallicRoughnessEmissionAmbient->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gBuffer->AttachTexture(m_gBufferMetallicRoughnessEmissionAmbient.get(), GL_COLOR_ATTACHMENT2);

    SetEnabled(true);
}

void Camera::Serialize(YAML::Emitter &out)
{
    out << YAML::Key << "m_resolutionX" << YAML::Value << m_resolutionX;
    out << YAML::Key << "m_resolutionY" << YAML::Value << m_resolutionY;
    out << YAML::Key << "m_isMainCamera" << YAML::Value << m_isMainCamera;
    out << YAML::Key << "m_useClearColor" << YAML::Value << m_useClearColor;
    out << YAML::Key << "m_clearColor" << YAML::Value << m_clearColor;
    out << YAML::Key << "m_allowAutoResize" << YAML::Value << m_allowAutoResize;
    out << YAML::Key << "m_nearDistance" << YAML::Value << m_nearDistance;
    out << YAML::Key << "m_farDistance" << YAML::Value << m_farDistance;
    out << YAML::Key << "m_fov" << YAML::Value << m_fov;
    m_skybox.Save("m_skybox", out);
}

void Camera::Deserialize(const YAML::Node &in)
{
    int resolutionX = in["m_resolutionX"].as<int>();
    int resolutionY = in["m_resolutionY"].as<int>();
    m_isMainCamera = in["m_isMainCamera"].as<bool>();
    if (m_isMainCamera)
        RenderManager::SetMainCamera(GetOwner().GetOrSetPrivateComponent<Camera>().lock());
    m_useClearColor = in["m_useClearColor"].as<bool>();
    m_clearColor = in["m_clearColor"].as<glm::vec3>();
    m_allowAutoResize = in["m_allowAutoResize"].as<bool>();
    m_nearDistance = in["m_nearDistance"].as<float>();
    m_farDistance = in["m_farDistance"].as<float>();
    m_fov = in["m_fov"].as<float>();
    ResizeResolution(resolutionX, resolutionY);

    m_skybox.Load("m_skybox", in);
}

void Camera::OnDestroy()
{
    if (!RenderManager::GetMainCamera().expired() && RenderManager::GetMainCamera().lock().get() == this)
    {
        RenderManager::SetMainCamera(nullptr);
    }
}

void Camera::OnInspect()
{
    ImGui::Checkbox("Allow auto resize", &m_allowAutoResize);
    if (!m_allowAutoResize)
    {
        glm::ivec2 resolution = {m_resolutionX, m_resolutionY};
        if (ImGui::DragInt2("Resolution", &resolution.x))
        {
            ResizeResolution(resolution.x, resolution.y);
        }
    }
    ImGui::Checkbox("Use clear color", &m_useClearColor);
    const bool savedState = m_isMainCamera;
    ImGui::Checkbox("Main Camera", &m_isMainCamera);
    if (savedState != m_isMainCamera)
    {
        if (m_isMainCamera)
        {
            RenderManager::SetMainCamera(GetOwner().GetOrSetPrivateComponent<Camera>().lock());
        }
        else
        {
            RenderManager::SetMainCamera(nullptr);
        }
    }
    if (m_useClearColor)
    {
        ImGui::ColorEdit3("Clear Color", (float *)(void *)&m_clearColor);
    }
    else
    {
        EditorManager::DragAndDropButton<Cubemap>(m_skybox, "Skybox");
    }

    ImGui::DragFloat("Near", &m_nearDistance, m_nearDistance / 10.0f, 0, m_farDistance);
    ImGui::DragFloat("Far", &m_farDistance, m_farDistance / 10.0f, m_nearDistance);
    ImGui::DragFloat("FOV", &m_fov, 1.0f, 1, 359);

    FileUtils::SaveFile("Screenshot", "Texture2D", {".png", ".jpg"}, [this](const std::filesystem::path &filePath) {
        m_colorTexture->Save(filePath);
    });

    if (ImGui::TreeNode("Debug"))
    {
        static float debugSacle = 0.25f;
        ImGui::DragFloat("Scale", &debugSacle, 0.01f, 0.1f, 1.0f);
        debugSacle = glm::clamp(debugSacle, 0.1f, 1.0f);
        ImGui::Image(
            (ImTextureID)m_colorTexture->UnsafeGetGLTexture()->Id(),
            ImVec2(m_resolutionX * debugSacle, m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::Image(
            (ImTextureID)m_gBufferNormal->Id(),
            ImVec2(m_resolutionX * debugSacle, m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::Image(
            (ImTextureID)m_gBufferAlbedo->Id(),
            ImVec2(m_resolutionX * debugSacle, m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::Image(
            (ImTextureID)m_gBufferMetallicRoughnessEmissionAmbient->Id(),
            ImVec2(m_resolutionX * debugSacle, m_resolutionY * debugSacle),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::TreePop();
    }
}

void Camera::Clone(const std::shared_ptr<IPrivateComponent> &target)
{
    auto ptr = std::static_pointer_cast<Camera>(target);
    m_allowAutoResize = ptr->m_allowAutoResize;
    m_nearDistance = ptr->m_nearDistance;
    m_farDistance = ptr->m_farDistance;
    m_fov = ptr->m_fov;
    m_useClearColor = ptr->m_useClearColor;
    m_clearColor = ptr->m_clearColor;
    m_skybox = ptr->m_skybox;
}
void Camera::Start()
{
    if (m_isMainCamera)
        RenderManager::SetMainCamera(GetOwner().GetOrSetPrivateComponent<Camera>().lock());
}
void Camera::CollectAssetRef(std::vector<AssetRef> &list)
{
    list.push_back(m_skybox);
}
std::shared_ptr<Texture2D> Camera::GetDepthStencil() const
{
    return m_depthStencilTexture;
}

void CameraInfoBlock::UpdateMatrices(const std::shared_ptr<Camera> &camera, glm::vec3 position, glm::quat rotation)
{
    const glm::vec3 front = rotation * glm::vec3(0, 0, -1);
    const glm::vec3 up = rotation * glm::vec3(0, 1, 0);
    const auto ratio = camera->GetResolutionRatio();
    m_projection =
        glm::perspective(glm::radians(camera->m_fov * 0.5f), ratio, camera->m_nearDistance, camera->m_farDistance);
    m_position = glm::vec4(position, 0);
    m_view = glm::lookAt(position, position + front, up);
    m_inverseProjection = glm::inverse(m_projection);
    m_inverseView = glm::inverse(m_view);
    m_reservedParameters = glm::vec4(
        camera->m_nearDistance,
        camera->m_farDistance,
        glm::tan(glm::radians(camera->m_fov * 0.5f)),
        static_cast<float>(camera->m_resolutionX) / camera->m_resolutionY);
}

void CameraInfoBlock::UploadMatrices(const std::shared_ptr<Camera> &camera) const
{
    Camera::m_cameraUniformBufferBlock->SubData(0, sizeof(CameraInfoBlock), this);
}
