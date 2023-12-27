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

SampleBxDFResult PerfectReflection::sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const
{
	return {Vector3f{1, 1, 1} / std::fabs(wo.z()), {-wo.x(), -wo.y(), wo.z()}};
}

SampleBxDFPdfResult PerfectReflection::sampleBxDFWithPdf(const Vector2f& uSample,
														 const Vector3f& wo) const
{
	return {Vector3f{1, 1, 1} / std::fabs(wo.z()), {-wo.x(), -wo.y(), wo.z()}, 1.f};
}

Vector3f PerfectReflection::sampleDirection(const Vector2f& uSample, const Vector3f& wo) const
{
	return {-wo.x(), -wo.y(), wo.z()};
}
float PerfectReflection::getPdf(const Vector3f& wo) const
{
	return 0;
}
bool PerfectReflection::isDelta() const
{
	return true;
}

SampleDirPdfResult PerfectReflection::sampleDirectionWithPdf(const Vector2f& uSample,
															 const Vector3f& wo) const
{
	return {{-wo.x(), -wo.y(), wo.z()}, 1.f};
}
