//
// Created by Frank on 2023/9/4.
//
#include <random>

#include "Film.h"
#include "HitRecord.h"
#include "Primitive.h"
#include "gtest/gtest.h"
#include "model/Sphere.h"
using namespace xd;
TEST(HitRecordTestSuite, DifferentialTest)
{
	Vector3f center{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(1.f);
	constexpr uint32_t TIMES = 1000u;
	uint32_t cnt = 0u;
	std::uniform_real_distribution<float> distrib;
	std::mt19937 generator(std::chrono::steady_clock::now().time_since_epoch().count());
	while (cnt < TIMES) {
		Vector3f dir =
			Vector3f{distrib(generator), distrib(generator), distrib(generator)}.normalized();
		HitRecord rec;
		Ray ray{center, dir};
		EXPECT_TRUE(sphere->hit(ray, rec));
		const auto localToModel = rec.getCurrentFrame();
		EXPECT_FLOAT_EQ(localToModel.col(0).norm(), 1.f);
		EXPECT_FLOAT_EQ(localToModel.col(1).norm(), 1.f);
		EXPECT_FLOAT_EQ(localToModel.col(2).norm(), 1.f);
		++cnt;
	}
}
#include "camera/CameraFactory.h"
#include "camera/OrthoCamera.h"
#include "material/MatteMaterial.h"
TEST(HitRecordTestSuite, FrameCategoryTest)
{
	Vector3f center{0, 0, 0};
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto matte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	constexpr uint32_t width = 100u;
	constexpr uint32_t height = 100u;
	const Vector3f cameraPos = Vector3f{0, 2, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const float rightNorm = 1.f;
	const Vector3f towards = (target - cameraPos).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);
	auto cam = CameraFactory::createOrthoCamera(cameraPos, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();
	auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	int cnt = 0;
	for (const auto pixel : *tile) {
		const auto pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
		const auto primRay = cam->generateRay(pixelSample);
		HitRecord primRec{};
		bool shapeHit, primHit;
		shapeHit = primHit = false;
		if (sphere->hit(primRay, primRec)) {
			++cnt;
			shapeHit = true;
			EXPECT_EQ(primRec.frame, FrameCategory::MODEL);
		}
		primRec = {};
		if (prim->hit(primRay, primRec)) {
			primHit = true;
			EXPECT_EQ(primRec.frame, FrameCategory::WORLD);
		}
		if (shapeHit != primHit)
			__debugbreak();
		EXPECT_EQ(shapeHit, primHit);
	}
	std::cout << cnt << std::endl;
}