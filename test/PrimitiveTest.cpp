//
// Created by Frank on 2023/10/4.
//
#include "Eigen/Geometry"
#include "Film.h"
#include "HitSolver.h"
#include "Integrator.h"
#include "Light.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "oneapi/tbb.h"
#include "sampler/SimpleSampler.h"
#include "texture/TextureFactory.h"
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
	SceneBuilder sb;
	const auto sphere = std::make_shared<Sphere>(Vector3f{0, 0, 0}, radius);
	Vector3f sphereCenter{-rightNorm + radius, 0, 0};
	const float rotationAngle = 90.f;
	const uint32_t sphereCount = uint32_t(360.f / rotationAngle);
	const Vector3f interval{(2 * rightNorm - 2 * radius) / (sphereCount - 1), 0, 0};

	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	const auto reflect = std::make_shared<PerfectReflectionMaterial>();
	for (auto i = 0u; i < sphereCount; ++i, sphereCenter += interval) {
		auto rotation = Eigen::AngleAxis<float>(i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		auto translation = Eigen::Translation3f(sphereCenter);
		auto transform = translation * rotation;
		auto prim = std::make_shared<Primitive>(sphere, matte, transform);
		sb.addPrimitive(prim);
	}

	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);
	sb.addEnvironment(domeLight);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), rightNorm,
												rightNorm / width * height, width, height);
	auto film = cam->getFilm();

	constexpr uint32_t SAMPLE_PER_PIXEL = 3u;
	constexpr uint32_t MAX_DEPTH = 5u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);
	// PathIntegrator integrator{sampler, MAX_DEPTH};
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::BXDF);
	integrator.setCamera(cam);
	integrator.render(*scene);

	const std::string hdrPath = R"(D:\instance_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "Loader/MeshLoader.h"
#include "Triangle.h"
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

	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	constexpr float rotationAngle = 30.f;
	auto yUpToZUp = Eigen::AngleAxis<float>(PI / 2, Vector3f{1, 0, 0});
	for (auto i = 0u; i < instanceCount; ++i, meshCenter.x() += delta) {
		auto translation = Eigen::Translation3f(meshCenter);
		auto rotation = Eigen::AngleAxis<float>(i * rotationAngle / 180.f * PI, Vector3f{0, 0, 1});
		Transform transform = translation * (rotation * yUpToZUp);
		auto prim = std::make_shared<Primitive>(mesh, matte, transform);
		sb.addPrimitive(prim);
	}

	const auto sphereTexture = TextureFactory::loadSphereTextureRGB(R"(D:/dome.hdr)");
	const auto domeLight = std::make_shared<DomeLight>(sphereTexture);
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