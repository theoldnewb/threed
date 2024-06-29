#version 450


const int max_instance = 32 ;

layout(binding = 0) uniform UniformBufferObject {
    vec2 offset_ ;
    vec2 scale_ ;
    vec4 pos_[max_instance] ;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 fragTex;


void main() {
    vec2 offset = ubo.offset_ ;
    vec2 scale  = ubo.scale_ ;
    vec2 pos    = ubo.pos_[gl_InstanceIndex].xy ;
    vec2 p      = (inPosition + pos - offset) * scale ;
    gl_Position = vec4(p, 0.0f, 1.0f) ;
    fragTex = inTex ;
}
