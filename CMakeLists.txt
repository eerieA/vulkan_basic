cmake_minimum_required(VERSION 3.22)
project(VulkanEngine)

find_package(Vulkan REQUIRED)

include(cmake/Shaders.cmake)
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm"
    GIT_TAG "1.0.1"
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.14.1"
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw"
    GIT_TAG "3.4"
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    microsoft-gsl
    GIT_REPOSITORY "https://github.com/microsoft/GSL.git"
    GIT_TAG "v4.0.0"
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(microsoft-gsl)

file(GLOB_RECURSE VulkanEngineSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(VulkanEngine ${VulkanEngineSources})

target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanEngine PRIVATE glm)
target_link_libraries(VulkanEngine PRIVATE glfw)
target_link_libraries(VulkanEngine PRIVATE Microsoft.GSL::GSL)
target_link_libraries(VulkanEngine PRIVATE spdlog)

target_include_directories(VulkanEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_features(VulkanEngine PRIVATE cxx_std_20)

target_precompile_headers(VulkanEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/precomp.h")

file(GLOB_RECURSE ShaderSources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)

add_shaders(VulkanEngineShaders ${ShaderSources})
add_dependencies(VulkanEngine VulkanEngineShaders)