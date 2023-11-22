//
// Created by Frank on 2023/8/16.
//
#include "../src/core/Sampler.h"
#include "gtest/gtest.h"

using namespace xd;
TEST(SimpleSamplerTestSuite, generateSamples1)
{
	uint32_t width = 2;
	uint32_t height = 2;
	Vector2f results[] = {{0.25, 0.25}, {0.75, 0.25}, {0.25, 0.75}, {0.75, 0.75}};

	SimpleSampler sampler(width, height);
	auto samples = sampler.generateSamples();
	EXPECT_EQ(samples.size(), width * height);
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			auto index = row * width + col;
			EXPECT_TRUE(samples[index].isApprox(results[index]));
		}
	}
}

TEST(SimpleSamplerTestSuite, generateSamples2)
{
	uint32_t width = 3;
	uint32_t height = 2;
	constexpr double oneThird = 1. / 3.;
	constexpr double oneSixth = oneThird / 2.;
	constexpr double oneHalf = 0.5;
	constexpr double oneQuater = oneHalf / 2.;
	Vector2f results[] = {
		{oneSixth, oneQuater},
		{oneSixth + oneThird, oneQuater},
		{oneSixth + oneThird * 2, oneQuater},
		{oneSixth, oneQuater + oneHalf},
		{oneSixth + oneThird, oneQuater + oneHalf},
		{oneSixth + oneThird * 2, oneQuater + oneHalf},
	};

	SimpleSampler sampler(width, height);
	auto samples = sampler.generateSamples();
	EXPECT_EQ(samples.size(), width * height);
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			auto index = row * width + col;
			EXPECT_TRUE(samples[index].isApprox(results[index]));
		}
	}
}