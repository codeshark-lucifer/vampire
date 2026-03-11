#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model; // keeping this for compatibility, but we use push constant
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

void main() {
    gl_Position = ubo.proj * ubo.view * push.model * vec4(aPos, 1.0);
    fragNormal = mat3(push.model) * aNormal;
    fragUV = aUV;
}
