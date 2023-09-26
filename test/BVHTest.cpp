//
// Created by Frank on 2023/9/12.
//
#include "HitAccel.h"
#include "Model.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(BVHTestSuite, buildTest)
{
	const Vector3f center{0, 0, 0};
	const float radius = 100.f;
	Sphere sphere{center, radius};
	std::vector<const Model*> models;
	models.emplace_back(&sphere);
	BVHNode bvh{models};
	auto aabb = bvh.getAABB();
	EXPECT_EQ(aabb, sphere.getAABB());
	const auto& bvhModels = bvh.getModels();
	EXPECT_EQ(bvhModels.size(), 1);
	EXPECT_EQ(bvhModels.front(), &sphere);
}

#include <oneapi/tbb.h>
#include <chrono>
#include "CameraFactory.h"
#include "Primitive.h"
#include "Scene.h"
TEST(BVHTestSuite, hitTest1)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	auto scene = std::make_shared<Scene>();
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
				scene->addPrimitive(prim);
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
	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	const NaiveHitSolver naiveSolver{scene};
	const BVHHitSolver bvhSolver{scene};

	auto start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (naiveSolver.solve(ray, rec)) {
								  film->addSample({rec.tHit, 0, 0}, sample);
							  }
						  }
					  });
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds{end - start};
	std::cout << "Naive solver cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\bvh_hit_test_naive_solver.hdr)"););

	film->clear();
	start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (bvhSolver.solve(ray, rec)) {
								  film->addSample({rec.tHit, 0, 0}, sample);
							  }
						  }
					  });

	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "BVH solver cost " << elapsedSeconds.count() << " seconds.\n";

	EXPECT_NO_THROW(film->saveToFile(R"(D:\bvh_hit_test_bvh_solver.hdr)"););
}
