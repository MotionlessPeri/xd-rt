//
// Created by Frank on 2023/8/28.
//
#include <oneapi/tbb.h>
#include <atomic>
#include "Distribution.h"
#include "Film.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(DistribTestSuite, UniformHemisphereTest)
{
	auto dis = std::make_shared<UniformHemisphere>();
	auto film =
		std::make_shared<Film>(Vector3f{0, 0, 0}, Vector3f{1, 0, 0}, Vector3f{0, 1, 0}, 100, 100);
	constexpr auto nSamples = 10000u;
	for (auto i = 0u; i < nSamples; ++i) {
		const auto sample = dis->operator()();
		const auto x = (sample.x() + 1.f) / 2.f;
		const auto y = (sample.y() + 1.f) / 2.f;
		film->addSample({1, 1, 1}, {x, y});
	}
	EXPECT_NO_THROW(film->saveToFile(R"(D:\uniform_hemisphere_test.hdr)"));
}
