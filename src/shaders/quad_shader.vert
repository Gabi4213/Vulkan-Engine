#version 450

layout(location=0) in vec2 inPos;
layout(location=1) in vec3 inColor;
layout(location=2) in vec2 inUV;

layout(set=0, binding=0) uniform UBO {
  mat4 model_dummy; // not used; keep your global UBO (view/proj)
  mat4 view;
  mat4 proj;
} ubo;

layout(push_constant) uniform PushConst {
  mat4 model;
  vec4 color;
} pc;

layout(location=0) out vec2 vUV;
layout(location=1) out vec4 vColor;

void main() {
  gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPos, 0.0, 1.0);
  vUV = inUV;
  vColor = pc.color;
}
