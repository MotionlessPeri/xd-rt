//
// Created by Frank on 2023/8/16.
//
#include <ranges>
#include "Film.h"
#include "gtest/gtest.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
TEST(SamplerTestSuite, SimpleSamplerGetTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 500};
	const Vector3f up{0, 500, 0};
	constexpr uint32_t width = 10;
	constexpr uint32_t height = 10;
	const Vector2f resolution{width, height};
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});
	SimpleSampler sampler(1);
	constexpr int sampleCnt = 1e5;
	for ([[maybe_unused]] auto i : std::views::iota(0, sampleCnt)) {
		auto sample = resolution.cwiseProduct(sampler.sample2D());
		tile->addSample({1, 1, 1}, sample, 1.f);
	}
	film.mergeTileToFilm(std::move(tile));
	EXPECT_NO_THROW(film.saveToFile(R"(D:\SimpleSampler_get2D_test.hdr)", {true}));
}

TEST(SamplerTestSuite, SimpleSamplerGetArrayTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 500};
	const Vector3f up{0, 500, 0};
	constexpr uint32_t width = 10;
	constexpr uint32_t height = 10;
	const Vector2f resolution{width, height};
	Film film{center, right, up, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});
	SimpleSampler sampler(1);
	constexpr int sampleCnt = 1e5;
	sampler.request2DArray(sampleCnt);
	for (const auto& r : sampler.get2DArray(sampleCnt)) {
		auto sample = resolution.cwiseProduct(r);
		tile->addSample({1, 1, 1}, sample, 1.f);
	}
	film.mergeTileToFilm(std::move(tile));
	EXPECT_NO_THROW(film.saveToFile(R"(D:\SimpleSampler_get2DArray_test.hdr)", {true}));
}
