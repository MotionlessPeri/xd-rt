//
// Created by Frank on 2023/12/16.
//
#include "BxDF.h"
using namespace xd;
PerfectTransmission::PerfectTransmission(float eta) : eta(eta) {}

PerfectTransmission::PerfectTransmission(float etaI, float etaT) : eta(etaT / etaI) {}

ColorRGB PerfectTransmission::getBxDF(const Vector3f& wi, const Vector3f& wo) const
{
	return {0, 0, 0};
}
ColorRGB PerfectTransmission::sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi)
{
	Vector3f& wt = wi;
	const auto [total, sin2ThetaT] = checkTotalReflection(wo);
	if (total)
		return {0, 0, 0};
	const float cosThetaT = std::sqrtf(1 - sin2ThetaT);
	wt = refract(wo, cosThetaT);
	return {1, 1, 1};
}
ColorRGB PerfectTransmission::sampleBxDFWithPdf(const Vector2f& uSample,
												const Vector3f& wo,
												Vector3f& wi,
												float& pdf)
{
	pdf = 1;
	return sampleBxDF(uSample, wo, wi);
}
Vector3f PerfectTransmission::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto [total, sin2ThetaT] = checkTotalReflection(wo);
	if (total)
		return {0, 0, 0};
	const float cosThetaT = std::sqrtf(1 - sin2ThetaT);
	return refract(wo, cosThetaT);
}

Vector3f PerfectTransmission::sampleDirectionWithPdf(const Vector2f& uSample,
													 const Vector3f& wo,
													 float& pdf)
{
	pdf = 1;
	return sampleDirection(uSample, wo);
}

float PerfectTransmission::getPdf(const Vector3f& wi) const
{
	return 0;
}
bool PerfectTransmission::isDelta() const
{
	return true;
}
Vector3f PerfectTransmission::refract(const Vector3f& wo, float cosThetaT) const
{
	return -wo / eta + Vector3f{0, 0, wo.z() / eta - cosThetaT};
}
