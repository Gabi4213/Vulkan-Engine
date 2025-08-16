#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;


layout(set=0, binding=0) uniform UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConst 
{
    mat4 modelPC;
    vec4 color;
} pc;

layout(location=0) out vec3 vNormal;
layout(location=1) out vec2 vUV;

void main() 
{
    mat4 M = pc.modelPC;
    mat3 N = mat3(transpose(inverse(M)));
    vNormal = normalize(N * inNormal);
    vUV     = inUV;
    gl_Position = ubo.proj * ubo.view * M * vec4(inPos, 1.0);
}
