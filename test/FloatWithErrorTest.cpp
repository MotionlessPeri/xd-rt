//
// Created by Frank on 2023/12/6.
//
#include <array>
#include <bit>
#include <chrono>
#include <random>
#include <ranges>
#include "FloatWithError.h"
#include "gtest/gtest.h"
using namespace xd;

TEST(FloatWithErrorTestSuite, arthmeticTest)
{
	constexpr float lower = std::numeric_limits<float>::min();
	constexpr float upper = 1e6f;
	std::mt19937 rd(std::chrono::steady_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<float> dis(lower, upper);
	constexpr uint32_t SAMPLE_CNT = 10000u;
	for ([[maybe_unused]] auto i : std::views::iota(0u, SAMPLE_CNT)) {
		const float a = dis(rd);
		const float b = dis(rd);
		FloatWithError ai{a};
		FloatWithError bi{b};
		constexpr std::size_t ARRAY_CNT = 7;
		std::array<float, ARRAY_CNT> trueValues{};
		trueValues[0] = a + b;
		trueValues[1] = a - b;
		trueValues[2] = a * b;
		trueValues[3] = a / b;
		trueValues[4] = std::sqrtf(a);
		trueValues[5] = a * a;
		trueValues[6] = std::fabs(a);
		std::array<FloatWithError, ARRAY_CNT> errs{};
		errs[0] = (ai + bi);
		errs[1] = (ai - bi);
		errs[2] = (ai * bi);
		errs[3] = (ai / bi);
		errs[4] = sqrt(ai);
		errs[5] = sqr(ai);
		errs[6] = abs(ai);
		for (auto j : std::views::iota(0u, ARRAY_CNT)) {
			if (j <= 3) {
				EXPECT_GT(errs[j].extent(), 0.f);
			}
			else {
				EXPECT_GE(errs[j].extent(), 0.f);
			}
			EXPECT_TRUE(errs[j].has(trueValues[j]));
		}
	}
}