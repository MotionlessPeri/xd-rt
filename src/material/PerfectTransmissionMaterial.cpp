//
// Created by Frank on 2023/12/16.
//
#include "PerfectTransmissionMaterial.h"
#include "HitRecord.h"
#include "MathUtil.h"
using namespace xd;
PerfectTransmissionMaterial::PerfectTransmissionMaterial(float etaOutside, float etaInside)
	: outToIn(std::make_unique<PerfectTransmission>(etaOutside, etaInside)),
	  inToOut(std::make_unique<PerfectTransmission>(etaInside, etaOutside))
{
}

ColorRGB PerfectTransmissionMaterial::getBxDF(const HitRecord& primRec,
											  const Vector3f& wo,
											  const Vector3f& wi) const
{
	return {0, 0, 0};
}
ColorRGB PerfectTransmissionMaterial::sampleBxDF(const Vector2f& uSample,
												 const HitRecord& primRec,
												 const Vector3f& wo,
												 Vector3f& wi)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(primRec.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(primRec, flipNormal);
	auto ret = btdf->sampleBxDF(uSample, (worldToLocal * wo).normalized(), wi);
	wi = localToWorld * wi;
	wi.normalize();
	return ret;
}
ColorRGB PerfectTransmissionMaterial::sampleBxDFWithPdf(const Vector2f& uSample,
														const HitRecord& primRec,
														const Vector3f& wo,
														Vector3f& wi,
														float& pdf)
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(primRec.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(primRec, flipNormal);
	auto ret = btdf->sampleBxDFWithPdf(uSample, worldToLocal * wo, wi, pdf);
	wi = localToWorld * wi;
	wi.normalize();
	return ret;
}
float PerfectTransmissionMaterial::getPdf(const HitRecord& primRec, const Vector3f& wo) const
{
	return 0;
}
Vector3f PerfectTransmissionMaterial::sampleDirection(const Vector2f& uSample,
													  const HitRecord& primRec,
													  const Vector3f& wo) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(primRec.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(primRec, flipNormal);
	return (localToWorld * btdf->sampleDirection(uSample, worldToLocal * wo)).normalized();
}
Vector3f PerfectTransmissionMaterial::sampleDirectionWithPdf(const Vector2f& uSample,
															 const HitRecord& primRec,
															 const Vector3f& wo,
															 float& pdf) const
{
	assert(primRec.frame == FrameCategory::WORLD);
	const auto [btdf, flipNormal] = chooseBtdf(primRec.n, wo);
	const auto [localToWorld, worldToLocal] = getTransform(primRec, flipNormal);
	return (localToWorld * btdf->sampleDirectionWithPdf(uSample, worldToLocal * wo, pdf))
		.normalized();
}
bool PerfectTransmissionMaterial::isDelta() const
{
	return true;
}
