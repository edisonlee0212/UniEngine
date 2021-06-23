#include "DefaultResources.hpp"
#include "Texture2D.hpp"

#include <RenderTarget.hpp>
#include <EnvironmentalMap.hpp>

using namespace UniEngine;
glm::mat4 EnvironmentalMapCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 EnvironmentalMapCaptureViews[] = {
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

void EnvironmentalMap::PrepareIrradianceMap()
{
	size_t resolution = m_irradianceMapResolution;
	auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
	auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
	renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
	renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);

	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	auto irradianceMap = std::make_unique<OpenGLUtils::GLTextureCubeMap>(1, GL_RGB32F, resolution, resolution, true);
    m_irradianceMap = std::make_unique<Cubemap>();
	m_irradianceMap->m_texture = std::move(irradianceMap);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_irradianceMap->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	DefaultResources::GLPrograms::ConvolutionProgram->Bind(); 
	DefaultResources::GLPrograms::ConvolutionProgram->SetInt("environmentMap", 0);
	DefaultResources::GLPrograms::ConvolutionProgram->SetFloat4x4("projection", EnvironmentalMapCaptureProjection);
    m_targetCubemap->m_texture->Bind(0);
	renderTarget->GetFrameBuffer()->ViewPort(
		resolution, resolution); // don't forget to configure the viewport to the capture dimensions.
	for (unsigned int i = 0; i < 6; ++i)
	{
		DefaultResources::GLPrograms::ConvolutionProgram->SetFloat4x4("view", EnvironmentalMapCaptureViews[i]);
		renderTarget->AttachTexture2D(m_irradianceMap->Texture().get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
		renderTarget->Clear();
		RenderCube();
	}
	OpenGLUtils::GLFrameBuffer::BindDefault();
}

void EnvironmentalMap::PreparePreFilteredMap()
{
	size_t resolution = m_preFilteredMapResolution;
	auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
	
	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------
	GLsizei mipmap = static_cast<GLsizei>(log2((glm::max)(resolution, resolution))) + 1;
	auto preFilteredMap = std::make_unique<OpenGLUtils::GLTextureCubeMap>(mipmap, GL_RGB32F, resolution, resolution, true);
    m_preFilteredMap = std::make_unique<Cubemap>();
	m_preFilteredMap->m_texture = std::move(preFilteredMap);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_preFilteredMap->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_preFilteredMap->m_texture->GenerateMipMap();

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
    DefaultResources::GLPrograms::PrefilterProgram->Bind();
    DefaultResources::GLPrograms::PrefilterProgram->SetInt("environmentMap", 0);
    DefaultResources::GLPrograms::PrefilterProgram->SetFloat4x4("projection", EnvironmentalMapCaptureProjection);
    
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
        unsigned int mipWidth = resolution * std::pow(0.5, mip);
        auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
        renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, mipWidth, mipWidth);
        renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
        m_targetCubemap->Texture()->Bind(0);
        renderTarget->GetFrameBuffer()->ViewPort(
            mipWidth, mipWidth); // don't forget to configure the viewport to the capture dimensions.
		float roughness = (float)mip / (float)(maxMipLevels - 1);
        DefaultResources::GLPrograms::PrefilterProgram->SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
            DefaultResources::GLPrograms::PrefilterProgram->SetFloat4x4("view", EnvironmentalMapCaptureViews[i]);
            renderTarget->AttachTexture2D(
                m_preFilteredMap->m_texture.get(), GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip);
            renderTarget->Clear();
            RenderCube();
		}
	}
    OpenGLUtils::GLFrameBuffer::BindDefault();
}

void EnvironmentalMap::PrepareBrdfLut()
{
	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
    auto brdfLut = std::make_shared<OpenGLUtils::GLTexture2D>(1, GL_RG16F, 512, 512, true);
    m_brdfLut = std::make_unique<Texture2D>();
    m_brdfLut->m_texture = std::move(brdfLut);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_brdfLut->m_texture->SetInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    size_t resolution = 512;
    auto renderTarget = std::make_unique<RenderTarget>(resolution, resolution);
    auto renderBuffer = std::make_unique<OpenGLUtils::GLRenderBuffer>();
    renderBuffer->AllocateStorage(GL_DEPTH_COMPONENT24, resolution, resolution);
    renderTarget->AttachRenderBuffer(renderBuffer.get(), GL_DEPTH_ATTACHMENT);
    renderTarget->AttachTexture(m_brdfLut->m_texture.get(), GL_COLOR_ATTACHMENT0);
    renderTarget->GetFrameBuffer()->ViewPort(resolution, resolution);
    DefaultResources::GLPrograms::BrdfProgram->Bind();
    renderTarget->Clear();
    RenderQuad();
    OpenGLUtils::GLFrameBuffer::BindDefault();
}

unsigned int environmentalMapCubeVAO = 0;
unsigned int environmentalMapCubeVBO = 0;
void EnvironmentalMap::RenderCube()
{
	// initialize (if necessary)
	if (environmentalMapCubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			0.0f,
			0.0f, // bottom-left
			1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			1.0f,
			1.0f, // top-right
			1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			1.0f,
			0.0f, // bottom-right
			1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			1.0f,
			1.0f, // top-right
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			0.0f,
			0.0f, // bottom-left
			-1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			-1.0f,
			0.0f,
			1.0f, // top-left
			// front face
			-1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f, // bottom-left
			1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			0.0f, // bottom-right
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			1.0f, // top-right
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			1.0f, // top-right
			-1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f, // top-left
			-1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f, // bottom-left
			// left face
			-1.0f,
			1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f, // top-right
			-1.0f,
			1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f, // top-left
			-1.0f,
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f, // bottom-left
			-1.0f,
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f, // bottom-left
			-1.0f,
			-1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f, // bottom-right
			-1.0f,
			1.0f,
			1.0f,
			-1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f, // top-right
				  // right face
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f, // top-left
			1.0f,
			-1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f, // bottom-right
			1.0f,
			1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f, // top-right
			1.0f,
			-1.0f,
			-1.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			1.0f, // bottom-right
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f,
			0.0f, // top-left
			1.0f,
			-1.0f,
			1.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f,
			0.0f, // bottom-left
			// bottom face
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			-1.0f,
			0.0f,
			0.0f,
			1.0f, // top-right
			1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			-1.0f,
			0.0f,
			1.0f,
			1.0f, // top-left
			1.0f,
			-1.0f,
			1.0f,
			0.0f,
			-1.0f,
			0.0f,
			1.0f,
			0.0f, // bottom-left
			1.0f,
			-1.0f,
			1.0f,
			0.0f,
			-1.0f,
			0.0f,
			1.0f,
			0.0f, // bottom-left
			-1.0f,
			-1.0f,
			1.0f,
			0.0f,
			-1.0f,
			0.0f,
			0.0f,
			0.0f, // bottom-right
			-1.0f,
			-1.0f,
			-1.0f,
			0.0f,
			-1.0f,
			0.0f,
			0.0f,
			1.0f, // top-right
			// top face
			-1.0f,
			1.0f,
			-1.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f, // top-left
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f, // bottom-right
			1.0f,
			1.0f,
			-1.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			1.0f, // top-right
			1.0f,
			1.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f, // bottom-right
			-1.0f,
			1.0f,
			-1.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			1.0f, // top-left
			-1.0f,
			1.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
			0.0f,
			0.0f // bottom-left
		};
		glGenVertexArrays(1, &environmentalMapCubeVAO);
		glGenBuffers(1, &environmentalMapCubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, environmentalMapCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(environmentalMapCubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(environmentalMapCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	OpenGLUtils::GLVAO::BindDefault();
}
unsigned int environmentalMapQuadVAO = 0;
unsigned int environmentalMapQuadVBO;
void EnvironmentalMap::RenderQuad()
{
    if (environmentalMapQuadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &environmentalMapQuadVAO);
        glGenBuffers(1, &environmentalMapQuadVBO);
        glBindVertexArray(environmentalMapQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, environmentalMapQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(environmentalMapQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void EnvironmentalMap::ConstructFromCubemap(const std::shared_ptr<Cubemap> &cubemap)
{
	m_targetCubemap = cubemap;
	PrepareIrradianceMap();
	PreparePreFilteredMap();
	PrepareBrdfLut();
    m_ready = true;
}
