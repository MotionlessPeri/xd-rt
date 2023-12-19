//
// Created by Frank on 2023/12/19.
//

#include "PerfectFresnelMaterial.h"
#include "HitRecord.h"
using namespace xd;
PerfectFresnelMaterial::PerfectFresnelMaterial(float etaOut, float etaIn)
	: etaOut(etaOut), etaIn(etaIn), reflection(), transmission(etaOut, etaIn)
{
}

ColorRGB PerfectFresnelMaterial::getBxDF(const HitRecord& primRec,
										 const Vector3f& wo,
										 const Vector3f& wi) const
{
	return {0, 0, 0};
}
ColorRGB PerfectFresnelMaterial::sampleBxDF(const Vector2f& uSample,
											const HitRecord& primRec,
											const Vector3f& wo,
											Vector3f& wi) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> ColorRGB {
		if (sampleReflection) {
			return fresnel * reflection.sampleBxDF(uSample, primRec, wo, wi);
		}
		else
			return (1 - fresnel) * transmission.sampleBxDF(uSample, primRec, wo, wi);
	};
	return sampleTemplate(uSample, primRec, wo, sampleFunc);
}
ColorRGB PerfectFresnelMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
												   const HitRecord& primRec,
												   const Vector3f& wo,
												   Vector3f& wi,
												   float& pdf) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> ColorRGB {
		ColorRGB res;
		if (sampleReflection) {
			const auto& factor = fresnel;
			res = factor * reflection.sampleBxDFWithPdf(uSample, primRec, wo, wi, pdf);
			pdf *= factor;
		}
		else {
			const auto factor = 1 - fresnel;
			res = factor * transmission.sampleBxDFWithPdf(uSample, primRec, wo, wi, pdf);
			pdf *= factor;
		}
		return res;
	};
	return sampleTemplate(uSample, primRec, wo, sampleFunc);
}
float PerfectFresnelMaterial::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	return 0;
}
Vector3f PerfectFresnelMaterial::sampleDirection(const Vector2f& uSample,
												 const HitRecord& primRec,
												 const Vector3f& wo) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> Vector3f {
		if (sampleReflection) {
			return reflection.sampleDirection(uSample, primRec, wo);
		}
		else
			return transmission.sampleDirection(uSample, primRec, wo);
	};
	return sampleTemplate(uSample, primRec, wo, sampleFunc);
}
Vector3f PerfectFresnelMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
														const HitRecord& primRec,
														const Vector3f& wo,
														float& pdf) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> Vector3f {
		Vector3f res;
		if (sampleReflection) {
			res = reflection.sampleDirectionWithPdf(uSample, primRec, wo, pdf);
			pdf *= fresnel;
		}
		else {
			res = transmission.sampleDirectionWithPdf(uSample, primRec, wo, pdf);
			pdf *= (1 - fresnel);
		}
		return res;
	};
	return sampleTemplate(uSample, primRec, wo, sampleFunc);
}
bool PerfectFresnelMaterial::isDelta() const
{
	return true;
}
