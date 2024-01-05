//
// Created by Frank on 2024/1/5.
//
#include "MathUtil.h"
#include "distribution/UniformDistribution.h"
#include "gtest/gtest.h"
#include "mapping/EquirectangularMapping.h"
#include "mapping/UVMapping.h"
using namespace xd;
TEST(MappingTestSuite, UVMappingTest)
{
	UVMapping mapping;
	UniformDistribution<2> dis;
	constexpr uint32_t TIMES = 1000u;
	for ([[maybe_unused]] const auto i : std::views::iota(0u, TIMES)) {
		const auto sample = dis.sample();
		EXPECT_EQ(mapping.map(sample), sample);
	}
}

TEST(MappingTestSuite, UVMappingTransformTest)
{
	const UVMapping mapping{2, 3, 1, 1, toRadians(45.f)};
	{
		const Vector2f sample = Vector2f{1, 1}.normalized();
		const auto res = mapping.map(sample);
		const Vector2f expected{1, 4};
		EXPECT_TRUE(res.isApprox(expected));
	}
	{
		const Vector2f sample = Vector2f{0, 0};
		const auto res = mapping.map(sample);
		const Vector2f expected{1, 1};
		EXPECT_TRUE(res.isApprox(expected));
	}
	{
		const Vector2f sample = Vector2f{1, 0};
		const auto res = mapping.map(sample);
		const float sqrt2 = std::sqrtf(2.f);
		const Vector2f expected{sqrt2 + 1.f, 1.5f * sqrt2 + 1.f};
		EXPECT_TRUE(res.isApprox(expected));
	}
}

TEST(MappingTestSuite, EquirectangularMappingTest)
{
	const EquirectangularMapping mapping{};
	{
		const Vector3f sample = Vector3f{1, 0, 0};
		const auto res = mapping.map(sample);
		const Vector2f expected{0.f, 0.5f};
		EXPECT_TRUE(res.isApprox(expected));
	}
	{
		const Vector3f sample = Vector3f{1, 1, 0}.normalized();
		const auto res = mapping.map(sample);
		const Vector2f expected{0.125f, 0.5f};
		EXPECT_TRUE(res.isApprox(expected));
	}
	{
		const Vector3f sample = Vector3f{0, 0, 1}.normalized();
		const auto res = mapping.map(sample);
		const Vector2f expected{0.f, 0.f};
		EXPECT_TRUE(res.isApprox(expected));
	}
	{
		const Vector3f sample = Vector3f{1, 1, 1}.normalized();
		const auto res = mapping.map(sample);
		const float sqrt3 = std::sqrtf(3.f);
		const Vector2f expected{0.125f, std::acosf(sqrt3 / 3.f) * INV_PI};
		EXPECT_TRUE(res.isApprox(expected));
	}
}