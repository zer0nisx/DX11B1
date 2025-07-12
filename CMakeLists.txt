cmake_minimum_required(VERSION 3.16)
project(DX11GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuración para Windows
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Buscar paquetes
find_package(PkgConfig REQUIRED)

# Configurar Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# Incluir Assimp como subdirectorio (requiere clonar assimp en Dependencies/)
# O usar find_package si está instalado en el sistema
if(EXISTS "${CMAKE_SOURCE_DIR}/Dependencies/assimp/CMakeLists.txt")
    add_subdirectory(Dependencies/assimp)
    set(ASSIMP_FOUND TRUE)
    set(ASSIMP_LIBRARIES assimp)
    set(ASSIMP_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Dependencies/assimp/include")
else()
    # Intentar encontrar Assimp instalado en el sistema
    find_package(assimp)
    if(NOT assimp_FOUND)
        message(WARNING "Assimp not found. Some features will be disabled.")
        message(STATUS "To enable Assimp: clone https://github.com/assimp/assimp.git to Dependencies/assimp/")
        set(ASSIMP_FOUND FALSE)
    else()
        set(ASSIMP_FOUND TRUE)
        set(ASSIMP_LIBRARIES assimp::assimp)
    endif()
endif()

# Archivos fuente organizados por módulos
file(GLOB_RECURSE CORE_SOURCES "Source/Core/*.cpp" "Source/Core/*.h")
file(GLOB_RECURSE RENDERER_SOURCES "Source/Renderer/*.cpp" "Source/Renderer/*.h")
file(GLOB_RECURSE MESH_SOURCES "Source/Mesh/*.cpp" "Source/Mesh/*.h")
file(GLOB_RECURSE ANIMATION_SOURCES "Source/Animation/*.cpp" "Source/Animation/*.h")
file(GLOB_RECURSE SCENE_SOURCES "Source/Scene/*.cpp" "Source/Scene/*.h")
file(GLOB_RECURSE INPUT_SOURCES "Source/Input/*.cpp" "Source/Input/*.h")
file(GLOB_RECURSE MATH_SOURCES "Source/Math/*.h")

# Main source
set(MAIN_SOURCES "Source/main.cpp")

# Todos los archivos fuente
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

# Shaders
file(GLOB_RECURSE SHADERS "Shaders/*.hlsl")

# Crear ejecutable
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Librerías DirectX (Windows)
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
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

# Enlazar Assimp si está disponible
if(ASSIMP_FOUND)
    target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASSIMP_ENABLED)

    if(ASSIMP_INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})
    endif()

    message(STATUS "Assimp integration enabled")
else()
    message(STATUS "Assimp integration disabled - building without model loading")
endif()

# Incluir directorios
target_include_directories(${PROJECT_NAME} PRIVATE
    "Source"
    "Source/Core"
    "Source/Renderer"
    "Source/Mesh"
    "Source/Animation"
    "Source/Scene"
    "Source/Input"
    "Source/Math"
)

# Configuraciones por build type
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

# Copiar assets y shaders
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
)

# Configuración de Visual Studio
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Organizar archivos en Visual Studio
    source_group("Core" FILES ${CORE_SOURCES})
    source_group("Renderer" FILES ${RENDERER_SOURCES})
    source_group("Mesh" FILES ${MESH_SOURCES})
    source_group("Animation" FILES ${ANIMATION_SOURCES})
    source_group("Scene" FILES ${SCENE_SOURCES})
    source_group("Input" FILES ${INPUT_SOURCES})
    source_group("Math" FILES ${MATH_SOURCES})
    source_group("Shaders" FILES ${SHADERS})
endif()

# Comentario para Assimp (agregar cuando esté disponible)
# find_package(assimp REQUIRED)
# target_link_libraries(${PROJECT_NAME} assimp::assimp)
