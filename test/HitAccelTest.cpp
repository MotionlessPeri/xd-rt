//
// Created by Frank on 2023/9/12.
//
#include "HitAccel.h"
#include "Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(HitAccelTestSuite, BVHBuildTest)
{
	const Vector3f center{0, 0, 0};
	const float radius = 100.f;
	Sphere sphere{center, radius};
	std::vector<const Model*> models;
	models.emplace_back(&sphere);
	BVHNode bvh{models};
	auto aabb = bvh.getAABB();
	EXPECT_EQ(aabb, sphere.getAABB());
	const auto& bvhModels = bvh.getLeafModels();
	EXPECT_EQ(bvhModels.size(), 1);
	EXPECT_EQ(bvhModels.front(), &sphere);
}

#include <oneapi/tbb.h>
#include <chrono>
#include "Film.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
TEST(HitAccelTestSuite, BVHHitTest1)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	SceneBuilder sb;
	std::vector<std::shared_ptr<Primitive>> prims;
	for (int i = 0u; i < count; ++i) {
		const float x = firstCenter.x() + 2 * radius * i;
		for (int j = 0u; j < count; ++j) {
			const float y = firstCenter.y() + 2 * radius * j;
			for (int k = 0u; k < count; ++k) {
				const float z = firstCenter.z() + 2 * radius * k;
				const Vector3f center{x, y, z};
				const auto model = std::make_shared<Sphere>(center, radius);
				const auto prim = std::make_shared<Primitive>(model, nullptr);
				sb.addPrimitive(prim);
			}
		}
	}

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const float sqrt3 = std::sqrtf(3);
	const Vector3f center = Vector3f{1, 1, 1} * halfLen * sqrt3;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const float rightNorm = 500.f;
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();

	const auto scene = sb.build();
	const NaiveHitSolver naiveSolver{scene};
	const BVHHitSolver bvhSolver{scene};

	auto start = std::chrono::steady_clock::now();
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				HitRecord rec;
				if (naiveSolver.hit(ray, rec)) {
					tile->addSample({rec.tHit, 0, 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds{end - start};
	std::cout << "Naive solver cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\bvh_hit_test_naive_solver.hdr)"););

	film->clear();
	start = std::chrono::steady_clock::now();
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				HitRecord rec;
				if (bvhSolver.hit(ray, rec)) {
					tile->addSample({rec.tHit, 0, 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "BVH solver cost " << elapsedSeconds.count() << " seconds.\n";

	EXPECT_NO_THROW(film->saveToFile(R"(D:\bvh_hit_test_bvh_solver.hdr)"););
}

#include "Loader/MeshLoader.h"
#include "Triangle.h"
TEST(HitAccelTestSuite, EmbreeHitTest1)
{
	ObjMeshLoader loader;
	auto mesh = loader.load(R"(D:\qem-test.obj)");
	auto prim = std::make_shared<Primitive>(mesh, nullptr);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center{0, 0, -2};
	const Vector3f origin{0, 0, 0};
	const float rightNorm = 1.5f;
	const Vector3f right{rightNorm, 0, 0};
	const Vector3f up{0, rightNorm / width * height, 0};

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	SceneBuilder sb;
	sb.addPrimitive(prim);
	const auto scene = sb.build();
	EmbreeHitSolver embreeSolver{scene};

	film->clear();

	auto start = std::chrono::steady_clock::now();
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				HitRecord rec;
				if (embreeSolver.hit(ray, rec)) {
					tile->addSample({rec.tHit, 0, 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds = end - start;
	std::cout << "Embree Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\embree_hit_solver_test_1.hdr)"););
}