cmake_minimum_required(VERSION 3.17)

project(
        UniEngine
        VERSION 0.1
        DESCRIPTION "An rendering Engine"
)
include(GenerateExportHeader)

option(UNIENGINE_BUILD_EXAMPLES "Build UniEngine Examples" OFF)

# Set a default build type if none was specified
set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release")
endif ()

# ------------------------------------------------------------------
# 3rd Party libraries
# ------------------------------------------------------------------
# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(${3RD_PARTY_DIR})

# ------------------------------------------------------------------
# UniEngine
# ------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
file(GLOB UNIENGINE_SOURCES_LOCAL "src/lib/*.cpp")

add_library(uniengine
        SHARED
        ${IMGUI_SOURCES}
        ${UNIENGINE_SOURCES_LOCAL}
        ${GLAD_SOURCE}
        )
set(UNIENGINE_PCH_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/UniEngine-pch.hpp)
target_precompile_headers(uniengine
        PRIVATE
        ${UNIENGINE_PCH_LOCAL}
        )
generate_export_header(uniengine
        BASE_NAME UNIENGINE
        EXPORT_MACRO_NAME UNIENGINE_API
        )

# Linker settings for all platforms
target_link_libraries(uniengine
        #volk::volk
        yaml-cpp
        )

set(UNIENGINE_INCLUDES_LOCAL
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/Animation
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/ECS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/Editor
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/Physics
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/Rendering
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Engine/Resources
        ${CMAKE_CURRENT_SOURCE_DIR}/include/lib/External
        ${3RD_PARTY_DIR}
        ${GLFW_DIR}/include
        ${ASSIMP_DIR}/include
        ${STB_DIR}
        ${IMGUI_DIR}
        ${GLAD_DIR}
        ${TINYOBJLOADER_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${TINYOBJLOADER_DIR}
        ${PHYSX_DIR}/include
        )

target_include_directories(uniengine
        PRIVATE
        ${UNIENGINE_INCLUDES_LOCAL}
        )

set(UNIENGINE_RESOURCE_DIR_LOCAL ${CMAKE_CURRENT_SOURCE_DIR}/Resources/)
set(UNIENGINE_RESOURCE_DEF UNIENGINE_RESOURCE_FOLDER=\"${UNIENGINE_RESOURCE_DIR_LOCAL}\")
target_compile_definitions(uniengine
        PRIVATE
        ${UNIENGINE_RESOURCE_DEF}
        GLAD_GLAPI_EXPORT
        GLAD_GLAPI_EXPORT_BUILD
        )
if (WIN32)
    # Linker settings for Windows platform
    if (${CMAKE_BUILD_TYPE} STREQUAL Release)
        file(GLOB PHYSX_LIBS "3rdParty/physx/win.x86_64.vc142.mt/release/*.lib")
        set(ASSIMP_LIBS
                ${ASSIMP_DIR}assimp-vc142-mt.lib
                ${ASSIMP_DIR}contrib/zlib/zlibstatic.lib
                )
    elseif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        file(GLOB PHYSX_LIBS "3rdParty/physx/win.x86_64.vc142.mt/debug/*.lib")
        set(ASSIMP_LIBS
                ${ASSIMP_DIR}assimp-vc142-mtd.lib
                ${ASSIMP_DIR}contrib/zlib/zlibstaticd.lib
                )
    endif ()
    target_link_libraries(uniengine
            opengl32
            ${GLFW_DIR}glfw3.lib
            ${PHYSX_LIBS}
            ${ASSIMP_LIBS}
            )
    # Compiler settings for Windows platform
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    target_compile_definitions(uniengine
            PRIVATE
            NOMINMAX
            UNIENGINE_EXPORTS
            "IMGUI_API=__declspec(dllexport)"
            USE_ASSIMP
            )
elseif (UNIX AND NOT APPLE)
    # Linker and compiler settings for Linux platform
    if (${CMAKE_BUILD_TYPE} STREQUAL Release)
        file(GLOB PHYSX_LIBS "3rdParty/physx/linux.clang/release/*.so")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    elseif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        file(GLOB PHYSX_LIBS "3rdParty/physx/linux.clang/debug/*.so")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
    endif ()
    target_link_libraries(uniengine
            GL
            glfw
            dl
            X11
            pthread
            Xrandr
            Xi
            dl
            assimp
            ${PHYSX_LIBS}
            )
elseif (APPLE)
endif ()

# Compiler-specific compile flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    message(STATUS "Setting G++ flags")
    # G++
    target_compile_options(uniengine PRIVATE -Wall -Wextra)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    message(STATUS "Setting MSVC flags")
    # MSVC
    target_compile_options(uniengine PRIVATE /EHsc /W2 /c)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    message(STATUS "Setting Clang flags")
    # Activate all clang warnings except those that don't really affect much
    #target_compile_options(uniengine PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-newline-eof -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors -Wno-constant-conversion)
endif ()

# ------------------------------------------------------------------
# Example program
# ------------------------------------------------------------------

if (${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_BINARY_DIR} OR UNIENGINE_BUILD_EXAMPLES)
    file(GLOB SAMPLE_CORE_SOURCES_LOCAL "src/core/*.cpp")
    file(GLOB PLANET_SOURCES_LOCAL "src/core/Planet/*.cpp")
    file(GLOB STAR_CLUSTER_SOURCES_LOCAL "src/core/StarCluster/*.cpp")
    add_library(examples-core
            ${SAMPLE_CORE_SOURCES_LOCAL}
            ${PLANET_SOURCES_LOCAL}
            ${STAR_CLUSTER_SOURCES_LOCAL}
            )
    target_precompile_headers(examples-core
            PRIVATE
            ${UNIENGINE_PCH_LOCAL}
            )
    target_include_directories(examples-core
            PUBLIC
            ${UNIENGINE_INCLUDES_LOCAL}
            ${CMAKE_CURRENT_SOURCE_DIR}/include/core
            )
    target_link_libraries(examples-core
            uniengine
            )

    add_executable(Rendering
            src/app/Rendering.cpp
            )
    target_include_directories(Rendering
            PRIVATE
            ${UNIENGINE_INCLUDES_LOCAL}
            )
    target_precompile_headers(Rendering
            PRIVATE
            ${UNIENGINE_PCH_LOCAL}
            )
    target_link_libraries(Rendering
            uniengine
            examples-core
            )
    target_compile_definitions(Rendering
            PRIVATE
            USE_ASSIMP
            )
    add_executable(Physics
            src/app/Physics.cpp
            )
    target_include_directories(Physics
            PRIVATE
            ${UNIENGINE_INCLUDES_LOCAL}
            )
    target_precompile_headers(Physics
            PRIVATE
            ${UNIENGINE_PCH_LOCAL}
            )
    target_link_libraries(Physics
            uniengine
            examples-core
            )

    add_executable(Planet
            src/app/Planet.cpp
            )
    target_include_directories(Planet
            PRIVATE
            ${UNIENGINE_INCLUDES_LOCAL}
            )
    target_precompile_headers(Planet
            PRIVATE
            ${UNIENGINE_PCH_LOCAL}
            )
    target_link_libraries(Planet
            uniengine
            examples-core
            )

    add_executable(StarCluster
            src/app/StarCluster.cpp
            )
    target_include_directories(StarCluster
            PRIVATE
            ${UNIENGINE_INCLUDES_LOCAL}
            )
    target_precompile_headers(StarCluster
            PRIVATE
            ${UNIENGINE_PCH_LOCAL}
            )
    target_link_libraries(StarCluster
            uniengine
            examples-core
            )

    file(COPY src/app/imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()

# ------------------------------------------------------------------
# Copy DLLs
# ------------------------------------------------------------------
if (WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL Release)
        file(GLOB PHYSX_DLLS "3rdParty/physx/win.x86_64.vc142.mt/release/*.dll")
        add_custom_command(
                TARGET uniengine
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${3RD_PARTY_DIR}/assimp/assimp-vc142-mt.dll
                ${CMAKE_BINARY_DIR}
        )
        foreach (PHYSX_DLL ${PHYSX_DLLS})
            add_custom_command(
                    TARGET uniengine
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${PHYSX_DLL}
                    ${CMAKE_BINARY_DIR}
            )
        endforeach ()
    elseif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        file(GLOB PHYSX_DLLS "3rdParty/physx/win.x86_64.vc142.mt/debug/*.dll")
        add_custom_command(
                TARGET uniengine
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${3RD_PARTY_DIR}/assimp/assimp-vc142-mtd.dll
                ${CMAKE_BINARY_DIR}
        )
        foreach (PHYSX_DLL ${PHYSX_DLLS})
            add_custom_command(
                    TARGET uniengine
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${PHYSX_DLL}
                    ${CMAKE_BINARY_DIR}
            )
        endforeach ()
    endif ()
endif ()
if (NOT ${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    if (WIN32)
        add_custom_command(
                TARGET uniengine
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_BINARY_DIR}/uniengine.dll
                ${CMAKE_BINARY_DIR}
        )
    endif ()
    set(UNIENGINE_INCLUDES ${UNIENGINE_INCLUDES_LOCAL} PARENT_SCOPE)
    set(UNIENGINE_PCH ${UNIENGINE_PCH_LOCAL} PARENT_SCOPE)
endif ()