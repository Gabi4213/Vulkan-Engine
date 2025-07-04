cmake_minimum_required(VERSION 3.10)
project(VulkanEngine)

# c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# dependencies

# Vulkan
add_subdirectory(third_party/Vulkan_headers)
add_subdirectory(third_party/Vulkan_loader)

# GLFW 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW docs")
add_subdirectory(third_party/GLFW)

# glm
add_subdirectory(third_party/glm EXCLUDE_FROM_ALL)

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
    third_party/Vulkan_headers/include
    third_party/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# link libraries
target_link_libraries(VulkanEngine PRIVATE
    vulkan
    glfw
)

# mark shader as resource
source_group("Shaders" FILES ${SHADER_FILES})

#custom command to run compile.bat
add_custom_command(
    OUTPUT shaders_compiled_marker
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders compile.bat
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/shaders_compiled_marker
    COMMENT "Running shader compiler script"
)

add_custom_target(CompileShaders ALL DEPENDS shaders_compiled_marker)

add_dependencies(VulkanEngine CompileShaders)
