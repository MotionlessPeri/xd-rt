#version 450
layout(set = 0, binding = 0) uniform SceneUbos
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normalTransform;
	vec4 cameraWorldPos;
}
sceneUbos;

layout(push_constant) uniform UBONode
{
	mat4 model;
}
primitive;
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
void main()
{
	fragWorldPos = (sceneUbos.model * vec4(inPos, 1.0)).xyz;
	const mat4 transform = sceneUbos.proj * sceneUbos.view * sceneUbos.model;
	gl_Position = transform * vec4(inPos, 1.0);
	fragUV = inUV;
	fragNormal = mat3(sceneUbos.normalTransform) * inNormal;
	fragTangent = mat3(transform) * inTangent;
	vec4 pos = sceneUbos.view * vec4(inPos, 1.0);
	fragNormal = normalize(transpose(inverse(mat3(sceneUbos.view * primitive.model))) * inNormal);
	if (abs(length(inTangent)) < 1e-5) {
		fragTangent = inTangent.xyz;
		fragBitangent = cross(fragNormal, fragTangent);
	}
	else {
		vec3 zero = vec3(0, 0, 0);
		fragTangent = zero;
		fragBitangent = zero;
	}
}