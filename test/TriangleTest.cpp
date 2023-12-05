//
// Created by Frank on 2023/9/5.
//
#include <oneapi/tbb.h>
#include "Film.h"
#include "Sampler.h"
#include "Triangle.h"
#include "camera/CameraFactory.h"
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

	const auto transform = [](const Vector2f& sample) -> Vector3f {
		const auto offsetted = 2 * (sample - Vector2f{0.5, 0.5});
		return {offsetted.x(), -offsetted.y(), 0};
	};

	Vector2i resolution{width, height};
	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const Vector2f sample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const Vector3f worldPos = film->getWorldCoordsFromSample(sample);
				const Vector3f baryCoord = triangle.getBarycentricCoordUnchecked(worldPos);
				const auto color = colorMat * baryCoord;
				tile->addSample(color, sample);
			}
			film->mergeTileToFilm(std::move(tile));
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

	tbb::parallel_for(
		tbb::blocked_range2d<int, int>(0, height, 0, width),
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			for (const auto pixel : *tile) {
				const auto sample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(sample);
				HitRecord rec;

				if (triangle.hit(ray, rec)) {
					const auto worldPos = ray.getTPoint(rec.tHit);
					const auto baryCoord = triangle.getBarycentricCoordUnchecked(worldPos);
					const auto color = colorMat * baryCoord;
					tile->addSample(baryCoord, sample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});

	film->saveToFile(R"(D:\triangle_hit_test.hdr)");
}