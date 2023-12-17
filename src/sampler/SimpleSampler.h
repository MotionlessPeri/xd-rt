//
// Created by Frank on 2023/11/28.
//

#ifndef XD_RT_SIMPLESAMPLER_H
#define XD_RT_SIMPLESAMPLER_H
#include "Sampler.h"
#include "distribution/DistributionTypes.h"
#include "distribution/UniformDistribution.h"
namespace xd {
class SimpleSampler : public Sampler {
public:
	explicit SimpleSampler(int samplePerPixel);
	SimpleSampler(int samplePerPixel, int seed);
	void request1DArray(int n) override;
	void request2DArray(int n) override;
	const std::span<const float> get1DArray(int n) override;
	const std::span<const Vector2f> get2DArray(int n) override;
	float sample1D() override;
	Vector2f sample2D() override;
	std::unique_ptr<Sampler> clone(int seed) const override;

protected:
	UniformDistribution<1> rng;
	std::vector<float> array1D;
	std::vector<Vector2f> array2D;
};
};		// namespace xd
#endif	// XD_RT_SIMPLESAMPLER_H
