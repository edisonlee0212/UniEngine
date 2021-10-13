#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <random>

// OpenGL and Vulkan
//#include <volk/volk.h>
#include <KHR/khrplatform.h>
#include <glad/glad.h>

#include <Math.hpp>

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Gui.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <tiny_obj_loader.hpp>

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

#include <yaml-cpp/yaml.h>

#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_DEFINED
#endif
#ifndef NDEBUG
#define NDEBUG
#define NDEBUG_WAS_NOT_DEFINED
#endif
#include <PxPhysicsAPI.h>
#ifdef DEBUG_WAS_DEFINED
#undef DEBUG_WAS_DEFINED
#define _DEBUG
#endif

#ifdef NDEBUG_WAS_NOT_DEFINED
#undef NDEBUG_WAS_NOT_DEFINED
#undef NDEBUG
#endif