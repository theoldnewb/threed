#version 450

layout(location = 0) in vec2 fragTex;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTex) ;
    if(outColor.w < 0.1)
    {
       discard ;
    }
}
