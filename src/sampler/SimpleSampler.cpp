//
// Created by Frank on 2023/8/16.
//
#include "SimpleSampler.h"
using namespace xd;
SimpleSampler::SimpleSampler(int samplePerPixel) : Sampler(samplePerPixel) {}
void SimpleSampler::request1DArray(int n) {}
void SimpleSampler::request2DArray(int n) {}
const std::span<const float> SimpleSampler::get1DArray(int n)
{
	array1D.clear();
	array1D.resize(n);
	for (auto& sample : array1D) {
		sample = rng.sample();
	}
	return {array1D};
}
const std::span<const Vector2f> SimpleSampler::get2DArray(int n)
{
	array2D.clear();
	array2D.resize(n);
	for (auto& sample : array2D) {
		sample = {rng.sample(), rng.sample()};
	}
	return {array2D};
}
float SimpleSampler::sample1D()
{
	return rng.sample();
}
Vector2f SimpleSampler::sample2D()
{
	return {rng.sample(), rng.sample()};
}
std::unique_ptr<Sampler> SimpleSampler::clone(int seed) const
{
	return std::make_unique<SimpleSampler>(samplePerPixel, seed);
}
SimpleSampler::SimpleSampler(int samplePerPixel, int seed) : Sampler(samplePerPixel), rng(seed) {}
