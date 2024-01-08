//
// Created by Frank on 2023/10/4.
//
#include "Eigen/Geometry"
#include "Film.h"
#include "Primitive.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "loader/TextureFactory.h"
#include "model/Sphere.h"
#include "oneapi/tbb.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
TEST(PrimitiveTestSuite, InstanceTest0)
{
	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center{0, 2, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const float rightNorm = 2.f;
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto sphere = std::make_shared<Sphere>(1.f);
	const Transform transform{Eigen::Translation3f{0.f, 0.f, 0.f}};
	const auto prim = std::make_shared<Primitive>(sphere, nullptr, transform);
	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();
	auto tile = film->getTile({0, 0}, {width - 1, height - 1});

	oneapi::tbb::parallel_for(
		tbb::blocked_range2d<int, int>{0, width, 0, height},
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.rows().begin(), range.cols().begin()};
			const Vector2i bottomRight{range.rows().end() - 1, range.cols().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);

			for (const auto& pixel : *tile) {
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5, 0.5};
				const auto primRay = cam->generateRay(pixelSample);
				HitRecord primRec;
				if (prim->hit(primRay, primRec)) {
					const auto shadowRayDir = reflected(-primRay.d, primRec.geom.derivatives.n);
					const auto shadowRay = primRec.spawnRay(shadowRayDir);
					HitRecord shadowRec{};
					if (!prim->hit(shadowRay, shadowRec)) {
						tile->addSample({0, 0, 1}, pixelSample);
					}
					else {
						EXPECT_TRUE(false);
#if 0
						// bad cases
						HitRecord debugRec;
						prim->hit(primRay, debugRec);
						const auto debugDir = debugRec.n;
						const auto debugRay = debugRec.spawnRay(debugDir);
						HitRecord debugShadowRec;
						prim->hit(debugRay, debugShadowRec);
						tile->addSample({debugShadowRec.tHit, 1, 0}, pixelSample);
#endif
					}
				}
			}
			film->mergeTileToFilm(std::move(tile));
		});

	const std::string hdrPath = R"(D:\instance_test_0.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "integrator/PathIntegrator.h"
#include "light/DomeLight.h"
#include "material/MatteMaterial.h"
#include "material/PerfectReflectionMaterial.h"
TEST(PrimitiveTestSuite, InstanceTest1)
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
	SceneBuilder sb;
	const auto sphere = std::make_shared<Sphere>(radius);
	Vector3f sphereCenter{-rightNorm + radius, 0, 0};
	const float rotationAngle = 90.f;
	const auto sphereCount = uint32_t(360.f / rotationAngle);
	const Vector3f interval{(2 * rightNorm - 2 * radius) / ((float)sphereCount - 1), 0, 0};

	auto diffuse = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	const auto reflect = std::make_shared<PerfectReflectionMaterial>();
	for (auto i = 0u; i < sphereCount; ++i, sphereCenter += interval) {
		auto rotation =
			Eigen::AngleAxis<float>((float)i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		auto translation = Eigen::Translation3f(sphereCenter);
		auto transform = translation * rotation;
		auto prim = std::make_shared<Primitive>(sphere, matte, transform);
		sb.addPrimitive(prim);
	}

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");
	sb.addEnvironment(domeLight);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();

	constexpr uint32_t SAMPLE_PER_PIXEL = 10u;
	constexpr uint32_t MAX_DEPTH = 8u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);
	PathIntegrator integrator{sampler, MAX_DEPTH};
	// DebugIntegrator integrator;
	// integrator.setDebugChannel(DebugChannel::BXDF);
	integrator.setCamera(cam);
	integrator.render(*scene);

	const std::string hdrPath = R"(D:\instance_test_1.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "integrator/DebugIntegrator.h"
#include "loader/ObjMeshLoader.h"
#include "model/Triangle.h"
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

	SceneBuilder sb;
	const int instanceCount = 4;
	const float delta = bound.getExtent().x();
	Vector3f meshCenter{-rightNorm + delta, 0, 0};

	auto diffuse = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	constexpr float rotationAngle = 30.f;
	auto yUpToZUp = Eigen::AngleAxis<float>(PI / 2, Vector3f{1, 0, 0});
	for (auto i = 0u; i < instanceCount; ++i) {
		auto translation = Eigen::Translation3f(meshCenter);
		auto rotation =
			Eigen::AngleAxis<float>((float)i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		Transform transform = translation * (rotation * yUpToZUp);
		auto prim = std::make_shared<Primitive>(mesh, matte, transform);
		sb.addPrimitive(prim);
		meshCenter.x() += delta;
	}

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");
	sb.addEnvironment(domeLight);
	sb.setHitSolverType(HitSolverType::EMBREE);
	const auto scene = sb.build();

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	DebugIntegrator integrator{};
	integrator.setCamera(cam);
	integrator.setDebugChannel(DebugChannel::BXDF);
	integrator.render(*scene);
	const std::string hdrPath = R"(D:\instance_test_2.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "integrator/DirectIntegrator.h"
TEST(PrimitiveTestSuite, InstanceTest3)
{
	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 600u;
	const Vector3f center = Vector3f{0, 1, 0} * 2;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const float rightNorm = 2.5f;
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();

	const float radius = 1.f;
	SceneBuilder sb;
	const auto sphere = std::make_shared<Sphere>(radius);

	auto diffuse = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	auto reflect = std::make_shared<PerfectReflectionMaterial>();
	Matrix4f rotMat;
	rotMat << 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	// Transform transform{Eigen::AngleAxis(toRadians(90.f), Vector3f{0, 0, 1})};
	Transform transform{rotMat};
	auto prim = std::make_shared<Primitive>(sphere, reflect, transform);
	sb.addPrimitive(prim);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");
	sb.addEnvironment(domeLight);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	constexpr uint32_t SAMPLE_PER_PIXEL = 1u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);

	// TBB_SERIAL
	MIDirectIntegrator integrator{sampler};
	// DebugIntegrator integrator;
	// integrator.setDebugChannel(DebugChannel::TOTAL_RADIANCE);
	integrator.setCamera(cam);
	integrator.render(*scene);
	const std::string hdrPath = R"(D:\instance_test_3.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}