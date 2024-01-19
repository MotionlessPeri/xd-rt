#version 450

// layout(binding = 0) uniform UniformBufferObject {
//     mat4 model;
//     mat4 view;
//     mat4 proj;
//     mat3 normalTransform;
// } ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragTangent;

void main() {
    // gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    // fragUV = inUV;
    // fragNormal = ubo.normalTransform * inNormal;
    // fragTangent = inTangent;
    gl_Position = vec4(inPosition, 1.0);
    fragUV = inUV;
}
