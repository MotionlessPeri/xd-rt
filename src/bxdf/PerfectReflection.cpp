//
// Created by Frank on 2023/11/26.
//
#include "BxDF.h"
#include "HitRecord.h"
#include "MathUtil.h"
using namespace xd;
ColorRGB PerfectReflection::getBRDF(const Vector3f& wi, const Vector3f& wo) const
{
	const auto half = wi + wo;
	if (fuzzyEqual(half.x(), 0) && fuzzyEqual(half.y(), 0))
		return {1, 1, 1};
	else
		return {0, 0, 0};
}

ColorRGB PerfectReflection::sampleBRDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi)
{
	wi = {-wo.x(), -wo.y(), wo.z()};
	return {1, 1, 1};
}

ColorRGB PerfectReflection::sampleBRDFWithPdf(const Vector2f& uSample,
											  const Vector3f& wo,
											  Vector3f& wi,
											  float& pdf)
{
	wi = {-wo.x(), -wo.y(), wo.z()};
	pdf = 1;
	return {1, 1, 1};
}

Vector3f PerfectReflection::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	return reflected(wo, {0, 0, 1});
}
float PerfectReflection::getPdf(const Vector3f& wo) const
{
	return 0;
}
bool PerfectReflection::isDelta() const
{
	return true;
}
Vector3f PerfectReflection::sampleDirectionWithPdf(const Vector2f& uSample,
												   const Vector3f& wo,
												   float& pdf)
{
	pdf = 1;
	return reflected(wo, {0, 0, 1});
}
