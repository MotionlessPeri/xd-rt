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
TEST(ObjLoaderTestSuite, LoadTest)
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

#include "embree4/rtcore.h"

TEST(ObjLoaderTestSuite, EmbreeTest)
{
	ObjLoader loader;
	auto mesh = loader.load(R"(D:\qem-test.obj)");
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

	RTCDevice device = rtcNewDevice(nullptr);
	RTCScene scene = rtcNewScene(device);
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	const auto& positions = mesh->getPositions();
	std::vector<float> vBufferDataCont;
	vBufferDataCont.reserve(positions.size() * 3 + 4);	// +4 for 16-byte padding
	for (const auto& v : positions) {
		vBufferDataCont.emplace_back(v.x());
		vBufferDataCont.emplace_back(v.y());
		vBufferDataCont.emplace_back(v.z());
	}
	for (auto i = 0u; i < 4u; ++i)
		vBufferDataCont.emplace_back(0);
	RTCBuffer vBuffer = rtcNewSharedBuffer(device, (void*)vBufferDataCont.data(),
										   vBufferDataCont.size() * sizeof(float));

	const auto& triangles = mesh->getTriangles();
	std::vector<uint32_t> iBufferDataCont;
	iBufferDataCont.reserve(triangles.size() * 3);
	for (const auto& triangle : triangles) {
		const auto& indices = triangle.getIndices();
		for (auto i : indices)
			iBufferDataCont.emplace_back(i);
	}
	for (auto i = 0u; i < 4u; ++i)
		iBufferDataCont.emplace_back(0);
	RTCBuffer iBuffer = rtcNewSharedBuffer(device, (void*)iBufferDataCont.data(),
										   iBufferDataCont.size() * sizeof(uint32_t));
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vBuffer, 0,
						 3 * sizeof(float), positions.size());
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, iBuffer, 0,
						 3 * sizeof(uint32_t), triangles.size());
	rtcCommitGeometry(geom);
	rtcAttachGeometry(scene, geom);
	rtcReleaseGeometry(geom);
	rtcCommitScene(scene);

	oneapi::tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
											 1);


	auto err= rtcGetDeviceError(device);
	film->clear();
	auto start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  RTCRayHit rayhit;
							  rayhit.ray.org_x = ray.o.x();
							  rayhit.ray.org_y = ray.o.y();
							  rayhit.ray.org_z = ray.o.z();
							  rayhit.ray.dir_x = ray.d.x();
							  rayhit.ray.dir_y = ray.d.y();
							  rayhit.ray.dir_z = ray.d.z();
							  rayhit.ray.tnear = 0.f;
							  rayhit.ray.tfar = std::numeric_limits<float>::infinity();
							  rayhit.ray.mask = -1;
							  rayhit.ray.flags = 0;
							  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
							  rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

							  rtcIntersect1(scene, &rayhit);
							  if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
								  film->addSample({rayhit.ray.tfar, 0, 0}, sample);
							  }
						  }
					  });

	auto end = std::chrono::steady_clock::now();
	err = rtcGetDeviceError(device);
	std::chrono::duration<double> elapsedSeconds = end - start;
	std::cout << "Embree Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_embree_accel.hdr)"););
}

TEST(ObjLoaderTestSuite, MeshHitTest)
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
								  film->addSample({rec.uv.x(), rec.uv.y(), 0}, sample);
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
								  film->addSample({rec.uv.x(), rec.uv.y(), 0}, sample);
							  }
						  }
					  });
	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "BVH Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_bvh_accel.hdr)"););

	RTCDevice device = rtcNewDevice(nullptr);
	RTCScene scene = rtcNewScene(device);
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	const auto& positions = meshWithNoAccel->getPositions();
	std::vector<float> vBufferDataCont;
	vBufferDataCont.reserve(positions.size() * 3 + 4);	// +4 for 16-byte padding
	for (const auto& v : positions) {
		vBufferDataCont.emplace_back(v.x());
		vBufferDataCont.emplace_back(v.y());
		vBufferDataCont.emplace_back(v.z());
	}
	RTCBuffer vBuffer = rtcNewSharedBuffer(device, (void*)vBufferDataCont.data(),
										   vBufferDataCont.size() * sizeof(float));

	const auto& triangles = meshWithNoAccel->getTriangles();
	std::vector<uint32_t> iBufferDataCont;
	iBufferDataCont.reserve(triangles.size() * 3);
	for (const auto& triangle : triangles) {
		const auto& indices = triangle.getIndices();
		for (auto i : indices)
			iBufferDataCont.emplace_back(i);
	}
	RTCBuffer iBuffer = rtcNewSharedBuffer(device, (void*)iBufferDataCont.data(),
										   iBufferDataCont.size() * sizeof(uint32_t));
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vBuffer, 0,
						 3 * sizeof(float), positions.size());
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, iBuffer, 0,
						 3 * sizeof(uint32_t), triangles.size());
	rtcCommitGeometry(geom);
	rtcAttachGeometry(scene, geom);
	rtcReleaseGeometry(geom);
	rtcCommitScene(scene);

	film->clear();
	start = std::chrono::steady_clock::now();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  auto ray = cam->generateRay(sample);
							  RTCRayHit rayhit;
							  rayhit.ray.org_x = ray.o.x();
							  rayhit.ray.org_y = ray.o.y();
							  rayhit.ray.org_z = ray.o.z();
							  rayhit.ray.dir_x = ray.d.x();
							  rayhit.ray.dir_y = ray.d.y();
							  rayhit.ray.dir_z = ray.d.z();
							  rayhit.ray.tnear = 0.f;
							  rayhit.ray.tfar = std::numeric_limits<float>::infinity();
							  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
							  HitRecord rec;

							  rtcIntersect1(scene, &rayhit);
							  if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
								  film->addSample({rayhit.ray.tfar, 0, 0}, sample);
							  }
						  }
					  });
	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "Embree Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_embree_accel.hdr)"););
}
