//
// Created by Frank on 2024/1/4.
//
#include "distribution/UniformDistribution.h"
#include "filter/NearestFilter.h"
#include "filter/TentFilter.h"
#include "gtest/gtest.h"
using namespace xd;

#define CREATE_TEST_IMAGE                                                           \
	constexpr uint32_t width = 3;                                                   \
	constexpr uint32_t height = 2;                                                  \
	const std::vector<ColorRGBA> pixels{{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1},   \
										{0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}};  \
	const auto* convertedRawData = reinterpret_cast<const uint8_t*>(pixels.data()); \
	const auto image = std::make_shared<Image2D>(                                   \
		PixelFormat::FORMAT_R32G32B32A32_SFLOAT, width, height, 0u,                 \
		std::vector<uint8_t>{convertedRawData,                                      \
							 convertedRawData + width * height * 4 * sizeof(float)});

TEST(FilterTestSuite, NearestFilterWrapTest)
{
	CREATE_TEST_IMAGE;
	const NearestFilter filter0{WrapMode::CLAMP, WrapMode::CLAMP};
	const NearestFilter filter1{WrapMode::REPEAT, WrapMode::CLAMP};
	const NearestFilter filter2{WrapMode::CLAMP, WrapMode::REPEAT};
	const NearestFilter filter3{WrapMode::REPEAT, WrapMode::REPEAT};
	// Test Wrap Mode
	{
		const Vector2f ibPos{0.5f, 0.5f};  // in-bound position
		const auto f0 = filter0.filter(image, ibPos);
		const auto f1 = filter1.filter(image, ibPos);
		const auto f2 = filter2.filter(image, ibPos);
		const auto f3 = filter3.filter(image, ibPos);
		EXPECT_EQ(f0, image->getPixelValue(0, 0));
		EXPECT_EQ(f1, image->getPixelValue(0, 0));
		EXPECT_EQ(f2, image->getPixelValue(0, 0));
		EXPECT_EQ(f3, image->getPixelValue(0, 0));
	}
	{
		const Vector2f oobPos{3.5f, 0.5f};	// out-of-bound position
		const auto f0 = filter0.filter(image, oobPos);
		const auto f1 = filter1.filter(image, oobPos);
		const auto f2 = filter2.filter(image, oobPos);
		const auto f3 = filter3.filter(image, oobPos);
		EXPECT_EQ(f0, image->getPixelValue(0, 2));
		EXPECT_EQ(f1, image->getPixelValue(0, 0));
		EXPECT_EQ(f2, image->getPixelValue(0, 2));
		EXPECT_EQ(f3, image->getPixelValue(0, 0));
	}
}

TEST(FilterTestSuite, NearestFilterFilterTest)
{
	CREATE_TEST_IMAGE;
	const NearestFilter filter0{WrapMode::CLAMP, WrapMode::CLAMP};
	const NearestFilter filter1{WrapMode::REPEAT, WrapMode::CLAMP};
	const NearestFilter filter2{WrapMode::CLAMP, WrapMode::REPEAT};
	const NearestFilter filter3{WrapMode::REPEAT, WrapMode::REPEAT};
	UniformDistribution<1> uDis;
	constexpr uint32_t TIMES = 1000u;
	for (const auto row : std::views::iota(0u, height)) {
		for (const auto col : std::views::iota(0u, width)) {
			for ([[maybe_unused]] const auto i : std::views::iota(0u, TIMES)) {
				const float pixelX = col + (uDis.sample() * .99f);
				const float pixelY = row + (uDis.sample() * .99f);
				const Vector2f samplePos{pixelX, pixelY};
				const auto f0 = filter0.filter(image, samplePos);
				const auto f1 = filter1.filter(image, samplePos);
				const auto f2 = filter2.filter(image, samplePos);
				const auto f3 = filter3.filter(image, samplePos);
				const auto expected = image->getPixelValue(row, col);
				EXPECT_EQ(f0, expected);
				EXPECT_EQ(f1, expected);
				EXPECT_EQ(f2, expected);
				EXPECT_EQ(f3, expected);
			}
		}
	}
}
TEST(FilterTestSuite, TentFilterTest)
{
	constexpr uint32_t width = 4;
	constexpr uint32_t height = 4;
	const std::vector<ColorRGBA> pixels{
		{1, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1},	 // rrgg
		{1, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1},	 // rrgg
		{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 0, 1}, {0, 0, 0, 1},	 // bbkk
		{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 0, 1}, {0, 0, 0, 1},	 // bbkk
	};
	const auto* convertedRawData = reinterpret_cast<const uint8_t*>(pixels.data());
	const auto image = std::make_shared<Image2D>(
		PixelFormat::FORMAT_R32G32B32A32_SFLOAT, width, height, 0u,
		std::vector<uint8_t>{convertedRawData,
							 convertedRawData + width * height * 4 * sizeof(float)});

	TentFilter filter{WrapMode::CLAMP, WrapMode::CLAMP};
	// pos at top left
	{
		const Vector2f pos{1.75f, 1.75f};
		const auto res = filter.filter(image, pos);
		const ColorRGBA expected{0.5625, 0.1875, 0.1875, 1};
		EXPECT_EQ(res, expected);
	}
	// pos at top right
	{
		const Vector2f pos{1.25f, 1.75f};
		const auto res = filter.filter(image, pos);
		const ColorRGBA expected{0.75, 0, 0.25, 1};
		EXPECT_EQ(res, expected);
	}
	// pos at bottom left
	{
		const Vector2f pos{1.75f, 1.25f};
		const auto res = filter.filter(image, pos);
		const ColorRGBA expected{0.75, 0.25, 0, 1};
		EXPECT_EQ(res, expected);
	}
	// pos at bottom right
	{
		const Vector2f pos{1.25f, 1.25f};
		const auto res = filter.filter(image, pos);
		const ColorRGBA expected{1, 0, 0, 1};
		EXPECT_EQ(res, expected);
	}
	// out-of-bound pos
	{
		const Vector2f pos{0.25f, 0.25f};
		const auto res = filter.filter(image, pos);
		const ColorRGBA expected{1, 0, 0, 1};
		EXPECT_EQ(res, expected);
	}
}