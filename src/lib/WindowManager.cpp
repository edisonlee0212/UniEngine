#include <Application.hpp>
#include <ConsoleManager.hpp>
#include <DefaultResources.hpp>
#include <ProfilerManager.hpp>
#include <ProjectManager.hpp>
#include <RenderTarget.hpp>
#include <WindowManager.hpp>
using namespace UniEngine;

void WindowManager::ResizeCallback(GLFWwindow *window, int width, int height)
{
    GetInstance().m_windowWidth = width;
    GetInstance().m_windowHeight = height;
}

void WindowManager::SetMonitorCallback(GLFWmonitor *monitor, int event)
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

void WindowManager::LateUpdate()
{
    glfwSwapBuffers(GetInstance().m_window);
}

void WindowManager::Init(std::string name, bool fullScreen)
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
    GetInstance().m_windowWidth = 1280;
    GetInstance().m_windowHeight = 720;

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

GLFWwindow *WindowManager::GetWindow()
{
    return GetInstance().m_window;
}

GLFWmonitor *WindowManager::PrimaryMonitor()
{
    return GetInstance().m_primaryMonitor;
}

void WindowManager::PreUpdate()
{
    glfwPollEvents();
    RenderTarget::BindDefault();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (glfwWindowShouldClose(WindowManager::GetWindow()))
    {
        Application::GetInstance().m_applicationStatus = ApplicationStatus::OnDestroy;
    }
}

void WindowManager::DrawTexture(OpenGLUtils::GLTexture2D *texture)
{
    RenderTarget::BindDefault();
    /* Make the window's context current */
    glViewport(0, 0, GetInstance().m_windowWidth, GetInstance().m_windowHeight);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    /* Render here */
    glDisable(GL_DEPTH_TEST);
    glDrawBuffer(GL_BACK);
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
void WindowManager::WindowFocusCallback(GLFWwindow* window, int focused)
{
    if (focused)
    {
        ProjectManager::ScanProjectFolder();
    }
}
