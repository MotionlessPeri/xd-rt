//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_INVERSEMETHODDISTRIBUTION_H
#define XD_RT_INVERSEMETHODDISTRIBUTION_H
#include <random>
#include <ranges>
#include "Distribution.h"
#include "UniformDistribution.h"
namespace xd {

template <uint32_t RetDim, uint32_t UniformSampleDim>
class InverseMethodDistribution : public Distribution<RetDim> {
public:
	typedef VectorNf<UniformSampleDim> UniformSampleType;
	typedef VectorNf<RetDim> RetType;
	InverseMethodDistribution() : rng(std::make_unique<UniformDistribution<UniformSampleDim>>()) {}
	virtual RetType sample(const UniformSampleType& uSample) = 0;
	virtual RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) = 0;

protected:
	std::unique_ptr<UniformDistribution<UniformSampleDim>> rng{};

private:
	// prohibit sample method without uniform sample
	RetType sample() override final { return {}; }
	RetType sampleWithPdf(float& pdf) override final { return {}; }
};
}  // namespace xd
#endif	// XD_RT_INVERSEMETHODDISTRIBUTION_H
