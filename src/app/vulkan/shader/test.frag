#version 450

// layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragUV,0,1);
}
