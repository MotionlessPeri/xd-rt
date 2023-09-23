//
// Created by Frank on 2023/8/29.
//
#include <oneapi/tbb.h>
#include "Camera.h"
#include "CameraFactory.h"
#include "Film.h"
#include "Light.h"
#include "Model.h"
#include "Primitive.h"
#include "Scene.h"
#include "gtest/gtest.h"
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
						HitRecord shadowRec;
						const Ray shadowRay{hitPoint, light->getDirection(rec, shadowRec)};
						const float cosTheta = n.dot(shadowRay.d);

						if (!hitSolver->solve(shadowRay, shadowRec)) {
							const auto projectedRadiance =
								light->getIntensity(shadowRay) * cosTheta;
							film->addSample(projectedRadiance, sample);
						}
					}
				}
			}
		});

	const std::string hdrPath = R"(D:\point_light_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "Material.h"
#include "TextureFactory.h"
TEST(LightTestSuite, DomeLightTest)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const auto spec = std::make_shared<SpecularReflectionMaterial>();
	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	auto scene = std::make_shared<Scene>();
	std::vector<std::shared_ptr<Primitive>> prims;
	for (int i = 0u; i < count; ++i) {
		const float x = firstCenter.x() + 2 * radius * i;
		for (int j = 0u; j < count; ++j) {
			const float y = firstCenter.y() + 2 * radius * j;
			for (int k = 0u; k < count; ++k) {
				const float z = firstCenter.z() + 2 * radius * k;
				const Vector3f center{x, y, z};
				const auto model = std::make_shared<Sphere>(center, radius);
				const auto prim = std::make_shared<Primitive>(model, spec);
				scene->addPrimitive(prim);
			}
		}
	}

	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const float sqrt3 = std::sqrtf(3);
	// const Vector3f center = Vector3f{1, 1, 1} * halfLen * sqrt3;
	const Vector3f center = Vector3f{2, 0, 0} * halfLen * sqrt3;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto sphereTexture = TextureFactory::loadSphereTexture3f(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(domeLight);

	const auto verticalFov = 75.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	constexpr uint32_t SAMPLE_PER_PIXEL = 1u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)SAMPLE_PER_PIXEL;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);

			constexpr uint32_t MAX_DEPTH = 5u;
			int depth = 0;
			Vector3f weight{1.f, 1.f, 1.f};
			while (depth < MAX_DEPTH) {
				HitRecord rec;
				if (!hitSolver->solve(ray, rec)) {
					film->addSample(domeLight->getIntensity(ray), sample);
					break;
				}

				constexpr float epsilon = 1e-4;
				auto hitPoint = ray.getTPoint(rec.tHit);
				const auto model = rec.primitive->getModel();
				const auto material = rec.primitive->getMaterial();
				const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
				hitPoint += (n * epsilon);

				for (const auto light : lights) {
					HitRecord dummy;
					const Ray shadowRay{hitPoint, material->getDirection(rec, -ray.d)};
					const float cosTheta = std::clamp(n.dot(shadowRay.d), 0.f, 1.f);
					if (!hitSolver->solve(shadowRay, dummy)) {
						const ColorRGB projectedRadiance =
							light->getIntensity(shadowRay) * cosTheta;
						const Vector3f brdf = material->getBRDF(rec, shadowRay.d, -ray.d);
						const Vector3f Lo = projectedRadiance.cwiseProduct(brdf);
						film->addSample(SAMPLE_WEIGHT * Lo.cwiseProduct(weight), sample);
					}
					else {
						// film->addSample({dummy.tHit, dummy.tHit, dummy.tHit}, sample);
					}
				}
				auto newDirection = material->getDirection(rec, -ray.d);
				weight = weight.cwiseProduct(material->getBRDF(rec, newDirection, -ray.d));
				ray = Ray{hitPoint, newDirection};
				++depth;
			}
		}
	};

	for (uint32_t i = 0u; i < SAMPLE_PER_PIXEL; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\dome_light_test_2.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}