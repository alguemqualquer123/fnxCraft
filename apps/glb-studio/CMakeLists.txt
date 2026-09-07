cmake_minimum_required(VERSION 3.16)
project(glb-studio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE GLB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(glb-studio ${GLB_SOURCES})

target_include_directories(glb-studio PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../shared"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/glfw-3.3.7/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/imgui-docking/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/imgui-docking/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/assimp-5.2.4/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../build/thirdparty/assimp-5.2.4/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/stb_image/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/gl2d/include"
)

target_link_libraries(glb-studio PRIVATE
    glfw
    glad
    imgui
    gl2d
    assimp
    glm
    stb_image
)

target_compile_definitions(glb-studio PRIVATE
    RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../resources/"
    GLM_ENABLE_EXPERIMENTAL
)

if(NOT MSVC)
    target_compile_options(glb-studio PRIVATE -Wno-error)
endif()
