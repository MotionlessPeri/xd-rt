//
// Created by Frank on 2023/9/18.
//
#include "BxDF.h"
#include "HitRecord.h"
#include "Material.h"
#include "MathUtil.h"
using namespace xd;
PerfectReflectionMaterial::PerfectReflectionMaterial() : brdf(std::make_unique<PerfectReflection>())
{
}

ColorRGB PerfectReflectionMaterial::getBRDF(const HitRecord& primRec,
											const Vector3f& wo,
											const Vector3f& wi) const
{
	return brdf->getBRDF((primRec.modelToLocal * wo).normalized(),
						 (primRec.modelToLocal * wi).normalized());
}

ColorRGB PerfectReflectionMaterial::sampleBRDF(const Vector2f& uSample,
											   const HitRecord& primRec,
											   const Vector3f& wo,
											   Vector3f& wi)
{
	auto ret = brdf->sampleBRDF(uSample, (primRec.modelToLocal * wo).normalized(), wi);
	wi = primRec.localToModel * wi;
	wi.normalize();
	return ret;
}

ColorRGB PerfectReflectionMaterial::sampleBRDFWithPdf(const Vector2f& uSample,
													  const HitRecord& primRec,
													  const Vector3f& wo,
													  Vector3f& wi,
													  float& pdf)
{
	auto ret = brdf->sampleBRDF(uSample, primRec.modelToLocal * wo, wi);
	wi = primRec.localToModel * wi;
	wi.normalize();
	return ret;
}

Vector3f PerfectReflectionMaterial::sampleDirection(const Vector2f& uSample,
													const Vector3f& wo,
													const HitRecord& hitRecord) const
{
	return (hitRecord.localToModel * brdf->sampleDirection(uSample, hitRecord.modelToLocal * wo))
		.normalized();
}
Vector3f PerfectReflectionMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
														   const Vector3f& wo,
														   const HitRecord& hitRecord,
														   float& pdf) const
{
	return (hitRecord.localToModel *
			brdf->sampleDirectionWithPdf(uSample, hitRecord.modelToLocal * wo, pdf))
		.normalized();
}
float PerfectReflectionMaterial::getPdf(const HitRecord& hitRecord, const Vector3f& wo) const
{
	return brdf->getPdf(wo);
}
