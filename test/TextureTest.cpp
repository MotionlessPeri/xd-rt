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
	auto textureF = std::make_shared<ConstantTextureF>(fParam);
	auto textureC = std::make_shared<ConstantTextureColor>(cParam);
	EXPECT_EQ(textureF->sample(samplePos), fParam);
	EXPECT_TRUE(cParam.isApprox(textureC->sample(samplePos)));
}

#include <oneapi/tbb.h>
#include "CameraFactory.h"
#include "Model.h"
#include "stb_image.h"
TEST(DomeTestSuite, SphereTextureTest)
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
	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()),
					  [&](const tbb::blocked_range<size_t>& r) {
						  for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
							  const auto& sample = samples[sampleIdx];
							  const auto ray = cam->generateRay(sample);
							  HitRecord rec;
							  film->addSample(texture.sample(ray.d), sample);
						  }
					  });
	const std::string hdrPath = R"(D:\sphere_texture_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}