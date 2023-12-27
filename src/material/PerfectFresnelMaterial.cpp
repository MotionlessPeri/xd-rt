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

ColorRGB PerfectFresnelMaterial::getBxDF(const LocalGeomParams& shadingGeom,
										 const Vector3f& wo,
										 const Vector3f& wi) const
{
	return {0, 0, 0};
}

SampleBxDFResult PerfectFresnelMaterial::sampleBxDF(const Vector2f& uSample,
													const LocalGeomParams& shadingGeom,
													const Vector3f& wo) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> SampleBxDFResult {
		SampleBxDFResult ret;
		if (sampleReflection) {
			ret = reflection.sampleBxDF(uSample, shadingGeom, wo);
			ret.bxdf *= fresnel;
		}
		else {
			ret = transmission.sampleBxDF(uSample, shadingGeom, wo);
			ret.bxdf *= (1 - fresnel);
		}
		return ret;
	};
	return sampleTemplate(uSample, shadingGeom, wo, sampleFunc);
}

SampleBxDFPdfResult PerfectFresnelMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
															  const LocalGeomParams& shadingGeom,
															  const Vector3f& wo) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> SampleBxDFPdfResult {
		SampleBxDFPdfResult ret;
		if (sampleReflection) {
			const auto& factor = fresnel;
			ret = reflection.sampleBxDFWithPdf(uSample, shadingGeom, wo);
			ret.bxdf *= factor;
			ret.pdf *= factor;
		}
		else {
#if 1
			const auto factor = 1 - fresnel;
			ret = transmission.sampleBxDFWithPdf(uSample, shadingGeom, wo);
			ret.bxdf *= factor;
			ret.pdf *= factor;
#else
			ret.bxdf = {0, 0, 0};
			ret.pdf = 1;
#endif
		}
		return ret;
	};
	return sampleTemplate(uSample, shadingGeom, wo, sampleFunc);
}
float PerfectFresnelMaterial::getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const
{
	return 0;
}
Vector3f PerfectFresnelMaterial::sampleDirection(const Vector2f& uSample,
												 const LocalGeomParams& shadingGeom,
												 const Vector3f& wo) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> Vector3f {
		if (sampleReflection) {
			return reflection.sampleDirection(uSample, shadingGeom, wo);
		}
		else
			return transmission.sampleDirection(uSample, shadingGeom, wo);
	};
	return sampleTemplate(uSample, shadingGeom, wo, sampleFunc);
}

SampleDirPdfResult PerfectFresnelMaterial::sampleDirectionWithPdf(
	const Vector2f& uSample,
	const LocalGeomParams& shadingGeom,
	const Vector3f& wo) const
{
	const auto sampleFunc = [&](bool sampleReflection, float fresnel) -> SampleDirPdfResult {
		SampleDirPdfResult ret;
		if (sampleReflection) {
			ret = reflection.sampleDirectionWithPdf(uSample, shadingGeom, wo);
			ret.pdf *= fresnel;
		}
		else {
			ret = transmission.sampleDirectionWithPdf(uSample, shadingGeom, wo);
			ret.pdf *= (1 - fresnel);
		}
		return ret;
	};
	return sampleTemplate(uSample, shadingGeom, wo, sampleFunc);
}
bool PerfectFresnelMaterial::isDelta() const
{
	return true;
}
