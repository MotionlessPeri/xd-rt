//
// Created by Frank on 2023/10/2.
//
#include <oneapi/tbb.h>
#include "../src/camera/CameraFactory.h"
#include "../src/core/HitSolver.h"
#include "../src/core/Light.h"
#include "../src/core/Primitive.h"
#include "../src/loader/GLTFSceneLoader.h"
#include "../src/texture/TextureFactory.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(GLTFSceneLoaderTestSuite, loadTest)
{
	// const std::string path = R"(D:\cube_test.gltf)";
	const std::string path = R"(D:\buster_drone\scene.gltf)";
	GLTFSceneLoader loader;
	auto scene = loader.load(path);
	auto hitSolver = std::make_shared<EmbreeHitSolver>(scene);

	constexpr uint32_t width = 2000u;
	constexpr uint32_t height = 2000u;
	const Vector3f center{0, 0, 1.5};
	// const Vector3f center{0, 0, 15};
	const Vector3f target{0, -1, 0};
	// const Vector3f target{0, 0, 0};
	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(domeLight);

	const auto verticalFov = 90.f / 180.f * PI;
	auto cam =
		CameraFactory::createPerspCamera(center, target, {0, 1, 0}, verticalFov, 1, width, height);

	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	constexpr uint32_t SAMPLE_PER_PIXEL = 1u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)SAMPLE_PER_PIXEL;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);
			HitRecord rec;
			if (!hitSolver->solve(ray, rec)) {
				film->addSample(SAMPLE_WEIGHT * domeLight->getIntensity(ray), sample);
			}
			else {
				Vector3f dummy;
				const Vector3f brdf = rec.primitive->getMaterial()->getBRDF(rec, dummy, dummy);
				film->addSample(brdf * PI, sample);
			}
		}
	};

	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	// 1);

	for (uint32_t i = 0u; i < SAMPLE_PER_PIXEL; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\gltf_load_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}