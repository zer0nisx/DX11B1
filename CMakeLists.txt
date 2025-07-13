cmake_minimum_required(VERSION 3.16)
project(DX11GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific configurations
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Set vcpkg paths
set(CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")

# Source files organization
# Explicitly list source files to ensure all are included in build
set(CORE_SOURCES
    "Source/Core/ConfigManager.cpp"
    "Source/Core/ConfigManager.h"
    "Source/Core/Engine.cpp"
    "Source/Core/Engine.h"
    "Source/Core/FileSystem.cpp"
    "Source/Core/FileSystem.h"
    "Source/Core/Logger.cpp"
    "Source/Core/Logger.h"
    "Source/Core/SettingsInterface.cpp"
    "Source/Core/SettingsInterface.h"
    "Source/Core/Timer.cpp"
    "Source/Core/Timer.h"
    "Source/Core/Window.cpp"
    "Source/Core/Window.h"
    "Source/Core/XmlManager.cpp"
    "Source/Core/XmlManager.h"
)

file(GLOB_RECURSE RENDERER_SOURCES "Source/Renderer/*.cpp" "Source/Renderer/*.h")
file(GLOB_RECURSE MESH_SOURCES "Source/Mesh/*.cpp" "Source/Mesh/*.h")
file(GLOB_RECURSE ANIMATION_SOURCES "Source/Animation/*.cpp" "Source/Animation/*.h")
file(GLOB_RECURSE SCENE_SOURCES "Source/Scene/*.cpp" "Source/Scene/*.h")
file(GLOB_RECURSE INPUT_SOURCES "Source/Input/*.cpp" "Source/Input/*.h")
file(GLOB_RECURSE MATH_SOURCES "Source/Math/*.h")
set(MAIN_SOURCES "Source/main.cpp")

set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${RENDERER_SOURCES}
    ${MESH_SOURCES}
    ${ANIMATION_SOURCES}
    ${SCENE_SOURCES}
    ${INPUT_SOURCES}
    ${MATH_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Find packages
find_package(directxtk CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Microsoft::DirectXTK
    assimp::assimp
)

# DirectX libraries (Windows only)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        dinput8.lib
        dxguid.lib
        user32.lib
        gdi32.lib
        winmm.lib
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "Source"
    "Source/Core"
    "Source/Renderer"
    "Source/Mesh"
    "Source/Animation"
    "Source/Scene"
    "Source/Input"
    "Source/Math"
    "Dependencies/rapidxml"
)

# Build configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Debug"
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
    )
endif()

# Copy assets and shaders
file(GLOB_RECURSE SHADERS "Shaders/*.hlsl")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
)

# Visual Studio specific configurations
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Organize files in Solution Explorer
    source_group("Core" FILES ${CORE_SOURCES})
    source_group("Renderer" FILES ${RENDERER_SOURCES})
    source_group("Mesh" FILES ${MESH_SOURCES})
    source_group("Animation" FILES ${ANIMATION_SOURCES})
    source_group("Scene" FILES ${SCENE_SOURCES})
    source_group("Input" FILES ${INPUT_SOURCES})
    source_group("Math" FILES ${MATH_SOURCES})
    source_group("Shaders" FILES ${SHADERS})
endif()
