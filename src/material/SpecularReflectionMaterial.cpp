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
	assert(primRec.frame == FrameCategory::WORLD);
	const auto worldToLocal = primRec.getCurrentFrame().inverse();
	return brdf->getBRDF((worldToLocal * wo).normalized(), (worldToLocal * wi).normalized());
}

ColorRGB PerfectReflectionMaterial::sampleBRDF(const Vector2f& uSample,
											   const HitRecord& primRec,
											   const Vector3f& wo,
											   Vector3f& wi)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBRDF(uSample, (worldToLocal * wo).normalized(), wi);
	wi = localToWorld * wi;
	wi.normalize();
	return ret;
}

ColorRGB PerfectReflectionMaterial::sampleBRDFWithPdf(const Vector2f& uSample,
													  const HitRecord& primRec,
													  const Vector3f& wo,
													  Vector3f& wi,
													  float& pdf)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto localToWorld = primRec.getCurrentFrame();
	const auto worldToLocal = localToWorld.inverse();
	auto ret = brdf->sampleBRDFWithPdf(uSample, worldToLocal * wo, wi, pdf);
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
	return brdf->getPdf(wo);
}
