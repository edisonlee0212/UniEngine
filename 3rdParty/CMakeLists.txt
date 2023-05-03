# ------------------------------------------------------------------
# assimp
# ------------------------------------------------------------------
set(ASSIMP_DIR_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include)
if (WIN32)
else ()
    find_package(ASSIMP REQUIRED)
    message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")
endif ()
set(ASSIMP_DIR ${3RD_PARTY_DIR}/assimp/ PARENT_SCOPE)

# ------------------------------------------------------------------
# glfw
# ------------------------------------------------------------------
set(GLFW_DIR_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include)
if (WIN32)
else ()
    find_package(GLFW3 REQUIRED)
    message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
endif ()
set(GLFW_DIR ${3RD_PARTY_DIR}/glfw/ PARENT_SCOPE)

# ------------------------------------------------------------------
# tinyobjloader
# ------------------------------------------------------------------
set(TINYOBJLOADER_DIR ${3RD_PARTY_DIR}/tinyobjloader/ PARENT_SCOPE)

# ------------------------------------------------------------------
# yaml
# ------------------------------------------------------------------
set(YAML_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/)
include_directories(${YAML_CPP_DIR}/include)
add_subdirectory(${YAML_CPP_DIR} EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------
# glm
# ------------------------------------------------------------------
if (WIN32)
    set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm/)
    include_directories(${GLM_DIR})
    add_subdirectory(${GLM_DIR} EXCLUDE_FROM_ALL)
elseif (UNIX AND NOT APPLE)
    find_package(GLM REQUIRED)
    message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
endif ()
# ------------------------------------------------------------------
# volk
# ------------------------------------------------------------------
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_NO_PROTOTYPES VK_USE_PLATFORM_WIN32_KHR)
elseif (UNIX AND NOT APPLE)
    set(VOLK_STATIC_DEFINES VK_NO_PROTOTYPES VK_USE_PLATFORM_XLIB_KHR)
elseif (APPLE)
    set(VOLK_STATIC_DEFINES VK_NO_PROTOTYPES VK_USE_PLATFORM_MACOS_MVK)
endif ()
set(VOLK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/volk/)
#add_subdirectory(${VOLK_DIR} EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------
# stb
# ------------------------------------------------------------------
set(STB_DIR ${3RD_PARTY_DIR}/stb/ PARENT_SCOPE)

# ------------------------------------------------------------------
# glad
# ------------------------------------------------------------------
set(GLAD_DIR_LOCAL
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/KHR
        )
set(GLAD_SOURCE ${3RD_PARTY_DIR}/glad/src/glad.cpp PARENT_SCOPE)
set(GLAD_DIR
        ${3RD_PARTY_DIR}/glad/include
        ${3RD_PARTY_DIR}/glad/include/glad
        ${3RD_PARTY_DIR}/glad/include/KHR
        PARENT_SCOPE)

# ------------------------------------------------------------------
# ImGui & ImGuizmo
# ------------------------------------------------------------------
set(IMGUI_DIR_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/ImGui)
set(IMGUIZMO_DIR_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)

if (WIN32)
    set(IMGUI_BACKENDS ${IMGUI_DIR_LOCAL}/backends/imgui_impl_win32.cpp ${IMGUI_DIR_LOCAL}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR_LOCAL}/backends/imgui_impl_opengl3.cpp)
elseif (UNIX AND NOT APPLE)
    set(IMGUI_BACKENDS ${IMGUI_DIR_LOCAL}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR_LOCAL}/backends/imgui_impl_opengl3.cpp)
elseif (APPLE)
endif ()
file(GLOB IMGUI_SOURCES_LOCAL ${IMGUI_DIR_LOCAL}/*.cpp)
file(GLOB IMGUIZMO_SOURCES_LOCAL ${IMGUIZMO_DIR_LOCAL}/*.cpp)
set(IMGUI_SOURCES
        ${IMGUI_SOURCES_LOCAL}
        ${IMGUI_BACKENDS}
        ${IMGUIZMO_SOURCES_LOCAL}
        PARENT_SCOPE
        )
set(IMGUI_DIR
        ${3RD_PARTY_DIR}/ImGui
        ${3RD_PARTY_DIR}/ImGui/backends
        ${3RD_PARTY_DIR}/ImGuizmo
        PARENT_SCOPE)

# ------------------------------------------------------------------
# physx
# ------------------------------------------------------------------
set(PHYSX_DIR
        ${3RD_PARTY_DIR}/physx
        PARENT_SCOPE)


# ------------------------------------------------------------------
# Includes
# ------------------------------------------------------------------
set(3RD_PARTY_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
        ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGui
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad
        ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/KHR
        ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader
        ${CMAKE_CURRENT_SOURCE_DIR}/physx/include
        PARENT_SCOPE
        )