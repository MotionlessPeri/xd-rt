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