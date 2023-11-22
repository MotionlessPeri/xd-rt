//
// Created by Frank on 2023/8/31.
//

#include "gtest/gtest.h"

#include <oneapi/tbb.h>
#include <thread>
#include "../src/camera/CameraFactory.h"
#include "../src/core/Camera.h"
#include "../src/core/Film.h"
#include "../src/core/Light.h"
#include "../src/core/Model.h"
#include "../src/core/Primitive.h"
#include "../src/core/Scene.h"
#include "../src/texture/TextureFactory.h"
using namespace xd;
TEST(MaterialTestSuite, LambertianTest)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(Vector3f{-200, 0, 0}, radius);
	auto greenSphere = std::make_shared<Sphere>(Vector3f{200, 0, 0}, radius);
	auto redMatte = std::make_shared<MatteMaterial>(Vector3f{1, 0, 0});
	auto greenMatte = std::make_shared<MatteMaterial>(Vector3f{0, 1, 0});
	auto whiteMatte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	auto prim1 = std::make_shared<Primitive>(redSphere, redMatte);
	auto prim2 = std::make_shared<Primitive>(greenSphere, whiteMatte);

	auto scene = std::make_shared<Scene>();
	scene->addPrimitive(prim1);
	scene->addPrimitive(prim2);

	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	const ColorRGB intensity = Vector3f{1.f, 1.f, 1.f} * 200000;
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);

	const ColorRGB intensity2 = Vector3f{1.f, 1.f, 1.f} * 200000;
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
	constexpr uint32_t LIMIT = 5u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)LIMIT;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);

			constexpr uint32_t MAX_DEPTH = 2u;
			int depth = 0;
			Vector3f weight{1.f, 1.f, 1.f};
			while (depth < MAX_DEPTH) {
				HitRecord rec;
				if (!hitSolver->solve(ray, rec))
					break;
				constexpr float epsilon = 1e-4;
				auto hitPoint = ray.getTPoint(rec.tHit);
				const auto model = rec.primitive->getModel();
				const auto material = rec.primitive->getMaterial();
				const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
				hitPoint += (n * epsilon);

				for (const auto light : lights) {
					HitRecord shadowRec;
					const Ray shadowRay{hitPoint, light->getDirection(rec, shadowRec)};
					const float cosTheta = std::clamp(n.dot(shadowRay.d), 0.f, 1.f);

					if (!hitSolver->solve(shadowRay, shadowRec)) {
						const ColorRGB projectedRadiance =
							light->getIntensity(shadowRay) * cosTheta;
						const Vector3f brdf = material->getBRDF(rec, shadowRay.d, -ray.d);
						const Vector3f Lo = projectedRadiance.cwiseProduct(brdf);
						film->addSample(SAMPLE_WEIGHT * Lo.cwiseProduct(weight), sample);
					}
				}
				auto newDirection = material->getDirection(rec, -ray.d);
				weight = weight.cwiseProduct(material->getBRDF(rec, newDirection, -ray.d));
				ray = Ray{hitPoint, newDirection};
				++depth;
			}
		}
	};

	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	// 1);

	for (uint32_t i = 0u; i < LIMIT; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\lambertian_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(MaterialTestSuite, SpecularReflectionTest1)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(Vector3f{-200, 0, 0}, radius);
	auto greenSphere = std::make_shared<Sphere>(Vector3f{200, 0, 0}, radius);
	auto redMatte = std::make_shared<MatteMaterial>(Vector3f{1, 0, 0});
	auto greenMatte = std::make_shared<MatteMaterial>(Vector3f{0, 1, 0});
	auto whiteMatte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	const auto spec = std::make_shared<SpecularReflectionMaterial>();
	auto prim1 = std::make_shared<Primitive>(redSphere, whiteMatte);
	auto prim2 = std::make_shared<Primitive>(greenSphere, spec);

	const float extent = 1000.f;
	const float y = -200;
	const float delta = 10;
	const Vector3f minPoint{-extent, y - delta, -extent};
	const Vector3f maxPoint{extent, y, extent};
	auto plane = std::make_shared<Box>(minPoint, maxPoint);

	const auto specPlane = std::make_shared<Primitive>(plane, spec);

	auto scene = std::make_shared<Scene>();
	scene->addPrimitive(prim1);
	scene->addPrimitive(prim2);
	scene->addPrimitive(specPlane);

	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 500, 0};

	const ColorRGB intensity = Vector3f{1.f, 0.f, 0.f} * 200000;
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);

	const ColorRGB intensity2 = Vector3f{0.f, 1.f, 0.f} * 10000;
	const Vector3f lightPos2{0, -100, -100};
	auto greenLight = std::make_shared<PointLight>(lightPos2, intensity2);

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(redLight);
	lights.push_back(greenLight);

	const auto verticalFov = 90.f / 180.f * PI;
	const Vector3f camPos{0, 0, -2 * radius};
	auto cam = CameraFactory::createPerspCamera(camPos, origin, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	constexpr uint32_t SAMPLE_PER_PIXEL = 2u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)SAMPLE_PER_PIXEL;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);

			constexpr uint32_t MAX_DEPTH = 10u;
			int depth = 0;
			Vector3f weight{1.f, 1.f, 1.f};
			while (depth < MAX_DEPTH) {
				HitRecord rec;
				if (!hitSolver->solve(ray, rec))
					break;
				constexpr float epsilon = 1e-4;
				auto hitPoint = ray.getTPoint(rec.tHit);
				const auto model = rec.primitive->getModel();
				const auto material = rec.primitive->getMaterial();
				const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
				hitPoint += (n * epsilon);

				for (const auto light : lights) {
					HitRecord shadowRec;
					const Ray shadowRay{hitPoint, light->getDirection(rec, shadowRec)};
					const float cosTheta = std::clamp(n.dot(shadowRay.d), 0.f, 1.f);
					if (!hitSolver->solve(shadowRay, shadowRec)) {
						const ColorRGB projectedRadiance =
							light->getIntensity(shadowRay) * cosTheta;
						const Vector3f brdf = material->getBRDF(rec, shadowRay.d, -ray.d);
						const Vector3f Lo = projectedRadiance.cwiseProduct(brdf);
						film->addSample(SAMPLE_WEIGHT * Lo.cwiseProduct(weight), sample);
					}
				}
				auto newDirection = material->getDirection(rec, -ray.d);
				weight = weight.cwiseProduct(material->getBRDF(rec, newDirection, -ray.d));
				ray = Ray{hitPoint, newDirection};
				++depth;
			}
		}
	};

		// oneapi::tbb::global_control
	 //global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	 //1);
	for (uint32_t i = 0u; i < SAMPLE_PER_PIXEL; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\specular_reflection_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(MaterialTestSuite, LambertianWithImageTest)
{
	const float radius = 400.f;
	const Vector3f origin{0, 0, 0};
	const ColorRGB white{1, 1, 1};
	auto scene = std::make_shared<Scene>();
	auto sphere = std::make_shared<Sphere>(origin, radius);
	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	auto spec = std::make_shared<SpecularReflectionMaterial>();
	auto primitive = std::make_shared<Primitive>(sphere, matte);
	scene->addPrimitive(primitive);

	auto hitSolver = std::make_shared<EmbreeHitSolver>(scene);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, 1.7, 0} * radius;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(domeLight);

	const auto verticalFov = 90.f / 180.f * PI;
	// auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
	//											width, height);
	auto cam =
		CameraFactory::createOrthoCamera(center, target, up.normalized(), 500, 500, width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	constexpr uint32_t SAMPLE_PER_PIXEL = 1u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)SAMPLE_PER_PIXEL;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);

			constexpr uint32_t MAX_DEPTH = 1u;
			int depth = 0;
			Vector3f weight{1.f, 1.f, 1.f};
			while (depth < MAX_DEPTH) {
				HitRecord rec;
				if (!hitSolver->solve(ray, rec)) {
					film->addSample(SAMPLE_WEIGHT * domeLight->getIntensity(ray), sample);
					break;
				}
				//else {
				//	const auto material = rec.primitive->getMaterial();
				//	const Vector3f brdf = material->getBRDF(rec, {}, -ray.d);
				//	film->addSample(brdf, sample);
				//	break;
				//}
				constexpr float epsilon = 1e-4;
				auto hitPoint = ray.getTPoint(rec.tHit);
				const auto model = rec.primitive->getModel();
				const auto material = rec.primitive->getMaterial();
				const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
				hitPoint += (n * epsilon);

				for (const auto light : lights) {
					const auto nLightSamples = light->getNumSamples();
					uint32_t acceptedCount = 0u;
					ColorRGB radiance{0, 0, 0};
					for (auto i = 0u; i < nLightSamples; ++i) {
						// sample light source
						HitRecord shadowRec;
						float pdf;
						const Ray shadowRay{hitPoint, light->getDirection(rec, shadowRec)};
						// const Ray shadowRay{hitPoint, light->sample(rec, shadowRec, pdf)};
						const float cosTheta = std::clamp(n.dot(shadowRay.d), 0.f, 1.f);
						if (!hitSolver->solve(shadowRay, shadowRec)) {
							const ColorRGB projectedRadiance =
								light->getIntensity(shadowRay) * cosTheta;
							const Vector3f brdf = material->getBRDF(rec, shadowRay.d, -ray.d);
							const Vector3f Lo = projectedRadiance.cwiseProduct(brdf);
							radiance += Lo.cwiseProduct(weight);
							++acceptedCount;
						}
					}
					film->addSample(weight.cwiseProduct(SAMPLE_WEIGHT * radiance / acceptedCount),
									sample);
				}
				auto newDirection = material->getDirection(rec, -ray.d);
				weight = weight.cwiseProduct(material->getBRDF(rec, newDirection, -ray.d));
				ray = Ray{hitPoint, newDirection};
				++depth;
			}
		}
	};

	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	// 1);

	for (uint32_t i = 0u; i < SAMPLE_PER_PIXEL; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\matte_with_texture_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}