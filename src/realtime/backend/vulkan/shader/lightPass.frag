#version 450
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputPosition;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput inputNormal;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput inputRMO;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput inputEmissive;

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
	vec3 specular = specular_brdf(roughness * roughness, V, L, N, H);
	vec3 diffuse = diffuse_brdf(color.rgb);
	vec3 metal_brdf = conductor_fresnel(color.rgb, specular, V, H);
	vec3 dielectric_brdf = fresnel_mix(1.5, diffuse, specular, V, H);
	vec3 f0 = vec3(0.04);
	vec3 specularColor = mix(f0, color.rgb, metallic);
	vec3 diffuseColor = color.rgb * (vec3(1) - f0);
	diffuseColor *= 1 - metallic;
	vec3 iblContrib = getIBLLo(diffuseColor, specularColor, roughness, V, N, R);
	vec3 directionalContrib = mix(dielectric_brdf, metal_brdf, metallic);
}