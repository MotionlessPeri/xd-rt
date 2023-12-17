//
// Created by Frank on 2023/11/26.
//
#include "PerfectReflection.h"
#include "MathUtil.h"
using namespace xd;
ColorRGB PerfectReflection::getBxDF(const Vector3f& wi, const Vector3f& wo) const
{
	return {0, 0, 0};
}

ColorRGB PerfectReflection::sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi)
{
	wi = {-wo.x(), -wo.y(), wo.z()};
	return {1, 1, 1};
}

ColorRGB PerfectReflection::sampleBxDFWithPdf(const Vector2f& uSample,
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
