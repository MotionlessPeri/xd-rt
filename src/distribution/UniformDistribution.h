//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_UNIFORMDISTRIBUTION_H
#define XD_RT_UNIFORMDISTRIBUTION_H
#include <random>
#include <ranges>
#include "Distribution.h"
namespace xd {
template <uint32_t N>
class UniformDistribution : Distribution<N> {
public:
	explicit UniformDistribution()
	{
		for (auto& gen : generators) {
			gen = std::mt19937{rd()};
		}
	}
	explicit UniformDistribution(int seed)
	{
		std::seed_seq seq{std::initializer_list<int>{seed}};
		std::array<unsigned int, N> seeds;
		seq.generate(seeds.begin(), seeds.end());
		for (auto i : std::views::iota(0u, N)) {
			generators[i] = std::mt19937{seeds[i]};
		}
	}
	VectorNf<N> sample() override
	{
		VectorNf<N> res;
		if constexpr (N != 1) {
			for (auto i = 0u; i < N; ++i) {
				res(i) = dis(generators[i]);
			}
		}
		else {
			res = dis(generators[0]);
		}

		return res;
	}
	VectorNf<N> sampleWithPdf(float& pdf) override
	{
		pdf = 1.f;
		return sample();
	}
	float getPdf(const VectorNf<N>& sample) const override { return 1.f; }

protected:
	std::random_device rd;
	std::array<std::mt19937, N> generators;
	std::uniform_real_distribution<float> dis{0, 1.f - 1e-4f};
};

}  // namespace xd
#endif	// XD_RT_UNIFORMDISTRIBUTION_H
