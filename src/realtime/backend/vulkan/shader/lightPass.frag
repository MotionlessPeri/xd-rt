#version 450
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputPosition;
layout(input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputNormal;
layout(input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput inputRMO;
layout(input_attachment_index = 4, set = 0, binding = 4) uniform subpassInput inputEmissive;
layout(location = 0) out vec4 outColor;
void main()
{
	vec4 color = subpassLoad(inputColor);
	float alpha = color.a;
	vec3 pos = subpassLoad(inputPosition).rgb;
	vec3 N = subpassLoad(inputNormal).rgb;
	vec4 rmo = subpassLoad(inputRMO);
	float roughness = rmo.x;
	float metallic = rmo.y;
	float occlusion = rmo.z;
	outColor = color;
}