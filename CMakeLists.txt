cmake_minimum_required(VERSION 3.10)
project(VulkanEngine)

# c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# dependencies

# Vulkan
add_subdirectory(third_party/vulkan_headers)
add_subdirectory(third_party/vulkan_loader)

# GLFW 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW docs")
add_subdirectory(third_party/glfw)

# glm
add_subdirectory(third_party/glm EXCLUDE_FROM_ALL)


#imgui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

file(GLOB IMGUI_CORE_SOURCES
    ${IMGUI_DIR}/*.cpp
)
file(GLOB IMGUI_BACKEND_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_*.cpp
)

add_library(imgui STATIC
    ${IMGUI_CORE_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
)

# Include directories for ImGui
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
	${CMAKE_CURRENT_SOURCE_DIR}/third_party/vulkan_headers/include
)

# Link ImGui backends to GLFW and Vulkan
target_link_libraries(imgui PUBLIC
    glfw       # for imgui_impl_glfw
    vulkan     # for imgui_impl_vulkan
)

# Prevent ImGui Vulkan backend from loading its own prototypes
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)


# get files like shaders, headers and cpps
file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES src/*.h include/*.h)
file(GLOB SHADER_FILES
    src/*.vert
    src/*.frag
    src/*.spv
)

# create executable
add_executable(VulkanEngine
    ${SRC_FILES}
    ${HEADER_FILES}
    ${SHADER_FILES}
)

# include directories
target_include_directories(VulkanEngine PRIVATE
    third_party/vulkan_headers/include
    third_party/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# link libraries
target_link_libraries(VulkanEngine PRIVATE
    vulkan
    glfw
	imgui
)

# mark shader as resource
source_group("Shaders" FILES ${SHADER_FILES})

add_custom_target(CompileShaders ALL DEPENDS shaders_compiled_marker)

add_dependencies(VulkanEngine CompileShaders)
