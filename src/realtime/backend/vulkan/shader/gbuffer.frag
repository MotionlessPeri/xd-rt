#version 450
#define SRGB_FAST_APPROXIMATION

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(set = 1, binding = 0) uniform sampler2D colorMap;
layout(set = 1, binding = 1) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D aoMap;
layout(set = 1, binding = 4) uniform sampler2D emissiveMap;
layout(set = 1, binding = 5) uniform MaterialParams
{
	vec4 baseColorFactor;
	vec3 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;
	int baseColorTextureSet;
	int metallicRoughnessTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	int alphaMode;
	float alphaCutoff;
}
material;

// TODO: change to a more tightly pack
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outRMO;	// [roughness, metallic, occlusion, alpha]
layout(location = 4) out vec4 outEmissive;

vec2 getUV(int set)
{
	return inUV;
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz, vec3(2.2));
#else	// SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045), srgbIn.xyz);
	vec3 linOut = mix(srgbIn.xyz / vec3(12.92),
					  pow((srgbIn.xyz + vec3(0.055)) / vec3(1.055), vec3(2.4)), bLess);
#endif	// SRGB_FAST_APPROXIMATION
	return vec4(linOut, srgbIn.w);
	;
}

void main()
{
	vec4 color = SRGBtoLINEAR(texture(colorMap, getUV(material.baseColorTextureSet))) *
				 material.baseColorFactor;
	float roughness = texture(metallicRoughnessMap, getUV(material.metallicRoughnessTextureSet)).g *
					  material.roughnessFactor;
	float metallic = texture(metallicRoughnessMap, getUV(material.metallicRoughnessTextureSet)).b *
					 material.metallicFactor;
	float occlusion = 1.0 + material.occlusionStrength *
								(texture(aoMap, getUV(material.occlusionTextureSet)).r - 1.0);
	vec3 emissive = SRGBtoLINEAR(texture(emissiveMap, getUV(material.emissiveTextureSet))).rgb *
					material.emissiveFactor;
	vec4 sampledNormal =
		normalize(texture(normalMap, getUV(material.normalTextureSet)) * 2 - vec4(1, 1, 1, 0));
	vec3 geometricNormal = inNormal;

	vec3 T = inTangent;
	vec3 B = inBitangent;
	if (abs(length(T)) < 1e-5) {
		vec3 edge1 = dFdx(inPosition).xyz;
		vec3 edge2 = dFdy(inPosition).xyz;
		vec2 deltaUV1 = dFdx(inUV);
		vec2 deltaUV2 = dFdy(inUV);
		float f = 1.0 / (deltaUV1.s * deltaUV2.t - deltaUV2.s * deltaUV1.t);
		T = normalize(f * (deltaUV2.t * edge1 - deltaUV1.t * edge2));
		B = normalize(f * (-deltaUV2.s * edge1 + deltaUV1.s * edge2));
	}
	vec3 N =
		normalize(geometricNormal * sampledNormal.b + T * sampledNormal.g + B * sampledNormal.r);

	outColor = color;
	outPosition = vec4(inPosition, 1);
	outNormal = vec4(N, 1);
	outRMO = vec4(roughness, metallic, occlusion, 1);
	outEmissive = vec4(emissive, 1);
}