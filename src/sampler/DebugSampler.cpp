//
// Created by Frank on 2023/12/3.
//

#include "DebugSampler.h"
namespace xd {
const std::vector<float> DebugSampler::array1d = std::vector<float>(MAX_SIZE, 0.5f);
DebugSampler::DebugSampler(int samplerPerPixel) : Sampler(samplerPerPixel) {}

void DebugSampler::request1DArray(int n)
{
	assert(n < MAX_SIZE);
}

void DebugSampler::request2DArray(int n)
{
	assert(n < MAX_SIZE);
}

const std::span<const float> DebugSampler::get1DArray(int n)
{
	assert(n < MAX_SIZE);
	return std::span(array1d.begin(), n);
}

const std::span<const Vector2f> DebugSampler::get2DArray(int n)
{
	assert(n < MAX_SIZE);
	return std::span(array2d.begin(), n);
}

float DebugSampler::sample1D()
{
	return 0.5f;
}

Vector2f DebugSampler::sample2D()
{
	return {0.5f, 0.5f};
}

std::unique_ptr<Sampler> DebugSampler::clone(int seed) const
{
	return std::make_unique<DebugSampler>(samplePerPixel);
}

}  // namespace xd