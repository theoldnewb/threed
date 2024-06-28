#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTex;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    //outColor = vec4(fragTex, 0.0, 1.0);
    //outColor = texture(texSampler, fragTex) ;
    //if(outColor.w < 0.1)
    //{
    //    discard ;
    //}
    outColor = fragColor * texture(texSampler, fragTex) ;
    //outColor = vec4(texture(texSampler, fragTex).a, 0.0f, 0.0f, 0.0f) ;
}
