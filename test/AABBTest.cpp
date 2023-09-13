//
// Created by Frank on 2023/9/10.
//
#include "AABB.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(AABBTestSuite, hitWithParamTest)
{
	const Vector3f minPoint{0, 0, 0};
	const Vector3f maxPoint{1, 1, 1};
	AABB aabb{minPoint, maxPoint};

	const Vector3f o0 = minPoint;
	const Vector3f d0{0, 0, -1};
	const Ray ray0{o0, d0};
	const auto res0 = aabb.hitWithParams(ray0);
	EXPECT_FALSE(res0.hit);

	const Vector3f o1 = minPoint;
	const Vector3f d1 = Vector3f{1, 1, 1}.normalized();
	const Ray ray1{o1, d1};
	const auto res1 = aabb.hitWithParams(ray1);
	EXPECT_TRUE(res1.hit);
	EXPECT_FLOAT_EQ(res1.tMin, 0.f);
	const float sqrt3 = std::sqrtf(3);
	EXPECT_FLOAT_EQ(res1.tMax, sqrt3);

	const Vector3f o2{0.5, 0.5, 0.5};
	const Vector3f d2{0, 0, 1};
	const Ray ray2{o2, d2};
	const auto res2 = aabb.hitWithParams(ray2);
	EXPECT_TRUE(res2.hit);
	EXPECT_FLOAT_EQ(res2.tMin, -0.5f);
	EXPECT_FLOAT_EQ(res2.tMax, 0.5f);

	const Vector3f o3{-0.5, 0.5, 0.5};
	const Vector3f d3{1, 0, 0};
	const Ray ray3{o3, d3};
	const auto res3 = aabb.hitWithParams(ray3);
	EXPECT_TRUE(res3.hit);
	EXPECT_FLOAT_EQ(res3.tMin, 0.5f);
	EXPECT_FLOAT_EQ(res3.tMax, 1.5f);
}

TEST(AABBTestSuite, isInsideTest)
{
	const Vector3f minPoint{0, 0, 0};
	const Vector3f maxPoint{1, 1, 1};
	AABB aabb{minPoint, maxPoint};

	const Vector3f P0{0.5, 0.5, 0.5};
	EXPECT_TRUE(aabb.isInside(P0));

	const Vector3f P1{0, 0, 0};
	EXPECT_TRUE(aabb.isInside(P1));

	const Vector3f P2{-1, -1, -1};
	EXPECT_FALSE(aabb.isInside(P2));
}

TEST(AABBTestSuite, isIntersectedTest)
{
	const Vector3f min0{0, 0, 0};
	const Vector3f max0{1, 1, 1};
	const AABB aabb0{min0, max0};

	// rhs is "less than"(pos under and left) lhs and isec
	const Vector3f min1{-1, -1, -1};
	const Vector3f max1{0.5, 0.5, 0.5};
	const AABB aabb1{min1, max1};
	EXPECT_TRUE(aabb0.isIntersected(aabb1));

	// rhs is "less than"(pos under and left) lhs and not isec
	const Vector3f min2{-1, -1, -1};
	const Vector3f max2{-0.5, -0.5, -0.5};
	const AABB aabb2{min2, max2};
	EXPECT_FALSE(aabb0.isIntersected(aabb2));

	// rhs contains lhs
	const Vector3f min3{-1, -1, -1};
	const Vector3f max3{2, 2, 2};
	const AABB aabb3{min3, max3};
	EXPECT_TRUE(aabb0.isIntersected(aabb3));

	// rhs inside lhs
	const Vector3f min4{0.2, 0.2, 0.2};
	const Vector3f max4{0.4, 0.3, 0.5};
	const AABB aabb4{min4, max4};
	EXPECT_TRUE(aabb0.isIntersected(aabb4));

	// rhs is "greater than"(pos over and right) lhs and isec
	EXPECT_TRUE(aabb1.isIntersected(aabb0));

	// rhs is "greater than"(pos over and right) lhs and not isec
	EXPECT_FALSE(aabb2.isIntersected(aabb0));

	// aabb is always isec to itself
	EXPECT_TRUE(aabb0.isIntersected(aabb0));

	const Vector3f min5{-1, 0, 0};
	const Vector3f max5{0, 1, 1};
	const AABB aabb5{min5, max5};
	EXPECT_TRUE(aabb0.isIntersected(aabb5));
}

TEST(AABBTestSuite, addPointTest)
{
	AABB aabb;
	EXPECT_FALSE(aabb.isValid());
	const Vector3f maxVec{FLT_MAX, FLT_MAX, FLT_MAX};
	const Vector3f minVec = -maxVec;
	EXPECT_TRUE(aabb.getMinPoint().isApprox(maxVec));
	EXPECT_TRUE(aabb.getMaxPoint().isApprox(minVec));

	const Vector3f p0{-1, 1, 2};
	aabb.addPoint(p0);
	EXPECT_TRUE(aabb.isValid());
	EXPECT_TRUE(aabb.getMinPoint().isApprox(p0));
	EXPECT_TRUE(aabb.getMaxPoint().isApprox(p0));

	const Vector3f p1{2, -3, -4};
	aabb.addPoint(p1);
	const Vector3f expectedMax{2, 1, 2};
	const Vector3f expectedMin{-1, -3, -4};
	EXPECT_TRUE(aabb.getMinPoint().isApprox(expectedMin));
	EXPECT_TRUE(aabb.getMaxPoint().isApprox(expectedMax));

	const Vector3f in{0, 0, 0};
	aabb.addPoint(in);
	EXPECT_TRUE(aabb.getMinPoint().isApprox(expectedMin));
	EXPECT_TRUE(aabb.getMaxPoint().isApprox(expectedMax));
}