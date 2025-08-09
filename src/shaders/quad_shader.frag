#version 450
layout(location=0) in vec2 vUV;
layout(location=1) in vec4 vColor;

layout(set=1, binding=0) uniform sampler2D uTex;

layout(location=0) out vec4 outColor;

void main() {
  vec4 tex = texture(uTex, vUV);
  outColor = tex * vColor;
}
