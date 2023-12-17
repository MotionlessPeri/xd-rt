//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_DISCRETEUNIFORMDISTRIBUTION_H
#define XD_RT_DISCRETEUNIFORMDISTRIBUTION_H
#include <random>
#include <ranges>
#include "Distribution.h"
namespace xd {
template <uint32_t N>
class DiscreteUniformDistribution {
public:
	explicit DiscreteUniformDistribution(int lower, int upper, int seed) : dis(lower, upper)
	{
		std::seed_seq seq{std::initializer_list<int>{seed}};
		std::array<unsigned int, N> seeds;
		seq.generate(seeds.begin(), seeds.end());
		for (auto i : std::views::iota(0u, N)) {
			generators[i] = std::mt19937{seeds[i]};
		}
	}

	VectorNi<N> sample()
	{
		VectorNi<N> res;
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

protected:
	std::array<std::mt19937, N> generators;
	std::uniform_int_distribution<int> dis;
};
}  // namespace xd

#endif	// XD_RT_DISCRETEUNIFORMDISTRIBUTION_H
