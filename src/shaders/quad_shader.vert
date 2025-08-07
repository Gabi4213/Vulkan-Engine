#version 450

layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConst {
    mat4 model;
    vec4 color;
} pc;

layout(location = 0) out vec4 vColor;

void main() {
    vColor      = pc.color;
    gl_Position = pc.model * vec4(inPos, 0.0, 1.0);
}
