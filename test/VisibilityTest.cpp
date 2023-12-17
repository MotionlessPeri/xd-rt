//
// Created by Frank on 2023/8/20.
//
#include <oneapi/tbb.h>
#include <thread>
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "model/Sphere.h"
using namespace xd;
TEST(VisibilityTestSuite, OrthoCamVisTest)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, sphereCenter, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();
	auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	const auto threadCnt = std::thread::hardware_concurrency();
	std::atomic<uint32_t> remains = threadCnt;
	for (auto i = 0; i < threadCnt; ++i) {
		std::thread th{
			[&](int index) {
				for (int sampleIdx = index; sampleIdx < tile->size(); sampleIdx += threadCnt) {
					const auto sample = (*tile)[sampleIdx].cast<float>() + Vector2f{0.5, 0.5};
					const auto ray = cam->generateRay(sample);
					HitRecord rec;
					if (sphere->hit(ray, rec)) {
						tile->addSample({1, 1, 1}, sample);
					}
				}
				--remains;
			},
			i};
		th.detach();
	}
	while (remains > 0) {
	}

	film->mergeTileToFilm(std::move(tile));
	const std::string hdrPath = R"(D:\sphere_ortho_visible_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath, {}););
}

TEST(VisibilityTestSuite, OrthoCamVisTestTBB)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, sphereCenter, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();
	auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, tile->size()), [&](const tbb::blocked_range<size_t>& r) {
			for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
				const auto sample = (*tile)[sampleIdx].cast<float>() + Vector2f{0.5, 0.5};
				const auto ray = cam->generateRay(sample);
				HitRecord rec;
				if (sphere->hit(ray, rec)) {
					tile->addSample({1, 1, 1}, sample);
				}
			}
		});
	film->mergeTileToFilm(std::move(tile));
	const std::string hdrPath = R"(D:\sphere_ortho_visible_test_tbb.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath, {}););
}

TEST(VisibilityTestSuite, PerspCamVisTestTBB)
{
	constexpr float radius = 400.f;
	const Vector3f sphereCenter{0, 0, 0};
	auto sphere = std::make_shared<Sphere>(radius);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, 0};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 800.f / std::sqrtf(3.), 0};

	//	auto film = std::make_shared<Film>(center, right, up, width, height);
	const Vector3f camPos{0, 0, -2 * radius};
	//	PerspCamera cam{camPos, film};
	const float verticalFov = 60.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(camPos, sphereCenter, up.normalized(), verticalFov,
												right.norm() * 1.f / up.norm(), width, height);
	auto film = cam->getFilm();
	auto tile = film->getTile({0, 0}, {width - 1, height - 1});
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, tile->size()), [&](const tbb::blocked_range<size_t>& r) {
			for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
				const auto& sample = (*tile)[sampleIdx].cast<float>() + Vector2f{0.5, 0.5};
				const auto ray = cam->generateRay(sample);
				HitRecord rec;
				if (sphere->hit(ray, rec)) {
					tile->addSample(ColorRGB{rec.tHit, 0, 0}, sample);
				}
			}
		});
	film->mergeTileToFilm(std::move(tile));
	const std::string hdrPath = R"(D:\sphere_persp_visible_test_tbb.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath, {}););
}

#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "sampler/SimpleSampler.h"
TEST(VisibilityTestSuite, OrthoCamVisTestTBBWithHitSolver)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(radius);
	auto greenSphere = std::make_shared<Sphere>(radius);
	const Transform redSphereTransform{Eigen::Translation3f{-50, 0, 0}};
	const Transform greenSphereTransform{Eigen::Translation3f{50, 0, 0}};
	auto prim1 = std::make_shared<Primitive>(redSphere, nullptr, redSphereTransform);
	auto prim2 = std::make_shared<Primitive>(greenSphere, nullptr, greenSphereTransform);

	SceneBuilder sceneBuilder;
	sceneBuilder.addPrimitive(prim1).addPrimitive(prim2);
	sceneBuilder.setHitSolverType(HitSolverType::NAIVE);
	auto scene = sceneBuilder.build();

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	SimpleSampler sampler(1);
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			auto tileSampler = sampler.clone(tile->getIndexFromSample(topLeft.cast<float>()));
			for (const auto pixel : *tile) {
				const auto sample = pixel.cast<float>() + sampler.sample2D();
				const auto ray = cam->generateRay(sample);
				HitRecord rec;
				if (scene->hit(ray, rec)) {
					if (rec.primitive->getModel() == redSphere) {
						tile->addSample({1, 0, 0}, sample);
					}
					if (rec.primitive->getModel() == greenSphere) {
						tile->addSample({0, 1, 0}, sample);
					}
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	const std::string hdrPath = R"(D:\sphere_ortho_visible_test_tbb_with_HitSolver.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath, {}););
}