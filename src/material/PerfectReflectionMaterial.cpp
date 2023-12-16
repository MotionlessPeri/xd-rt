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

ColorRGB PerfectReflectionMaterial::getBxDF(const HitRecord& primRec,
											const Vector3f& wo,
											const Vector3f& wi) const
{
	return {0, 0, 0};
}

ColorRGB PerfectReflectionMaterial::sampleBxDF(const Vector2f& uSample,
											   const HitRecord& primRec,
											   const Vector3f& wo,
											   Vector3f& wi)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBxDF(uSample, (worldToLocal * wo).normalized(), wi);
	wi = localToWorld * wi;
	wi.normalize();
	return ret;
}

ColorRGB PerfectReflectionMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
													  const HitRecord& primRec,
													  const Vector3f& wo,
													  Vector3f& wi,
													  float& pdf)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBxDFWithPdf(uSample, worldToLocal * wo, wi, pdf);
	wi = localToWorld * wi;
	wi.normalize();
	return ret;
}

Vector3f PerfectReflectionMaterial::sampleDirection(const Vector2f& uSample,
													const HitRecord& primRec,
													const Vector3f& wo) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	return (localToWorld * brdf->sampleDirection(uSample, worldToLocal * wo)).normalized();
}

Vector3f PerfectReflectionMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
														   const HitRecord& primRec,
														   const Vector3f& wo,
														   float& pdf) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	return (localToWorld * brdf->sampleDirectionWithPdf(uSample, worldToLocal * wo, pdf))
		.normalized();
}

float PerfectReflectionMaterial::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	return 0;
}
