#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#define STBI_MSC_SECURE_CRT
// define something for Windows (32-bit and 64-bit, this part is common)
#include <imgui_impl_glfw.h>
//#include <backends/imgui_impl_vulkan.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#else
// linux
#include <imgui_impl_glfw.h>
//#include <imgui_impl_vulkan.h>
#include <imgui_impl_opengl3.h>
#endif