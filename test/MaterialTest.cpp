//
// Created by Frank on 2023/8/31.
//

#include <oneapi/tbb.h>
#include <thread>
#include "Film.h"
#include "Macros.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/PathIntegrator.h"
#include "light/PointLight.h"
#include "loader/TextureFactory.h"
#include "material/MatteMaterial.h"
#include "model/Sphere.h"
#include "sampler/SimpleSampler.h"
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

	auto sampler = std::make_shared<SimpleSampler>(50);

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

#include "material/PerfectReflectionMaterial.h"
#include "model/Box.h"
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

	auto sampler = std::make_shared<SimpleSampler>(20);

	PathIntegrator integrator{sampler, 20};
	integrator.setCamera(cam);
	auto scene = sceneBuilder.build();
	integrator.render(*scene);

	const std::string hdrPath = R"(D:\specular_reflection_test.hdr)";
	EXPECT_NO_THROW(film->saveToFile(hdrPath););
}

#include "integrator/DirectIntegrator.h"
#include "light/DomeLight.h"
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

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, 1.7, 0} * radius;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);

	const auto verticalFov = toRadians(90.f);
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	// auto cam =
	//	CameraFactory::createOrthoCamera(center, target, up.normalized(), 500, 500, width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(10);

	//{
	//	film->clear();
	//	MIDirectIntegrator integrator{sampler};
	//	integrator.setCamera(cam);
	//	const auto scene = sceneBuilder.build();
	//	integrator.render(*scene);

	//	const std::string hdrPath = R"(D:\matte_with_texture_test_mi_direct.hdr)";
	//	EXPECT_NO_THROW(film->saveToFile(hdrPath););
	//}

	{
		// TBB_SERIAL
		film->clear();
		PathIntegrator integrator{sampler, 8};
		integrator.setCamera(cam);
		const auto scene = sceneBuilder.build();
		integrator.render(*scene);
		const std::string hdrPath = R"(D:\matte_with_texture_test_path.hdr)";
		EXPECT_NO_THROW(film->saveToFile(hdrPath););
	}
}

#include "material/PerfectTransmissionMaterial.h"

TEST(MaterialTestSuite, PerfectTransmissionBasicTest)
{
	const float radius = 1.f;
	auto sphere = std::make_shared<Sphere>(radius);
	constexpr float etaIn = 1.5f;
	constexpr float etaOut = 1.f;
	auto transmission = std::make_shared<PerfectTransmissionMaterial>(etaOut, etaIn);
	auto spec = std::make_shared<PerfectReflectionMaterial>();
	auto primitive = std::make_shared<Primitive>(sphere, transmission);
	Ray ray{{2, 0, 0}, {-1, 0, 0}};
	HitRecord rec{};
	EXPECT_TRUE(primitive->hit(ray, rec));
	const auto btdfDirPdf = rec.sampleBxDFPdf({0, 0}, -ray.d);
	EXPECT_TRUE(btdfDirPdf.bxdf.isApprox(Vector3f(1, 1, 1) * (etaIn * etaIn) / (etaOut * etaOut) /
										 std::fabs(btdfDirPdf.dir.dot(rec.geom.derivatives.n))));
	EXPECT_EQ(btdfDirPdf.pdf, 1.f);
	EXPECT_TRUE(btdfDirPdf.dir.isApprox(Vector3f{-1, 0, 0}));
}

TEST(MaterialTestSuite, PerfectTransmissionSceneTest)
{
	const float radius = 1.f;
	SceneBuilder sceneBuilder;
	auto sphere = std::make_shared<Sphere>(radius);
	auto matte = std::make_shared<PerfectTransmissionMaterial>(1.0f, 2.5f);
	auto spec = std::make_shared<PerfectReflectionMaterial>();
	auto primitive = std::make_shared<Primitive>(sphere, matte);
	sceneBuilder.addPrimitive(primitive);

	sceneBuilder.setHitSolverType(HitSolverType::NAIVE);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, 1.7, 0} * radius;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);

	const auto verticalFov = toRadians(90.f);
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	// auto cam =
	//	CameraFactory::createOrthoCamera(center, target, up.normalized(), 1, 1, width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(1);
	{
		film->clear();
		PathIntegrator integrator{sampler, 8};
		integrator.setCamera(cam);
		const auto scene = sceneBuilder.build();
		integrator.render(*scene);
		const std::string hdrPath = R"(D:\perfect_transmission_test_path.hdr)";
		EXPECT_NO_THROW(film->saveToFile(hdrPath););
	}
}

TEST(MaterialTestSuite, PerfectTransmissionSceneTest2)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const auto transmission = std::make_shared<PerfectTransmissionMaterial>(1.0f, 2.5f);
	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	SceneBuilder sceneBuilder;

	std::vector<std::shared_ptr<Primitive>> prims;
	for (int i = 0u; i < count; ++i) {
		const float x = firstCenter.x() + 2 * radius * (float)i;
		for (int j = 0u; j < count; ++j) {
			const float y = firstCenter.y() + 2 * radius * (float)j;
			for (int k = 0u; k < count; ++k) {
				const float z = firstCenter.z() + 2 * radius * (float)k;
				const Vector3f center{x, y, z};
				const auto model = std::make_shared<Sphere>(radius);
				const Transform transform{Eigen::Translation3f{center}};
				const auto prim = std::make_shared<Primitive>(model, transmission, transform);
				sceneBuilder.addPrimitive(prim);
			}
		}
	}

	sceneBuilder.setHitSolverType(HitSolverType::EMBREE);

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

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);
	auto scene = sceneBuilder.build();
	const auto verticalFov = 75.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	constexpr uint32_t SAMPLE_PER_PIXEL = 10u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);

	PathIntegrator integrator{sampler, 20};
	integrator.setCamera(cam);
	integrator.render(*scene);
	EXPECT_NO_THROW(film->saveToFile(R"(D:\perfect_transmission_test2_path.hdr)"));
}

TEST(MaterialTestSuite, PerfectTransmissionSceneTest3)
{
	const auto transmission = std::make_shared<PerfectTransmissionMaterial>(1.f, 2.5f);
	const auto matte = std::make_shared<MatteMaterial>(ColorRGB{1, 1, 1});
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto spherePrim = std::make_shared<Primitive>(sphere, transmission);
	const auto box = std::make_shared<Box>(Vector3f{-1000, -1000, -2}, Vector3f{1000, 1000, -1});
	const auto boxPrim = std::make_shared<Primitive>(box, matte);

	const auto dome = std::make_shared<DomeLight>(R"(D:\dome.hdr)");
	SceneBuilder sb;
	const auto scene = sb.addPrimitive(spherePrim)
						   .addPrimitive(boxPrim)
						   .addEnvironment(dome)
						   .setHitSolverType(HitSolverType::NAIVE)
						   .build();

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, -1.7, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);
	const auto verticalFov = toRadians(90.f);
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(100);

	PathIntegrator integrator{sampler, 20};
	integrator.setCamera(cam);
	integrator.render(*scene);

	EXPECT_NO_THROW(film->saveToFile(R"(D:\perfect_transmission_test3_path.hdr)"));
}

#include "material/PerfectFresnelMaterial.h"
TEST(MaterialTestSuite, PerfectFresnelTest1)
{
	const float radius = 1.f;
	SceneBuilder sceneBuilder;
	auto sphere = std::make_shared<Sphere>(radius);
	auto mtl = std::make_shared<PerfectFresnelMaterial>(1.0f, 5.f);
	// auto mtl = std::make_shared<PerfectTransmissionMaterial>(1.0f, 5.f);
	// auto mtl = std::make_shared<PerfectReflectionMaterial>();
	auto primitive = std::make_shared<Primitive>(sphere, mtl);
	sceneBuilder.addPrimitive(primitive);

	sceneBuilder.setHitSolverType(HitSolverType::NAIVE);

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, 1.7f, 0} * radius;
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);

	const auto verticalFov = toRadians(90.f);
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	// auto cam =
	//	CameraFactory::createOrthoCamera(center, target, up.normalized(), 1, 1, width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(100);
	const auto timeStart = std::chrono::steady_clock::now();
	{
		// TBB_SERIAL
		film->clear();
		// DebugIntegrator integrator;
		// integrator.setDebugChannel(DebugChannel::TEMP);
		PathIntegrator integrator{sampler, 200};
		integrator.setCamera(cam);
		const auto scene = sceneBuilder.build();
		integrator.render(*scene);
		const std::string hdrPath = R"(D:\perfect_fresnel_test_path.hdr)";
		EXPECT_NO_THROW(film->saveToFile(hdrPath););
	}
	const auto timeEnd = std::chrono::steady_clock::now();
	const std::chrono::duration<double> duration = timeEnd - timeStart;
	std::cout << "rendering fresnel sphere costing " << duration << " seconds.\n";
}

TEST(MaterialTestSuite, PerfectFresnelSceneTest2)
{
	const float halfLen = 400.f;
	const float len = 2 * halfLen;
	const Vector3f maxPoint{halfLen, halfLen, halfLen};
	const Vector3f minPoint{-halfLen, -halfLen, -halfLen};

	const auto transmission = std::make_shared<PerfectFresnelMaterial>(1.0f, 2.5f);
	const uint32_t count = 5;
	const float radius = len / count / 2;
	const Vector3f firstCenter = minPoint + Vector3f{radius, radius, radius};
	SceneBuilder sceneBuilder;

	std::vector<std::shared_ptr<Primitive>> prims;
	for (int i = 0u; i < count; ++i) {
		const float x = firstCenter.x() + 2 * radius * (float)i;
		for (int j = 0u; j < count; ++j) {
			const float y = firstCenter.y() + 2 * radius * (float)j;
			for (int k = 0u; k < count; ++k) {
				const float z = firstCenter.z() + 2 * radius * (float)k;
				const Vector3f center{x, y, z};
				const auto model = std::make_shared<Sphere>(radius);
				const Transform transform{Eigen::Translation3f{center}};
				const auto prim = std::make_shared<Primitive>(model, transmission, transform);
				sceneBuilder.addPrimitive(prim);
			}
		}
	}

	sceneBuilder.setHitSolverType(HitSolverType::EMBREE);

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

	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");

	sceneBuilder.addEnvironment(domeLight);
	auto scene = sceneBuilder.build();
	const auto verticalFov = 75.f / 180.f * PI;
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	constexpr uint32_t SAMPLE_PER_PIXEL = 100u;
	auto sampler = std::make_shared<SimpleSampler>(SAMPLE_PER_PIXEL);

	PathIntegrator integrator{sampler, 20};
	integrator.setCamera(cam);
	integrator.render(*scene);
	EXPECT_NO_THROW(film->saveToFile(R"(D:\perfect_fresnel_test2_path.hdr)"));
}

TEST(MaterialTestSuite, PerfectFresnelSceneTest3)
{
	const auto transmission = std::make_shared<PerfectFresnelMaterial>(1.f, 2.5f);
	const auto matte = std::make_shared<MatteMaterial>(ColorRGB{1, 1, 1});
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto spherePrim = std::make_shared<Primitive>(sphere, transmission);
	const auto box = std::make_shared<Box>(Vector3f{-1000, -1000, -2}, Vector3f{1000, 1000, -1});
	const auto boxPrim = std::make_shared<Primitive>(box, matte);

	const auto dome = std::make_shared<DomeLight>(R"(D:\dome.hdr)");
	SceneBuilder sb;
	const auto scene = sb.addPrimitive(spherePrim)
						   .addPrimitive(boxPrim)
						   .addEnvironment(dome)
						   .setHitSolverType(HitSolverType::NAIVE)
						   .build();

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, -1.7, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);
	const auto verticalFov = toRadians(90.f);
	auto cam = CameraFactory::createPerspCamera(center, target, up.normalized(), verticalFov, 1,
												width, height);
	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(100);

	PathIntegrator integrator{sampler, 20};
	integrator.setCamera(cam);
	integrator.render(*scene);

	EXPECT_NO_THROW(film->saveToFile(R"(D:\perfect_fresnel_test3_path.hdr)"));
}