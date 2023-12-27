//
// Created by Frank on 2023/12/16.
//
#include "PerfectTransmission.h"
using namespace xd;
PerfectTransmission::PerfectTransmission(float eta) : eta(eta) {}

PerfectTransmission::PerfectTransmission(float etaI, float etaT) : eta(etaT / etaI) {}

ColorRGB PerfectTransmission::getBxDF(const Vector3f& wi, const Vector3f& wo) const
{
	return {0, 0, 0};
}

SampleBxDFResult PerfectTransmission::sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto [total, sin2ThetaT] = checkTotalReflection(eta, wo.z());
	if (total)
		return {{0, 0, 0}, {}};
	const float cosThetaT = std::sqrtf(1 - sin2ThetaT);
	Vector3f brdf{1, 1, 1};
	brdf *= eta * eta;
	brdf /= std::fabs(wo.z());
	return {brdf, refract(wo, cosThetaT)};
}

SampleBxDFPdfResult PerfectTransmission::sampleBxDFWithPdf(const Vector2f& uSample,
														   const Vector3f& wo) const
{
	return {sampleBxDF(uSample, wo), 1.f};
}
Vector3f PerfectTransmission::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto [total, sin2ThetaT] = checkTotalReflection(eta, wo.z());
	if (total)
		return {0, 0, 0};
	const float cosThetaT = std::sqrtf(1 - sin2ThetaT);
	return refract(wo, cosThetaT);
}

SampleDirPdfResult PerfectTransmission::sampleDirectionWithPdf(const Vector2f& uSample,
															   const Vector3f& wo) const
{
	return {sampleDirection(uSample, wo), 1.f};
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
