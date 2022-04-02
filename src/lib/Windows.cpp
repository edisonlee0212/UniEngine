#include <Application.hpp>
#include "Engine/Utilities/Console.hpp"
#include <DefaultResources.hpp>
#include <ProfilerLayer.hpp>
#include <ProjectManager.hpp>
#include <RenderTarget.hpp>
#include "Engine/Core/Windows.hpp"
using namespace UniEngine;

void Windows::ResizeCallback(GLFWwindow *window, int width, int height)
{
    GetInstance().m_windowWidth = width;
    GetInstance().m_windowHeight = height;
}

void Windows::SetMonitorCallback(GLFWmonitor *monitor, int event)
{
    if (event == GLFW_CONNECTED)
    {
        // The monitor was connected
        for (auto i : GetInstance().m_monitors)
            if (i == monitor)
                return;
        GetInstance().m_monitors.push_back(monitor);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        // The monitor was disconnected
        for (auto i = 0; i < GetInstance().m_monitors.size(); i++)
        {
            if (monitor == GetInstance().m_monitors[i])
            {
                GetInstance().m_monitors.erase(GetInstance().m_monitors.begin() + i);
            }
        }
    }
    GetInstance().m_primaryMonitor = glfwGetPrimaryMonitor();
}

void Windows::LateUpdate()
{
    auto& windowManager = GetInstance();
    glfwSwapBuffers(windowManager.m_window);
}

void Windows::Init(std::string name, bool fullScreen)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    int size;
    auto monitors = glfwGetMonitors(&size);
    for (auto i = 0; i < size; i++)
    {
        GetInstance().m_monitors.push_back(monitors[i]);
    }
    GetInstance().m_primaryMonitor = glfwGetPrimaryMonitor();
    glfwSetMonitorCallback(SetMonitorCallback);

    // glfw window creation
    // --------------------
    // const GLFWvidmode *mode = glfwGetVideoMode(GetInstance().m_primaryMonitor);
    GetInstance().m_windowWidth = 250;
    GetInstance().m_windowHeight = 50;

    GetInstance().m_window =
        glfwCreateWindow(GetInstance().m_windowWidth, GetInstance().m_windowHeight, name.c_str(), NULL, NULL);
    if (fullScreen)
        glfwMaximizeWindow(GetInstance().m_window);
    glfwSetFramebufferSizeCallback(GetInstance().m_window, ResizeCallback);
    glfwSetWindowFocusCallback(GetInstance().m_window, WindowFocusCallback);
    if (GetInstance().m_window == NULL)
    {
        UNIENGINE_ERROR("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(GetInstance().m_window);
}

GLFWwindow *Windows::GetWindow()
{
    return GetInstance().m_window;
}

GLFWmonitor *Windows::PrimaryMonitor()
{
    return GetInstance().m_primaryMonitor;
}

void Windows::PreUpdate()
{
    glfwPollEvents();
    RenderTarget::BindDefault();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    if (glfwWindowShouldClose(Windows::GetWindow()))
    {
        Application::GetInstance().m_applicationStatus = ApplicationStatus::OnDestroy;
    }
}

void Windows::DrawTexture(OpenGLUtils::GLTexture2D *texture)
{
    RenderTarget::BindDefault();
    /* Make the window's context current */
    OpenGLUtils::SetViewPort(GetInstance().m_windowWidth, GetInstance().m_windowHeight);
    OpenGLUtils::SetPolygonMode(OpenGLPolygonMode::Fill);
    /* Render here */
    OpenGLUtils::SetEnable(OpenGLCapability::DepthTest, false);
    OpenGLUtils::GLFrameBuffer::DefaultFrameBufferDrawBuffer(GL_BACK);
    auto program = DefaultResources::ScreenProgram;
    program->Bind();
    program->SetFloat("depth", 0);
    DefaultResources::ScreenVAO->Bind();
    // Default::Textures::UV->Texture()->Bind(GL_TEXTURE_2D);
    texture->Bind(0);
    program->SetInt("screenTexture", 0);
    program->SetFloat2("center", glm::vec2(0));
    program->SetFloat2("size", glm::vec2(1.0));
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void Windows::WindowFocusCallback(GLFWwindow* window, int focused)
{
    if (focused)
    {
        ProjectManager::ScanProject();
    }
}
void Windows::ResizeWindow(int x, int y)
{
    glfwSetWindowSize(GetWindow(), x, y);
}
