//
// Created by Frank on 2023/9/8.
//
#include <oneapi/tbb.h>
#include <numeric>
#include "Loader/MeshLoader.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "loader/ObjMeshLoader.h"
#include "model/Triangle.h"
using namespace xd;
TEST(ObjLoaderTestSuite, LoadTest)
{
	ObjMeshLoader loader;
	auto mesh = loader.load(R"(D:\qem-simple.obj)");
	const auto& positions = mesh->getPositions();
	const auto& uvs = mesh->getUVs();
	const auto& normals = mesh->getNormals();
	const auto& tangents = mesh->getTangents();
	EXPECT_EQ(positions.size(), 36);
	EXPECT_TRUE(uvs.empty());
	EXPECT_TRUE(normals.empty());
	EXPECT_TRUE(tangents.empty());
}

#include "Film.h"
#include "embree4/rtcore.h"
TEST(ObjLoaderTestSuite, EmbreeTest)
{
	ObjMeshLoader loader;
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

	RTCDevice device = rtcNewDevice(nullptr);
	RTCScene scene = rtcNewScene(device);
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	const auto& positions = mesh->getPositions();
	RTCBuffer vBuffer =
		rtcNewSharedBuffer(device, (void*)positions.data(), positions.size() * sizeof(float));

	const auto& triangles = mesh->getTriangles();
	const auto& indices = mesh->getIndices();
	RTCBuffer iBuffer =
		rtcNewSharedBuffer(device, (void*)indices.data(), indices.size() * sizeof(uint32_t));

	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vBuffer, 0,
						 3 * sizeof(float), positions.size());
	rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, iBuffer, 0,
						 3 * sizeof(uint32_t), triangles.size());
	rtcCommitGeometry(geom);
	rtcAttachGeometry(scene, geom);
	rtcReleaseGeometry(geom);
	rtcCommitScene(scene);

	RTCBounds rtcBounds{};
	rtcGetSceneBounds(scene, &rtcBounds);
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
				RTCRayHit rayhit{};
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
					tile->addSample({rayhit.ray.tfar, 0, 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds = end - start;
	std::cout << "Embree Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_embree_accel.hdr)"););
}

TEST(ObjLoaderTestSuite, MeshHitTest)
{
	ObjMeshLoader loader;
	auto meshWithNoAccel = loader.load(R"(D:\qem-test.obj)");
	auto meshWithBVHAccel = loader.load(R"(D:\qem-test.obj)", {HitAccelMethod::BVH});
	constexpr uint32_t width = 200u;
	constexpr uint32_t height = 200u;
	const Vector3f center{0, 0, -2};
	const Vector3f origin{0, 0, 0};
	const float rightNorm = 1.5f;
	const Vector3f right{rightNorm, 0, 0};
	const Vector3f up{0, rightNorm / width * height, 0};

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

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
				if (meshWithNoAccel->hit(ray, rec)) {
					tile->addSample({rec.geom.uv.x(), rec.geom.uv.y(), 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds{end - start};
	std::cout << "NoAccel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_no_accel.hdr)"););

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
				if (meshWithBVHAccel->hit(ray, rec)) {
					tile->addSample({rec.geom.uv.x(), rec.geom.uv.y(), 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "BVH Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_bvh_accel.hdr)"););

	RTCDevice device = rtcNewDevice(nullptr);
	RTCScene scene = rtcNewScene(device);
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	const auto& positions = meshWithNoAccel->getPositions();
	RTCBuffer vBuffer =
		rtcNewSharedBuffer(device, (void*)positions.data(), positions.size() * sizeof(float));

	const auto& triangles = meshWithNoAccel->getTriangles();
	const auto& indices = meshWithNoAccel->getIndices();
	RTCBuffer iBuffer =
		rtcNewSharedBuffer(device, (void*)indices.data(), indices.size() * sizeof(uint32_t));

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
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				RTCRayHit rayhit{};
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
					tile->addSample({rayhit.ray.tfar, 0, 0}, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});

	end = std::chrono::steady_clock::now();
	elapsedSeconds = end - start;
	std::cout << "Embree Accel mesh cost " << elapsedSeconds.count() << " seconds.\n";
	EXPECT_NO_THROW(film->saveToFile(R"(D:\obj_load_and_hit_embree_accel.hdr)"););
}
