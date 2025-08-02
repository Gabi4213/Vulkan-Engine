#!/bin/bash

$VULKAN_SDK/bin/glslc shaders/default_shader.vert -o shaders/default_shader.vert.spv
$VULKAN_SDK/bin/glslc shaders/default_shader.frag -o shaders/default_shader.frag.spv
