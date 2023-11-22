//
// Created by Frank on 2023/11/21.
//
#include "Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(TriagulateTestSuite, SphereTriangulateTest)
{
	auto sphere = std::make_shared<Sphere>(Vector3f{1, 1, 1}, 1.f);
	auto triangulated = sphere->triangulate();
}