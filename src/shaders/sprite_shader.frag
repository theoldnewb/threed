#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTex;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    //outColor = vec4(fragTex, 0.0, 1.0);
    //outColor = texture(texSampler, fragTex) ;
    //outColor = vec4(fragColor * texture(texSampler, fragTex)) ;
    outColor = texture(texSampler, fragTex) ;
}
