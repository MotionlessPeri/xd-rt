//
// Created by Frank on 2023/12/7.
//
#include "MathUtil.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(MathUtilTestSuite, NextFloatTest)
{
	EXPECT_EQ(nextFloatUp(-0.f), 0.f);
	EXPECT_EQ(nextFloatDown(0.f), -0.f);
	const auto down1 = std::bit_cast<float>(0b01000000011111111111111111111111);
	const auto up1 = std::bit_cast<float>(0b01000000100000000000000000000000);
	EXPECT_EQ(nextFloatUp(down1), up1);
	EXPECT_EQ(nextFloatDown(up1), down1);
	const auto down2 = std::bit_cast<float>(0b01000000010010010000111111011011);
	const auto up2 = std::bit_cast<float>(0b01000000010010010000111111011100);
	EXPECT_EQ(nextFloatUp(down2), up2);
	EXPECT_EQ(nextFloatDown(up2), down2);
}

TEST(MathUtilTestSuite, applyTransformTest)
{
	Vector3f translation{100, 100, 100};
	Transform t{Eigen::Translation3f{100, 100, 100}};

	Vector3f o{0, 0, 0};
	Vector3f ePoint{0, 0, 0};
	EXPECT_TRUE(isExact(ePoint));
	applyTransformToPoint(t, o, ePoint);
	EXPECT_TRUE(o.isApprox(translation));

	auto ePoint2nd = ePoint;
	applyTransformToPoint(t, o, ePoint2nd);
	EXPECT_TRUE(ePoint2nd.cwiseGreaterOrEqual(ePoint).all());

	Vector3f d{0, 0, 1};
	Vector3f eDir{0, 0, 0};
	auto dCopy = d;
	applyTransformToDirection(t, dCopy, eDir);
	EXPECT_TRUE(d.isApprox(dCopy));
	EXPECT_TRUE(eDir.cwiseGreaterOrEqual(Vector3f{0, 0, 0}).all());

	auto eDir2nd = eDir;
	applyTransformToDirection(t, dCopy, eDir2nd);
	EXPECT_TRUE(eDir2nd.cwiseGreaterOrEqual(eDir).all());
}