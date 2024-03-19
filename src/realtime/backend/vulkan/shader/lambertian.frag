#version 460

layout(set = 0, binding = 0) uniform SceneUbos
{
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 normalTransform;
	vec3 cameraWorldPos;
}
sceneUbos;
// TODO: we use a one-pixel image for now, change to Variant(choose different variant of shader
// according to different input types, like diffuseMap and diffseColor) when possible
layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
struct LightIndex {
	uint type;
	uint index;
};
layout(std430, set = 2, binding = 0) readonly buffer LightIndexes
{
	uint size;
	LightIndex indexes[];
}
lightIndexes;
struct PointLightInfo {
	vec3 pos;
	vec3 intensity;
};
layout(std430, set = 2, binding = 1) readonly buffer PointLightInfos
{
	PointLightInfo infos[];
}
pointLightInfos;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

#define PI 3.141592653589793
#define INV_PI 0.3183098861837907
#define LIGHT_TYPE_POINT 0
mat3 TBN;

struct SurfaceInteraction {
	vec3 pos;
	vec2 uv;
	vec3 geomNormal;
	vec3 shadingNormal;
	vec3 tangent;
	vec3 biTangent;
	vec3 dpdx;
	vec3 dpdy;
} si;
vec3 diffuse_brdf(vec3 color)
{
	return 1 / PI * color;
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

vec3 transformNormal(in vec3 normal)
{
	return TBN * normalize(normal * 2 - vec3(1, 1, 1)).xyz;
}
/******** material specific functions begin ********/
void computeGlobalParams()
{
	const vec3 n = normalize(fragNormal);
	const vec3 t = normalize(fragTangent);
	const vec3 b = cross(n, t);
	TBN = mat3(t, b, n);
	si.pos = fragWorldPos;
	si.uv = fragUV;
	si.geomNormal = n;
	si.shadingNormal = transformNormal(texture(normalMap, si.uv).xyz);
	si.tangent = t;
	si.biTangent = b;
	// TODO: check if partial derivatives right
	si.dpdx = dFdx(gl_FragCoord).xyz;
	si.dpdy = dFdy(gl_FragCoord).xyz;
}

vec3 getBsdf(in vec3 wi, in vec3 wo)
{
	const vec3 diffuse = texture(diffuseMap, fragUV).rgb;
	return diffuse * INV_PI;
}
/******** material specific functions end ********/

vec3 getPointLightRadiance(in uint index)
{
	PointLightInfo light = pointLightInfos.infos[index];
	const float dist = distance(light.pos, si.pos);
	return light.intensity / (dist * dist);
}

vec3 getIncidentRadiance(in LightIndex light)
{
	switch (light.type) {
		case LIGHT_TYPE_POINT: {
			return getPointLightRadiance(light.index);
		}
		default: {
			return vec3(0, 0, 0);
		}
	}
}

// Note: the light shader either calculate the exit randiance by sampling or approx it by other
// methods. In the second manner, the light equation is calculated in a different form, so we
// provide this method wrapping 2 kinds of methods.
vec3 getExitanceRadiance(in LightIndex light, in vec3 wo)
{
	switch (light.type) {
		case 0: {
			const vec3 wi = normalize(pointLightInfos.infos[light.index].pos - fragWorldPos);
			const float cosTheta = dot(wi, si.shadingNormal);
			return getBsdf(wi, wo) * getPointLightRadiance(light.index) * cosTheta;
		}
		default: {
			return vec3(0, 0, 0);
		}
	}
}

void main()
{
	computeGlobalParams();
	const vec3 diffuse = texture(diffuseMap, fragUV).rgb;
	const vec3 wo = normalize(fragWorldPos - sceneUbos.cameraWorldPos);
	vec3 res = vec3(0, 0, 0);
	for (uint i = 0; i < lightIndexes.size; ++i) {
		res += getExitanceRadiance(lightIndexes.indexes[i], wo);
	}
	outColor = vec4(res, 1.0);
}
