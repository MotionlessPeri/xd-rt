//
// Created by Frank on 2023/9/5.
//
#include <oneapi/tbb.h>
#include "../src/camera/CameraFactory.h"
#include "../src/core/Film.h"
#include "../src/core/Sampler.h"
#include "../src/core/Triangle.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(TriangleTestSuite, BaryCentricCoordTest)
{
	const float sqrt3 = std::sqrtf(3);
	TriangleMesh mesh{{0, 1, 0, -sqrt3 / 2.f, -0.5f, 0, sqrt3 / 2.f, -0.5f, 0},
					  {1, 1, 0, 0, 0, 1},
					  {},
					  {},
					  {0, 1, 2}};
	auto triangles = mesh.getTriangles();
	EXPECT_EQ(triangles.size(), 1);
	const auto& triangle = triangles.front();
	const ColorRGB colorA{1, 0, 0};
	const ColorRGB colorB{0, 1, 0};
	const ColorRGB colorC{0, 0, 1};
	Matrix3f colorMat;
	colorMat << colorA, colorB, colorC;

	const Vector3f center{0, 0, 0};
	const Vector3f right{1, 0, 0};
	const Vector3f up{0, 1, 0};
	const uint32_t width = 1000u;
	const uint32_t height = 1000u;
	auto film = std::make_shared<Film>(center, right, up, width, height);
	auto sampler = std::make_shared<SimpleSampler>(width, height);
	const auto transform = [](const Vector2f& sample) -> Vector3f {
		const auto offsetted = 2 * (sample - Vector2f{0.5, 0.5});
		return {offsetted.x(), -offsetted.y(), 0};
	};
	const auto samples = sampler->generateSamples();
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, samples.size()), [&](const tbb::blocked_range<size_t>& r) {
			for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
				const auto& sample = samples[sampleIdx];
				const auto worldPos = transform(sample);
				const auto baryCoord = triangle.getBarycentricCoordUnchecked(worldPos);
				const auto color = colorMat * baryCoord;
				film->addSample(baryCoord, sample);
			}
		});
	film->saveToFile(R"(D:\bary_centric_coord_test.hdr)");
}

TEST(TriangleTestSuite, HitTest)
{
	const float sqrt3 = std::sqrtf(3);
	TriangleMesh mesh{{0, 1, 0, -sqrt3 / 2.f, -0.5f, 0, sqrt3 / 2.f, -0.5f, 0},
					  {1, 1, 0, 0, 0, 1},
					  {},
					  {},
					  {0, 1, 2}};
	auto triangles = mesh.getTriangles();
	EXPECT_EQ(triangles.size(), 1);
	const auto& triangle = triangles.front();
	const ColorRGB colorA{1, 0, 0};
	const ColorRGB colorB{0, 1, 0};
	const ColorRGB colorC{0, 0, 1};
	Matrix3f colorMat;
	colorMat << colorA, colorB, colorC;

	const Vector3f origin{0, 0, 0};
	const Vector3f center{0, 0, -1};
	const Vector3f right{1.25, 0, 0};
	const Vector3f up{0, 1, 0};
	const uint32_t width = 1000u;
	const uint32_t height = 800u;
	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();
	auto sampler = std::make_shared<SimpleSampler>(width, height);
	const auto samples = sampler->generateSamples();
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, samples.size()), [&](const tbb::blocked_range<size_t>& r) {
			for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
				const auto& sample = samples[sampleIdx];
				const auto ray = cam->generateRay(sample);
				HitRecord rec;

				if (triangle.hit(ray, rec)) {
					const auto worldPos = ray.getTPoint(rec.tHit);
					const auto baryCoord = triangle.getBarycentricCoordUnchecked(worldPos);
					const auto color = colorMat * baryCoord;
					film->addSample(baryCoord, sample);
				}
			}
		});
	film->saveToFile(R"(D:\triangle_hit_test.hdr)");
}