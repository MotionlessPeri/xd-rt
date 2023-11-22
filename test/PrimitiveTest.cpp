//
// Created by Frank on 2023/10/4.
//
#include "../src/camera/CameraFactory.h"
#include "../src/core/HitSolver.h"
#include "../src/core/Light.h"
#include "Eigen/Geometry"

#include "../src/core/Primitive.h"
#include "../src/core/Scene.h"
#include "../src/texture/TextureFactory.h"
#include "gtest/gtest.h"
#include "oneapi/tbb.h"
using namespace xd;
TEST(PrimitiveTestSuite, InstanceTest)
{
	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 600u;
	const Vector3f center = Vector3f{0, 1, 0} * 400;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const float rightNorm = 500.f;
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const float radius = 200.f;
	auto scene = std::make_shared<Scene>();
	const auto sphere = std::make_shared<Sphere>(Vector3f{0, 0, 0}, radius);
	Vector3f sphereCenter{-rightNorm + radius, 0, 0};
	const float rotationAngle = 90.f;
	const uint32_t sphereCount = uint32_t(360.f / rotationAngle);
	const Vector3f interval{(2 * rightNorm - 2 * radius) / (sphereCount - 1), 0, 0};

	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	for (auto i = 0u; i < sphereCount; ++i, sphereCenter += interval) {
		auto rotation = Eigen::AngleAxis<float>(i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		auto translation = Eigen::Translation3f(sphereCenter);
		auto transform = translation * rotation;
		auto prim = std::make_shared<Primitive>(sphere, matte, transform);
		scene->addPrimitive(prim);
	}

	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);
	scene->addLight(domeLight);
	auto hitSolver = std::make_shared<NaiveHitSolver>(scene);

	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();
	auto sampler = std::make_shared<SimpleSampler>(width, height);

	const auto samples = sampler->generateSamples();
	constexpr uint32_t SAMPLE_PER_PIXEL = 3u;
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
				constexpr float epsilon = 1e-4;
				auto hitPoint = rec.tPoint;
				const auto model = rec.primitive->getModel();
				const auto material = rec.primitive->getMaterial();
				const auto [dpdu, dpdv, n] = std::tie(rec.dpdu, rec.dpdv, rec.n);
				hitPoint += (n * epsilon);

				for (const auto& light : scene->getLights()) {
					const auto nLightSamples = light->getNumSamples();
					uint32_t acceptedCount = 0u;
					ColorRGB radiance{0, 0, 0};
					for (auto i = 0u; i < nLightSamples; ++i) {
						// sample light source
						HitRecord shadowRec;
						const Ray shadowRay{hitPoint, light->getDirection(rec, shadowRec)};
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

	const std::string hdrPath = R"(D:\instance_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "../src/core/Triangle.h"
#include "Loader/MeshLoader.h"
TEST(PrimitiveTestSuite, InstanceTest2)
{
	ObjMeshLoader loader;
	auto mesh = loader.load(R"(D:\qem-test.obj)");
	const auto bound = mesh->getAABB();

	constexpr uint32_t width = 200u;
	constexpr uint32_t height = 200u;
	const Vector3f center{0, -2, 0};
	const Vector3f origin{0, 0, 0};
	const float rightNorm = 5.f;
	const Vector3f right{rightNorm, 0, 0};
	const Vector3f up{0, 0, rightNorm / width * height};

	auto scene = std::make_shared<Scene>();
	const int instanceCount = 4;
	const float delta = bound.getExtent().x();
	Vector3f meshCenter{-rightNorm + delta, 0, 0};

	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	constexpr float rotationAngle = 30.f;
	auto yUpToZUp = Eigen::AngleAxis<float>(PI / 2, Vector3f{1, 0, 0});
	for (auto i = 0u; i < instanceCount; ++i, meshCenter.x() += delta) {
		auto translation = Eigen::Translation3f(meshCenter);
		auto rotation = Eigen::AngleAxis<float>(i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		Transform transform = translation * (rotation * yUpToZUp);
		auto prim = std::make_shared<Primitive>(mesh, matte, transform);
		scene->addPrimitive(prim);
	}

	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);
	scene->addLight(domeLight);
	auto hitSolver = std::make_shared<EmbreeHitSolver>(scene);

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();
	auto sampler = std::make_shared<SimpleSampler>(width, height);
	const auto samples = sampler->generateSamples();

	constexpr uint32_t SAMPLE_PER_PIXEL = 3u;
	constexpr float SAMPLE_WEIGHT = 1.f / (float)SAMPLE_PER_PIXEL;
	const auto work = [&](const tbb::blocked_range<size_t>& r) {
		for (size_t sampleIdx = r.begin(); sampleIdx != r.end(); ++sampleIdx) {
			const auto& sample = samples[sampleIdx];
			auto ray = cam->generateRay(sample);
			HitRecord rec;
			if (!hitSolver->solve(ray, rec)) {
				film->addSample({100, 0, 0}, sample);
			}
			else {
				Vector3f dummy;
				const Vector3f brdf = rec.primitive->getMaterial()->getBRDF(rec, dummy, dummy);
				film->addSample(brdf, sample);
			}
		}
	};

	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	// 1);
	for (uint32_t i = 0u; i < SAMPLE_PER_PIXEL; ++i) {
		tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), work);
	}

	const std::string hdrPath = R"(D:\instance_test_2.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}