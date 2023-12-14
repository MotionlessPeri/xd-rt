//
// Created by Frank on 2023/8/31.
//

#include <oneapi/tbb.h>
#include <thread>
#include "Camera.h"
#include "Film.h"
#include "Integrator.h"
#include "Light.h"
#include "Macros.h"
#include "Material.h"
#include "Model.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "sampler/SimpleSampler.h"
#include "texture/TextureFactory.h"
using namespace xd;
TEST(MaterialTestSuite, LambertianTest)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(radius);
	auto greenSphere = std::make_shared<Sphere>(radius);
	auto redMatte = std::make_shared<MatteMaterial>(Vector3f{1, 0, 0});
	auto greenMatte = std::make_shared<MatteMaterial>(Vector3f{0, 1, 0});
	auto whiteMatte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	auto prim1 = std::make_shared<Primitive>(redSphere, redMatte,
											 Transform{Eigen::Translation3f{-200, 0, 0}});
	auto prim2 = std::make_shared<Primitive>(greenSphere, whiteMatte,
											 Transform{Eigen::Translation3f{200, 0, 0}});

	SceneBuilder sceneBuilder;
	sceneBuilder.addPrimitive(prim1);
	sceneBuilder.addPrimitive(prim2);

	sceneBuilder.setHitSolverType(xd::HitSolverType::NAIVE);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 800u;
	const Vector3f center{0, 0, -radius - 10.f};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 400, 0};

	const ColorRGB intensity = Vector3f{1.f, 1.f, 1.f} * 200000;
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);
	sceneBuilder.addLight(redLight);

	const ColorRGB intensity2 = Vector3f{1.f, 1.f, 1.f} * 200000;
	const Vector3f lightPos2 = 2 * center;
	auto greenLight = std::make_shared<PointLight>(lightPos2, intensity2);
	sceneBuilder.addLight(greenLight);

	auto cam = CameraFactory::createOrthoCamera(center, origin, up.normalized(), right.norm(),
												up.norm(), width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(5);

	PathIntegrator integrator{sampler, 8};
	// DebugIntegrator integrator;
	// integrator.setDebugChannel(DebugChannel::SHADOW_HIT);
	// integrator.setLightIndex(1);
	integrator.setCamera(cam);
	auto scene = sceneBuilder.build();
	integrator.render(*scene);

	const std::string hdrPath = R"(D:\lambertian_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

TEST(MaterialTestSuite, SpecularReflectionTest1)
{
	const Vector3f origin{0, 0, 0};
	const float radius = 200.f;
	auto redSphere = std::make_shared<Sphere>(radius);
	auto greenSphere = std::make_shared<Sphere>(radius);
	auto redMatte = std::make_shared<MatteMaterial>(Vector3f{1, 0, 0});
	auto greenMatte = std::make_shared<MatteMaterial>(Vector3f{0, 1, 0});
	auto whiteMatte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	const auto spec = std::make_shared<PerfectReflectionMaterial>();
	auto prim1 = std::make_shared<Primitive>(redSphere, whiteMatte,
											 Transform{Eigen::Translation3f{-200, 0, 0}});
	auto prim2 =
		std::make_shared<Primitive>(greenSphere, spec, Transform{Eigen::Translation3f{200, 0, 0}});

	const float extent = 1000.f;
	const float y = -200;
	const float delta = 10;
	const Vector3f minPoint{-extent, y - delta, -extent};
	const Vector3f maxPoint{extent, y, extent};
	auto plane = std::make_shared<Box>(minPoint, maxPoint);

	const auto specPlane = std::make_shared<Primitive>(plane, spec);

	SceneBuilder sceneBuilder;
	sceneBuilder.addPrimitive(prim1);
	sceneBuilder.addPrimitive(prim2);
	sceneBuilder.addPrimitive(specPlane);

	sceneBuilder.setHitSolverType(HitSolverType::NAIVE);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center{0, 0, -radius};
	const Vector3f right{500, 0, 0};
	const Vector3f up{0, 500, 0};

	const ColorRGB intensity = Vector3f{1.f, 0.f, 0.f} * 200000;
	const Vector3f lightPos{0, 500, 0};
	auto redLight = std::make_shared<PointLight>(lightPos, intensity);
	sceneBuilder.addLight(redLight);

	const ColorRGB intensity2 = Vector3f{0.f, 1.f, 0.f} * 10000;
	const Vector3f lightPos2{0, -100, -100};
	auto greenLight = std::make_shared<PointLight>(lightPos2, intensity2);
	sceneBuilder.addLight(greenLight);

	const auto verticalFov = 90.f / 180.f * PI;
	const Vector3f camPos{0, 0, -2 * radius};
	auto cam = CameraFactory::createPerspCamera(camPos, origin, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(2);

	PathIntegrator integrator{sampler, 5};
	integrator.setCamera(cam);
	auto scene = sceneBuilder.build();
	integrator.render(*scene);

	const std::string hdrPath = R"(D:\specular_reflection_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "MathType.h"
TEST(MaterialTestSuite, LambertianWithImageTest)
{
	const float radius = 1.f;
	SceneBuilder sceneBuilder;
	auto sphere = std::make_shared<Sphere>(radius);
	auto diffuse = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	auto matte = std::make_shared<MatteMaterial>(diffuse);
	auto spec = std::make_shared<PerfectReflectionMaterial>();
	auto primitive = std::make_shared<Primitive>(sphere, matte);
	sceneBuilder.addPrimitive(primitive);

	sceneBuilder.setHitSolverType(HitSolverType::NAIVE);

	constexpr uint32_t width = 100u;
	constexpr uint32_t height = 100u;
	const Vector3f center = Vector3f{0, 1.7, 0} * radius;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);

	const auto verticalFov = toRadians(90.f);
#if 1
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
#else
	Vector2i topLeft{194, 340};
	Vector2i bottomRight{244, 365};
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height, topLeft, bottomRight);
#endif
	// auto cam =
	//	CameraFactory::createOrthoCamera(center, target, up.normalized(), 500, 500, width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(50);

	{
		film->clear();
		MIDirectIntegrator integrator{sampler};
		integrator.setCamera(cam);
		const auto scene = sceneBuilder.build();
		integrator.render(*scene);

		const std::string hdrPath = R"(D:\matte_with_texture_test_mi_direct.hdr)";
		EXPECT_NO_THROW(film->saveToFile(hdrPath););
	}

	{
		film->clear();
		PathIntegrator integrator{sampler, 1};
		integrator.setCamera(cam);
		const auto scene = sceneBuilder.build();
		integrator.render(*scene);
		const std::string hdrPath = R"(D:\matte_with_texture_test_path.hdr)";
		EXPECT_NO_THROW(film->saveToFile(hdrPath););
	}
}