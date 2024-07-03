#version 450


const int max_instance = 32 ;

layout(binding = 0) uniform UniformBufferObject {
    vec4 pos_[max_instance][4] ;
    vec4 ori_[max_instance] ;
    vec2 offset_ ;
    vec2 scale_ ;
} ubo;

//layout(location = 0) in vec2 inPosition;
//layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 fragTex;


void main() {
    vec2 offset = ubo.offset_ ;
    vec2 scale  = ubo.scale_ ;
    vec2 pos    = ubo.pos_[gl_InstanceIndex][gl_VertexIndex].xy ;
    vec2 uv     = ubo.pos_[gl_InstanceIndex][gl_VertexIndex].zw ;
    vec2 ori    = ubo.ori_[gl_InstanceIndex].xy ;
    //vec2 p      = (inPosition + ori + pos - offset) * scale ;
    vec2 p      = (ori + pos - offset) * scale ;
    gl_Position = vec4(p, 0.0f, 1.0f) ;
    //fragTex = inTex + uv ;
    fragTex = uv ;
}
