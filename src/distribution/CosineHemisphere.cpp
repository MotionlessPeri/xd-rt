//
// Created by Frank on 2023/9/22.
//
#include "Distribution.h"
using namespace xd;
float CosineHemisphere::getPdf(const Vector3f& sample) const
{
	return std::fabs(sample.z()) * INV_PI;
}
Vector3f CosineHemisphere::sample(const Vector2f& uSample)
{
	const auto sample1 = uSample(0);
	const auto sample2 = uSample(1);
	const float sqrtKsai1 = std::sqrtf(sample1);
	const float sqrtOneMinusKsai1 = std::sqrt(1 - sample1);
	const float twoPiKsai2 = 2 * PI * sample2;
	const float x = sqrtKsai1 * std::cos(twoPiKsai2);
	const float y = sqrtKsai1 * std::sin(twoPiKsai2);
	const float z = sqrtOneMinusKsai1;
	return {x, y, z};
}
Vector3f CosineHemisphere::sampleWithPdf(const Vector2f& uSample,
																		 float& pdf)
{
	const auto s = sample(uSample);
	pdf = getPdf(s);
	return s;
}
