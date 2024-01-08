//
// Created by Frank on 2023/9/2.
//
#include "Macros.h"
#include "gtest/gtest.h"
#include "loader/TextureFactory.h"
#include "texture/ConstantTexture.h"
using namespace xd;
TEST(TextureTestSuite, ConstantTextureSampleTest)
{
	const float fParam = 0.6f;
	const Vector3f cParam{0.1f, 0.7f, 0.2f};
	const auto cTextureF = std::make_shared<ConstantTexture>(fParam);
	const auto cTextureC = std::make_shared<ConstantTexture>(cParam);
	EXPECT_EQ(cTextureF->sample(TextureEvalContext{}).x(), fParam);
	EXPECT_EQ(cTextureC->sample(TextureEvalContext{}).head<3>(), cParam);
}

#include <oneapi/tbb.h>
#include "HitRecord.h"
#include "camera/CameraFactory.h"
#include "stb_image.h"
TEST(TextureTestSuite, SphereTextureTest)
{
	const auto texture = TextureFactory::get().loadSphereTexture(R"(D:\dome.hdr)");
	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 0, 500};

	const float theta = 90.f / 180.f * PI;
	const Vector3f camPos{std::cosf(theta), std::sinf(theta), 0};
	const Vector3f target{0, 0, 0};
	const float verticalFov = 90.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(camPos, target, up.normalized(), verticalFov,
												right.norm() / up.norm(), width, height);
	auto film = cam->getFilm();
	TBB_SERIAL
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
				tile->addSample(texture->sample(TextureEvalContext{ray.d}).head<3>(), pixelSample);
			}
			film->mergeTileToFilm(std::move(tile));
		});
	const std::string hdrPath = R"(D:\sphere_texture_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "model/Triangle.h"
TEST(TextureTestSuite, UVTextureTest)
{
	const float sqrt3 = std::sqrtf(3);
	TriangleMesh mesh{{0, 1, 0, -sqrt3 / 2.f, -0.5f, 0, sqrt3 / 2.f, -0.5f, 0},
					  {0.5, 0, 0, sqrt3 / 2, 1, sqrt3 / 2},
					  {},
					  {},
					  {0, 1, 2}};

	const auto texture = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	const Vector3f origin{0, 0, 0};
	const Vector3f center{0, 0, -1};
	const Vector3f right{1.25f, 0, 0};
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
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				HitRecord rec;
				if (mesh.hit(ray, rec)) {
					const auto color = texture->sample(TextureEvalContext{rec.geom}).head<3>();
					tile->addSample(color, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	film->saveToFile(R"(D:\uv_texture_test.hdr)");
}

TEST(TextureTestSuite, TentFilterTest)
{
	const Vector3f center{0, 0, 0};
	const Vector3f up{0, 1, 0};
	const Vector3f right{1, 0, 0};

	LoadTextureOptions options;
	options.filterType = FilterType::NEAREST;
	using namespace std::string_literals;
	const auto imagePath = R"(D:\filter_test_image.png)"s;
	const auto nearestTexture = TextureFactory::get().loadUVTexture(imagePath, options);
	options.filterType = FilterType::TENT;
	const auto tentTexture = TextureFactory::get().loadUVTexture(imagePath, options);

	// magnification
	{
		constexpr uint32_t width = 1000u;
		constexpr uint32_t height = 1000u;
		Film nearestFilm{center, right, up, width, height};
		Film tentFilm{center, right, up, width, height};
		auto nTile = nearestFilm.getTile({0, 0}, {width - 1, height - 1});
		auto tTile = tentFilm.getTile({0, 0}, {width - 1, height - 1});
		for (const auto& pixel : *nTile) {
			const Vector2f pixelCenter = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
			TextureEvalContext ctx;
			ctx.uv = pixelCenter.cwiseQuotient(nearestFilm.getResolution().cast<float>());
			nTile->addSample(nearestTexture->sample(ctx).head<3>(), pixelCenter);
			tTile->addSample(tentTexture->sample(ctx).head<3>(), pixelCenter);
		}
		nearestFilm.mergeTileToFilm(std::move(nTile));
		tentFilm.mergeTileToFilm(std::move(tTile));
		nearestFilm.saveToFile(R"(D:\texture_filter_test_nearest.hdr)");
		tentFilm.saveToFile(R"(D:\texture_filter_test_tent.hdr)");
	}
}