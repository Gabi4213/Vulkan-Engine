#version 450
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vWorldPos;

layout(location = 0) out vec4 outColor;

layout(set=1, binding=0) uniform sampler2D uTex;

layout(push_constant) uniform PushConst 
{
    mat4 model;
    vec4 color;
} pc;

void main() 
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.2));
    float ndotl = max(dot(N, L), 0.0);

    vec3 albedo = texture(uTex, vUV).rgb * pc.color.rgb;
    vec3 lit = albedo * (0.15 + 0.85 * ndotl);
    outColor = vec4(lit, 1.0);
}
