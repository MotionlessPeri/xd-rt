//
// Created by Frank on 2023/8/28.
//
#include "UniformHemisphere.h"
#include "MathUtil.h"
using namespace xd;
float UniformHemisphere::getPdf(const Vector3f& sample) const
{
	return INV_TWO_PI;
}
InverseMethodDistribution<3, 2>::RetType UniformHemisphere::sample(
	const InverseMethodDistribution<3, 2>::UniformSampleType& uSample)
{
	const auto sample1 = uSample(0);
	const auto sample2 = uSample(1);
	const float sqrtOneMinusKsai1 = std::sqrt(1 - sample1 * sample1);
	const float twoPiKsai2 = 2 * PI * sample2;
	const float x = sqrtOneMinusKsai1 * std::cos(twoPiKsai2);
	const float y = sqrtOneMinusKsai1 * std::sin(twoPiKsai2);
	const float z = sample1;
	return {x, y, z};
}
InverseMethodDistribution<3, 2>::RetType UniformHemisphere::sampleWithPdf(
	const InverseMethodDistribution<3, 2>::UniformSampleType& uSample,
	float& pdf)
{
	pdf = INV_TWO_PI;
	return sample(uSample);
}
