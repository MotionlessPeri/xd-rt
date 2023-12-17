//
// Created by Frank on 2023/8/19.
//

#include "Film.h"
#include "Ray.h"
#include "camera/OrthoCamera.h"
#include "camera/PerspCamera.h"
#include "gtest/gtest.h"
using namespace xd;

TEST(CameraTestSuite, PerspectiveCameraTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{0, 0, 1.5};
	const Vector3f up{0, 0.5, 0};
	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	const auto film = std::make_shared<Film>(center, right, up, width, height);
	const auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	const Vector3f camPos{-0.5, 0, 0};
	PerspCamera camera{camPos, film};

	EXPECT_EQ(tile->size(), width * height);

	Vector3f centers[] = {{0, 0.25, -1},  {0, 0.25, 0},	 {0, 0.25, 1},
						  {0, -0.25, -1}, {0, -0.25, 0}, {0, -0.25, 1}};
	for (auto i = 0u; i < width * height; ++i) {
		const Vector2f sample = (*tile)[i].cast<float>() + Vector2f{0.5, 0.5};
		const auto ray = camera.generateRay(sample);
		EXPECT_TRUE(ray.o.isApprox(camPos));
		EXPECT_TRUE(ray.d.isApprox((centers[i] - camPos).normalized()));
	}
}

TEST(CameraTestSuite, OrthoCameraTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f right{1.5, 0, 0};
	const Vector3f up{0, 0.5, 0};
	const Vector3f towards = up.cross(right).normalized();

	constexpr uint32_t width = 3;
	constexpr uint32_t height = 2;
	auto film = std::make_shared<Film>(center, right, up, width, height);
	const auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	const Vector3f camPos{-0.5, 0, 0};
	OrthoCamera camera{film};

	EXPECT_EQ(tile->size(), width * height);

	Vector3f centers[] = {{-1, 0.25, 0},  {0, 0.25, 0},	 {1, 0.25, 0},
						  {-1, -0.25, 0}, {0, -0.25, 0}, {1, -0.25, 0}};
	for (auto i = 0u; i < width * height; ++i) {
		const auto sample = (*tile)[i].cast<float>() + Vector2f{0.5, 0.5};
		auto ray = camera.generateRay(sample);
		EXPECT_TRUE(ray.o.isApprox(centers[i]));
		EXPECT_TRUE(ray.d.isApprox(towards));
	}
}
