//
// Created by Frank on 2024/1/3.
//
#include <ranges>
#include "Image.h"
#include "MathUtil.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(ImageTestSuite, ImageCreateTest)
{
	constexpr uint32_t width = 3u;
	constexpr uint32_t height = 2u;
	std::vector<ColorRGBA> colors{
		{0.8f, 0.2f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f},	 {0.5f, 0.5f, 0.5f, 1.f},
		{1.f, 0.0f, 0.f, 1.f},	{0.4f, 0.6f, 0.3f, 1.f}, {0.2f, 0.5f, 0.7f, 1.f},
	};
	std::vector<uint8_t> u8Colors;
	std::vector<float> floatColors;
	constexpr uint32_t compCnt = width * height * 4;
	u8Colors.reserve(compCnt);
	floatColors.reserve(compCnt);
	for (const auto i : std::views::iota(0u, width * height)) {
		const auto& color = colors[i];
		u8Colors.emplace_back(uint8_t(color(0) * 255));
		u8Colors.emplace_back(uint8_t(color(1) * 255));
		u8Colors.emplace_back(uint8_t(color(2) * 255));
		u8Colors.emplace_back(uint8_t(color(3) * 255));
		floatColors.emplace_back(color(0));
		floatColors.emplace_back(color(1));
		floatColors.emplace_back(color(2));
		floatColors.emplace_back(color(3));
	}
	const auto* floatColorsConvertedRaw = reinterpret_cast<const uint8_t*>(floatColors.data());
	std::vector<uint8_t> floatColorsToChar{floatColorsConvertedRaw,
										   floatColorsConvertedRaw + compCnt * sizeof(float)};

	const Image2D u8Image{PixelFormat::FORMAT_R8G8B8A8_UNORM, width, height, std::move(u8Colors)};
	const Image2D floatImage{PixelFormat::FORMAT_R32G32B32A32_SFLOAT, width, height,
							 std::move(floatColorsToChar)};
	for (const auto i : std::views::iota(0u, width * height)) {
		const auto row = i / width;
		const auto col = i % width;
		const auto u8Color = u8Image.getPixelValue(row, col);
		const auto floatColor = floatImage.getPixelValue(row, col);
		const auto refColor = colors[i];
		EXPECT_TRUE(fuzzyEqual(u8Color, refColor, 1e-2f));
		EXPECT_TRUE(fuzzyEqual(floatColor, refColor, 1e-2f));
	}
}

TEST(ImageTestSuite, ColorSpaceTest)
{
	ColorRGBA linearExpected{0.5f, 0.5f, 0.5f, 1.f};
	ColorRGBA srgbExpected{0.21404f, 0.21404f, 0.21404f, 1.f};
	std::vector<float> pixel0{0.5f, 0.5f, 0.5f};
	std::vector<uint8_t> pixel1{127, 127, 127};
	const auto* u8Pixel = reinterpret_cast<const uint8_t*>(pixel0.data());
	Image2D linearImage{
		PixelFormat::FORMAT_R32G32B32_SFLOAT, 1, 1, {u8Pixel, u8Pixel + 3 * sizeof(float)}};
	Image2D srgbImage{PixelFormat::FORMAT_R8G8B8_SRGB, 1, 1, std::move(pixel1)};
	const auto d1 = linearImage.getPixelValue(0, 0);
	const auto d2 = srgbImage.getPixelValue(0, 0);
	EXPECT_TRUE(fuzzyEqual(linearImage.getPixelValue(0, 0), linearExpected, 1e-4f));
	EXPECT_TRUE(fuzzyEqual(srgbImage.getPixelValue(0, 0), srgbExpected, 1e-2f));
}