//
// Created by Frank on 2023/8/29.
//
#include "gtest/gtest.h"

#include <oneapi/tbb.h>
#include <thread>
#include "Camera.h"
#include "CameraFactory.h"
#include "Film.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"
#include "Primitive.h"
#include "Scene.h"
using namespace xd;
TEST(LightTestSuite, PointLightTest)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(Vector3f{-50, 0, 0}, radius);
	auto greenSphere = std::make_shared<Sphere>(Vector3f{50, 0, 0}, radius);
	auto prim1 = std::make_shared<Primitive>(redSphere, nullptr);
	auto prim2 = std::make_shared<Primitive>(greenSphere, nullptr);

	auto scene = std::make_shared<Scene>();
	scene->addPrimitive(prim1);
	scene->addPrimitive(prim2);

	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	const ColorRGB intensity{1.f, 0.f, 0.f};
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);

	const ColorRGB intensity2{0.f, 1.f, 0.f};
	const Vector3f lightPos2 = 2 * center;
	auto greenLight = std::make_shared<PointLight>(lightPos2, intensity2);

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(redLight);
	lights.push_back(greenLight);

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
				if (hitSolver->solve(ray, rec)) {
					constexpr float epsilon = 1e-4;
					auto hitPoint = ray.getTPoint(rec.tHit);
					const auto model = rec.primitive->getModel();
					const auto material = rec.primitive->getMaterial();
					const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
					hitPoint += (n * epsilon);

					for (const auto& light : lights) {
						HitRecord dummy;
						const auto shadowRay = light->getShadowRay(hitPoint, dummy);
						const float cosTheta = n.dot(shadowRay.d);

						if (!hitSolver->solve(shadowRay, dummy)) {
							const auto projectedRadiance = light->getIntensity(hitPoint) * cosTheta;
							film->addSample(projectedRadiance, sample);
						}
					}
				}
			}
		});

	const std::string hdrPath = R"(D:\point_light_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}