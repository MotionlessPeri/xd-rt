//
// Created by Frank on 2023/8/28.
//
#include <numeric>
#include <random>
#include "Distribution.h"
#include "MathUtil.h"
using namespace xd;
Vector3f UniformHemisphere::operator()() const
{
	std::random_device rd;	 // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd());	 // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<float> ksai1(0.f, 1.f);
	std::uniform_real_distribution<float> ksai2(0.f, 1.f);
	const float sample1 = ksai1(gen);
	// const float sample2 = ksai2(gen);
	const float sample2 = ksai1(gen);
	const float sqrtOneMinusKsai1 = std::sqrt(1 - sample1 * sample1);
	const float twoPiKsai2 = 2 * PI * sample2;
	const float x = sqrtOneMinusKsai1 * std::cos(twoPiKsai2);
	const float y = sqrtOneMinusKsai1 * std::sin(twoPiKsai2);
	const float z = sample1;
	return {x, y, z};
}
Vector3f UniformHemisphere::operator()(float& pdf) const
{
	pdf = INV_TWO_PI;
	return (*this)();
}
float UniformHemisphere::getPdf(const Vector3f& sample) const
{
	return INV_TWO_PI;
}
