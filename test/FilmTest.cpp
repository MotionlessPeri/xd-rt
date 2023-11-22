//
// Created by Frank on 2023/8/16.
//
#include "../src/core/Film.h"
#include "gtest/gtest.h"
using namespace xd;

TEST(FilmTestSuite, sampleTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;

	constexpr double oneSixth = 1. / 6.;
	constexpr double oneQuater = 1. / 4.;
	const Vector2f sample0{oneSixth, oneQuater};  // should be index 0
	const ColorRGB color0{0.2, 0.3, 0.6};
	const Vector2f sample1{4 * oneSixth, 2 * oneQuater};  // should be index 5
	const ColorRGB color1{0.3, 0.7, 0.2};
	const ColorRGB zero{0, 0, 0};

	const Vector2f sample2{0, 0};						  // should be index 0
	const Vector2f sample3{2 * oneSixth, 2 * oneQuater};  // should be index 4
	Film film{center, right, up, width, height};
	film.addSample(color0, sample0);
	film.addSample(color1, sample1);

	const auto res0 = film.getPixelColor(sample0);
	const auto res1 = film.getPixelColor(sample1);
	const auto res2 = film.getPixelColor(sample2);
	const auto res3 = film.getPixelColor(sample3);

	EXPECT_TRUE(res0.isApprox(color0));
	EXPECT_TRUE(res1.isApprox(color1));
	EXPECT_TRUE(res2.isApprox(color0));
	EXPECT_TRUE(res3.isApprox(zero));
}

TEST(FilmTestSuite, saveToFileFailedTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	Film film{center, right, up, width, height};
	const std::string fileName = R"(D:\wtf.cpp)";
	EXPECT_THROW({ film.saveToFile(fileName); }, std::runtime_error);
}

TEST(FilmTestSuite, saveToFileTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	Film film{center, right, up, width, height};

	constexpr double oneSixth = 1. / 6.;
	constexpr double oneQuater = 1. / 4.;
	const Vector2f sample0{oneSixth, oneQuater};  // should be index 0
	const ColorRGB red{0.8, 0.1, 0.1};
	const Vector2f sample4{3 * oneSixth, 2 * oneQuater};  // should be index 4
	const ColorRGB green{0.1, 0.7, 0.1};
	const Vector2f sample5{5 * oneSixth, 2 * oneQuater};  // should be index 5
	const ColorRGB white{1, 1, 1};

	film.addSample(red, sample0);
	film.addSample(green, sample4);
	film.addSample(white, sample5);

	const std::string fileName = R"(D:\saveToFileTest.hdr)";
	EXPECT_NO_THROW(film.saveToFile(fileName););
}