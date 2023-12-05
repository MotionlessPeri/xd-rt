//
// Created by Frank on 2023/9/2.
//
#include "gtest/gtest.h"

#include "Sampler.h"
#include "Texture.h"
using namespace xd;
TEST(TextureTestSuite, ConstantTextureSampleTest)
{
	const float fParam = 0.6f;
	const Vector3f cParam{0.1, 0.7, 0.2};
	const Vector2f samplePos{0.4, 0.6};
	auto textureF = std::make_shared<ConstantTexture<float, Vector2f>>(fParam);
	auto textureC = std::make_shared<ConstantTexture<Vector3f, Vector2f>>(cParam);
	EXPECT_EQ(textureF->sample(samplePos), fParam);
	EXPECT_TRUE(cParam.isApprox(textureC->sample(samplePos)));
}

#include <oneapi/tbb.h>
#include "Film.h"
#include "Model.h"
#include "camera/CameraFactory.h"
#include "stb_image.h"
TEST(TextureTestSuite, SphereTextureTest)
{
	int domeWidth, domeHeight, channels;
	auto* rawData = stbi_loadf(R"(D:\dome.hdr)", &domeWidth, &domeHeight, &channels, 0);
	std::vector<Vector3f> domeData{};
	domeData.reserve(domeWidth * domeHeight);
	for (auto i = 0u; i < domeWidth * domeHeight; ++i) {
		domeData.emplace_back(rawData[channels * i], rawData[channels * i + 1],
							  rawData[channels * i + 2]);
	}

	SphereTexture<Vector3f> texture{domeData, (uint32_t)domeWidth, (uint32_t)domeHeight};

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
				tile->addSample(texture.sample(ray.d), pixelSample);
			}
			film->mergeTileToFilm(std::move(tile));
		});
	const std::string hdrPath = R"(D:\sphere_texture_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "Triangle.h"
TEST(TextureTestSuite, UVTextureTest)
{
	const float sqrt3 = std::sqrtf(3);
	TriangleMesh mesh{{0, 1, 0, -sqrt3 / 2.f, -0.5f, 0, sqrt3 / 2.f, -0.5f, 0},
					  {0.5, 0, 0, sqrt3 / 2, 1, sqrt3 / 2},
					  {},
					  {},
					  {0, 1, 2}};

	int imageWidth, imageHeight, imageChannels;
	auto* rawData = stbi_load(R"(D:\uv_checker.jpg)", &imageWidth, &imageHeight, &imageChannels, 0);
	const auto pixelCnt = imageWidth * imageHeight;
	std::vector<float> imgData;
	imgData.reserve(pixelCnt);
	for (auto i = 0u; i < pixelCnt; ++i) {
		imgData.emplace_back((float)rawData[i * imageChannels] / 255.f);
		imgData.emplace_back((float)rawData[i * imageChannels + 1] / 255.f);
		imgData.emplace_back((float)rawData[i * imageChannels + 2] / 255.f);
	}
	UVTextureRGB texture{imgData, (uint32_t)imageWidth, (uint32_t)imageHeight};
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
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
				const auto ray = cam->generateRay(pixelSample);
				HitRecord rec;
				if (mesh.hit(ray, rec)) {
					const auto color = texture.sample(rec.uv);
					tile->addSample(color, pixelSample);
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});
	film->saveToFile(R"(D:\uv_texture_test.hdr)");
}
