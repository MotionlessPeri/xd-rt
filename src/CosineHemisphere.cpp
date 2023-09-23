//
// Created by Frank on 2023/9/22.
//
#include "Distribution.h"
using namespace xd;
Vector3f CosineHemisphere::operator()() const
{
	static std::random_device rd;	// Will be used to obtain a seed for the random number engine
	static std::mt19937 gen(rd());	// Standard mersenne_twister_engine seeded with rd()
	static std::uniform_real_distribution<float> ksai1(0.f, 1.f);
	std::uniform_real_distribution<float> ksai2(0.f, 1.f);
	const float sample1 = ksai1(gen);
	const float sample2 = ksai1(gen);
	const float sqrtKsai1 = std::sqrtf(sample1);
	const float sqrtOneMinusKsai1 = std::sqrt(1 - sample1);
	const float twoPiKsai2 = 2 * PI * sample2;
	const float x = sqrtKsai1 * std::cos(twoPiKsai2);
	const float y = sqrtKsai1 * std::sin(twoPiKsai2);
	const float z = sqrtOneMinusKsai1;
	return {x, y, z};
}
Vector3f CosineHemisphere::operator()(float& pdf) const
{
	const auto sample = (*this)();
	pdf = sample.z();
	return sample;
}
float CosineHemisphere::getPdf(const Vector3f& sample) const
{
	return 0;
}
