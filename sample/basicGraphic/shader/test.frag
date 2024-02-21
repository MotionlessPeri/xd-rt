#version 460

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

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
	outColor = SRGBtoLINEAR(texture(texSampler, fragUV));
}
