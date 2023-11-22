//
// Created by Frank on 2023/8/20.
//
#include <oneapi/tbb.h>
#include <thread>
#include "../src/camera/CameraFactory.h"
#include "../src/core/Camera.h"
#include "../src/core/Film.h"
#include "../src/core/Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(VisibilityTestSuite, OrthoCamVisTest)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(sphereCenter, radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, sphereCenter, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	const auto threadCnt = std::thread::hardware_concurrency();
	std::atomic<uint32_t> remains = threadCnt;
	for (auto i = 0; i < threadCnt; ++i) {
		std::thread th{[&](int index) {
						   for (int sampleIdx = index; sampleIdx < samples.size();
								sampleIdx += threadCnt) {
							   const auto& sample = samples[sampleIdx];
							   const auto ray = cam->generateRay(sample);
							   HitRecord rec;
							   if (sphere->hit(ray, rec)) {
								   film->addSample({1, 1, 1}, sample);
							   }
						   }
						   --remains;
					   },
					   i};
		th.detach();
	}
	while (remains > 0) {
	}

	const std::string hdrPath = R"(D:\sphere_ortho_visible_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(VisibilityTestSuite, OrthoCamVisTestTBB)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(sphereCenter, radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, sphereCenter, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  const auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (sphere->hit(ray, rec)) {
								  film->addSample({1, 1, 1}, sample);
							  }
						  }
					  });

	const std::string hdrPath = R"(D:\sphere_ortho_visible_test_tbb.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(VisibilityTestSuite, PerspCamVisTestTBB)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(sphereCenter, radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, 0};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 800.f / std::sqrtf(3.), 0};

	//	auto film = std::make_shared<Film>(center, right, up, width, height);
	const Vector3f camPos{0, 0, -2 * radius};
	//	PerspCamera cam{camPos, film};
	constexpr float PI = 3.1415927f;
	const float verticalFov = 60.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(camPos, sphereCenter, up.normalized(), verticalFov,
												right.norm() * 1.f / up.norm(), width, height);
	auto film = cam->getFilm();
	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  const auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (sphere->hit(ray, rec)) {
								  film->addSample(ColorRGB{rec.tHit, 0, 0}, sample);
							  }
						  }
					  });

	const std::string hdrPath = R"(D:\sphere_persp_visible_test_tbb.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "../src/core/Primitive.h"
#include "../src/core/Scene.h"
TEST(VisibilityTestSuite, OrthoCamVisTestTBBWithHitSolver)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(Vector3f{-50, 0, 0}, radius);
	auto greenSphere = std::make_shared<Sphere>(Vector3f{50, 0, 0}, radius);
	auto prim1 = std::make_shared<Primitive>(redSphere, nullptr);
	auto prim2 = std::make_shared<Primitive>(greenSphere, nullptr);

	auto scene = std::make_shared<Scene>();
	scene->addPrimitive(prim1);
	scene->addPrimitive(prim2);

	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	constexpr uint32_t width = 100u;
	constexpr uint32_t height = 80u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  const auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (hitSolver->solve(ray, rec)) {
								  if (rec.primitive->getModel() == redSphere) {
									  film->addSample({1, 0, 0}, sample);
								  }
								  if (rec.primitive->getModel() == greenSphere) {
									  film->addSample({0, 1, 0}, sample);
								  }
							  }
						  }
					  });
	const std::string hdrPath = R"(D:\sphere_ortho_visible_test_tbb_with_HitSolver.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}