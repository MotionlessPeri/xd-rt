//
// Created by Frank on 2023/8/29.
//
#include <oneapi/tbb.h>
#include "Film.h"
#include "Macros.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "light/PointLight.h"
#include "material/MatteMaterial.h"
#include "model/Sphere.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
TEST(LightTestSuite, PointLightTest)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(radius);
	auto greenSphere = std::make_shared<Sphere>(radius);
	const Transform redSphereTransform{Eigen::Translation3f{-50, 0, 0}};
	const Transform greenSphereTransform{Eigen::Translation3f{50, 0, 0}};
	auto prim1 = std::make_shared<Primitive>(
		redSphere, std::make_shared<MatteMaterial>(ColorRGB{1, 1, 1}), redSphereTransform);
	auto prim2 = std::make_shared<Primitive>(
		greenSphere, std::make_shared<MatteMaterial>(ColorRGB{1, 1, 1}), greenSphereTransform);

	SceneBuilder sceneBuilder;
	sceneBuilder.addPrimitive(prim1);
	sceneBuilder.addPrimitive(prim2);
	sceneBuilder.setHitSolverType(xd::HitSolverType::NAIVE);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	const ColorRGB intensity = Vector3f{1.f, 0.f, 0.f} * 100000;
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);

	const ColorRGB intensity2 = Vector3f{0.f, 1.f, 0.f} * 100000;
	const Vector3f lightPos2 = 2 * center;
	auto greenLight = std::make_shared<PointLight>(lightPos2, intensity2);

	sceneBuilder.addLight(redLight);
	sceneBuilder.addLight(greenLight);
	auto scene = sceneBuilder.build();
	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(1);
	MIDirectIntegrator integrator{sampler};
	// DebugIntegrator integrator;
	// TBB_SERIAL
	// integrator.setDebugBreakPixel({39, 33});
	// integrator.setDebugChannel(DebugChannel::SHADOW_HIT);
	integrator.setCamera(cam);
	integrator.render(*scene);
	const std::string hdrPath = R"(D:\point_light_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "light/DomeLight.h"
#include "loader/TextureFactory.h"
#include "material/PerfectReflectionMaterial.h"
TEST(LightTestSuite, DomeLightTest)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const auto spec = std::make_shared<PerfectReflectionMaterial>();
	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	SceneBuilder sceneBuilder;

	std::vector<std::shared_ptr<Primitive>> prims;
	for (int i = 0u; i < count; ++i) {
		const float x = firstCenter.x() + 2 * radius * i;
		for (int j = 0u; j < count; ++j) {
			const float y = firstCenter.y() + 2 * radius * j;
			for (int k = 0u; k < count; ++k) {
				const float z = firstCenter.z() + 2 * radius * k;
				const Vector3f center{x, y, z};
				const auto model = std::make_shared<Sphere>(radius);
				const Transform transform{Eigen::Translation3f{center}};
				const auto prim = std::make_shared<Primitive>(model, spec, transform);
				sceneBuilder.addPrimitive(prim);
			}
		}
	}

	sceneBuilder.setHitSolverType(xd::HitSolverType::EMBREE);

	constexpr uint32_t width = 2000u;
	constexpr uint32_t height = 2000u;
	const float sqrt3 = std::sqrtf(3);
	// const Vector3f center = Vector3f{1, 1, 1} * halfLen * sqrt3;
	const Vector3f center = Vector3f{2, 0, 0} * halfLen * sqrt3;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);
	auto scene = sceneBuilder.build();
	const auto verticalFov = 75.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	constexpr uint32_t SAMPLE_PER_PIXEL = 1u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);

	const auto work = [&](const tbb::blocked_range2d<int, int>& range) {
		const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
		const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
		auto tile = film->getTile(topLeft, bottomRight);
		auto tileSampler = sampler->clone(topLeft.y() * width + topLeft.x());
		for (const auto pixel : *tile) {
			tileSampler->setCurrentPixel(pixel);
			do {
				const auto sample = pixel.cast<float>() + tileSampler->sample2D();
				auto primRay = cam->generateRay(sample);
				constexpr uint32_t MAX_DEPTH = 10u;
				int depth = 0;
				Vector3f weight{1.f, 1.f, 1.f};
				while (depth < MAX_DEPTH) {
					HitRecord primRec;
					if (!scene->hit(primRay, primRec)) {
						tile->addSample(domeLight->getRadiance({}, primRay.d), sample);
						break;
					}

					const auto shadingGeom = primRec.getShadingGeomParams();
					const auto [dpdu, dpdv, n] =
						std::tie(primRec.geom.derivatives.dpdu, primRec.geom.derivatives.dpdv,
								 primRec.geom.derivatives.n);

					for (const auto& light : scene->getLights()) {
						HitRecord shadowRec;
						const auto shadowRay = primRec.spawnRay(
							primRec.sampleMaterialWi(tileSampler->sample2D(), -primRay.d));
						const float cosTheta = std::clamp(n.dot(shadowRay.d), 0.f, 1.f);
						if (!scene->hit(shadowRay, shadowRec)) {
							const ColorRGB projectedRadiance =
								light->getRadiance(shadingGeom, shadowRay.d) * cosTheta;
							const Vector3f brdf = primRec.getBxDF(-primRay.d, shadowRay.d);
							const Vector3f Lo = projectedRadiance.cwiseProduct(brdf);
							tile->addSample(Lo.cwiseProduct(weight), sample);
						}
					}
					const auto newDirection =
						primRec.sampleMaterialWi(tileSampler->sample2D(), -primRay.d);
					weight = weight.cwiseProduct(primRec.getBxDF(-primRay.d, newDirection));
					primRay = primRec.spawnRay(newDirection);
					++depth;
				}
			} while (tileSampler->startNextSample());
		}
		film->mergeTileToFilm(std::move(tile));
	};

	tbb::parallel_for(tbb::blocked_range2d<int, int>(0, width, 0, height), work);

	// DebugIntegrator integrator;
	// integrator.setDebugChannel(DebugChannel::NORMAL);
	// integrator.setCamera(cam);
	// integrator.render(*scene);
	const std::string hdrPath = R"(D:\dome_light_test_2.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(LightTestSuite, DomeLightPdfTest)
{
	const auto domeLight = std::make_shared<DomeLight>(R"(D:\dome.hdr)");
	const auto texture = std::dynamic_pointer_cast<SphereTextureRGB>(domeLight->dome);
	EXPECT_TRUE(texture);
	const auto& dis = domeLight->dis;
	const uint32_t width = dis->getWidth();
	const uint32_t height = dis->getHeight();
	Film film{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, width, height};
	auto tile = film.getTile({0, 0}, {width - 1, height - 1});
	for (const auto pixel : *tile) {
		const Vector2f samplePos = pixel.cast<float>() + Vector2f{0.5f, 0.5f};
		const Vector2f uv = samplePos.cwiseQuotient(Vector2f{width, height});
		const auto dir = getSphereDirFromUV(uv);
		const auto pdf = domeLight->getPdf({}, dir);
		tile->addSample({pdf, 0, 0}, samplePos);
	}
	film.mergeTileToFilm(std::move(tile));
	film.saveToFile(R"(D:\dome_light_pdf_test.hdr)");
}