#!/bin/bash

$VULKAN_SDK/bin/glslc shaders/default_shader.vert -o shaders/default_shader.vert.spv
$VULKAN_SDK/bin/glslc shaders/default_shader.frag -o shaders/default_shader.frag.spv
$VULKAN_SDK/bin/glslc shaders/quad_shader.vert -o shaders/quad_shader.vert.spv
$VULKAN_SDK/bin/glslc shaders/quad_shader.frag -o shaders/quad_shader.frag.spv