//
// Created by Frank on 2023/9/4.
//
#include <random>
#include "../src/core/HitRecord.h"
#include "../src/core/Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(HitRecordTestSuite, DifferentialTest)
{
	Vector3f center{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(center, 1.f);
	constexpr uint32_t TIMES = 1000u;
	uint32_t cnt = 0u;
	std::uniform_real_distribution<float> distrib;
	std::mt19937 generator;
	while (cnt < TIMES) {
		Vector3f dir =
			Vector3f{distrib(generator), distrib(generator), distrib(generator)}.normalized();
		HitRecord rec;
		Ray ray{center, dir};
		EXPECT_TRUE(sphere->hit(ray, rec));
		const auto localToWorld = rec.getLocalToWorld();
		EXPECT_FLOAT_EQ(localToWorld.col(0).norm(), 1.f);
		EXPECT_FLOAT_EQ(localToWorld.col(1).norm(), 1.f);
		EXPECT_FLOAT_EQ(localToWorld.col(2).norm(), 1.f);
		++cnt;
	}
}