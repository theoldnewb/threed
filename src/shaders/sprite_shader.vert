#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTex;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTex;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0f, 1.0f) ;
    fragColor = inColor ;
    fragTex = inTex ;
}
