//
// Created by Frank on 2023/9/22.
//
#include "Distribution.h"
using namespace xd;
float CosineHemisphere::getPdf(const Vector3f& sample) const
{
	return sample.z() * INV_PI;
}
InverseMethodDistribution<3, 2>::RetType CosineHemisphere::sample(
	const InverseMethodDistribution<3, 2>::UniformSampleType& uSample)
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
InverseMethodDistribution<3, 2>::RetType CosineHemisphere::sampleWithPdf(
	const InverseMethodDistribution<3, 2>::UniformSampleType& uSample,
	float& pdf)
{
	const auto s = sample(uSample);
	pdf = s.z() * INV_PI;
	return s;
}
