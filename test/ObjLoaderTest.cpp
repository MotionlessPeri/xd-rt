//
// Created by Frank on 2023/9/8.
//
#include <oneapi/tbb.h>
#include <numeric>
#include "CameraFactory.h"
#include "MeshLoader.h"
#include "Triangle.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(ObjLoaderTestSuite, loadTest)
{
	ObjLoader loader;
	auto mesh = loader.load(R"(D:\qem-simple.obj)");
	const auto& positions = mesh->getPositions();
	const auto& uvs = mesh->getUvs();
	const auto& normals = mesh->getNormals();
	const auto& tangents = mesh->getTangents();
	const auto& biTangents = mesh->getBiTangents();
	EXPECT_EQ(positions.size(), 12);
	EXPECT_TRUE(uvs.empty());
	EXPECT_TRUE(normals.empty());
	EXPECT_TRUE(tangents.empty());
	EXPECT_TRUE(biTangents.empty());
}

TEST(ObjLoaderTestSuite, meshHitTest)
{
	ObjLoader loader;
	auto meshWithNoAccel = loader.load(R"(D:\qem-test.obj)");
	auto meshWithBVHAccel = loader.load(R"(D:\qem-test.obj)", {HitAccelMethod::BVH});
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

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();

	auto start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (meshWithNoAccel->hit(ray, rec)) {
								  film->addSample({rec.uv.x(), rec.uv.y(), 1}, sample);
							  }
						  }
					  });
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds{end - start};
	std::cout << "NoAccel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_no_accel.hdr)"););

	film->clear();
	start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  if (meshWithBVHAccel->hit(ray, rec)) {
								  film->addSample({(float)rec.debug, 0, 0}, sample);
							  }
						  }
					  });
	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "BVH Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_bvh_accel.hdr)"););
}