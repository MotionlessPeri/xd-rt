//
// Created by Frank on 2023/12/13.
//

#include "Film.h"
#include "Macros.h"
#include "Primitive.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "integrator/DirectIntegrator.h"
#include "light/DomeLight.h"
#include "loader/TextureFactory.h"
#include "material/MatteMaterial.h"
#include "model/Sphere.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
TEST(IntegratorTestSuite, AnalyticalSceneTest)
{
	const ColorRGB matteColor{1, 0, 0};
	const auto matte = std::make_shared<MatteMaterial>(matteColor);
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	const ColorRGB backgroundColor{1, 1, 1};
	const auto domeLight = std::make_shared<DomeLight>(backgroundColor);

	SceneBuilder sb;
	sb.addEnvironment(domeLight);
	sb.addPrimitive(prim);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	const Vector3f camPos{0, 1.7, 0};
	const Vector3f camTarget{0, 0, 0};
	const Vector3f camUp{0, 0, 1};
	const Vector3f camRight{0, 1, 0};
	const auto camVerticalFov = toRadians(90.f);
	const uint32_t width = 500u;
	const uint32_t height = 500u;
	auto cam = CameraFactory::createPerspCamera(camPos, camTarget, camUp.normalized(),
												camVerticalFov, 1, width, height);
	const auto film = cam->getFilm();

	const auto sampler = std::make_shared<SimpleSampler>(10);
#if 0
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::SHADOW_HIT);
	integrator.setCamera(cam);
	integrator.render(*scene);
	film->saveToFile(R"(D:\analytical_scene_test_debug.hdr)");
#endif
	// direct
	{
		// TBB_SERIAL
		film->clear();
		MIDirectIntegrator integrator{sampler};
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\analytical_scene_test_mi_direct.hdr)");
	}
	//// path
	//{
	//	film->clear();
	//	PathIntegrator integrator{{false}, sampler, 1};
	//	integrator.setCamera(cam);
	//	integrator.render(*scene);
	//	film->saveToFile(R"(D:\analytical_scene_test_path.hdr)");
	//}
}

TEST(IntegratorTestSuite, ImageDomeTest)
{
	const ColorRGB matteColor{1, 1, 1};
	const auto matte = std::make_shared<MatteMaterial>(matteColor);
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	const auto domeLight = std::make_shared<DomeLight>(R"(D:\dome.hdr)");

	SceneBuilder sb;
	sb.addEnvironment(domeLight);
	sb.addPrimitive(prim);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	const Vector3f camPos{0, 1.7, 0};
	const Vector3f camTarget{0, 0, 0};
	const Vector3f camUp{0, 0, 1};
	const Vector3f camRight{0, 1, 0};
	const auto camVerticalFov = toRadians(90.f);
	const uint32_t width = 500u;
	const uint32_t height = 500u;
	auto cam = CameraFactory::createPerspCamera(camPos, camTarget, camUp.normalized(),
												camVerticalFov, 1, width, height);
	const auto film = cam->getFilm();

	const auto sampler = std::make_shared<SimpleSampler>(20);
#if 0
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::SHADOW_HIT);
	integrator.setCamera(cam);
	integrator.render(*scene);
	film->saveToFile(R"(D:\analytical_scene_test_debug.hdr)");
#endif
	// direct
	{
		// TBB_SERIAL
		film->clear();
		MIDirectIntegrator integrator{sampler};
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\image_dome_scene_test.hdr)");
	}
}

TEST(IntegratorTestSuite, ImageMaterialTest)
{
	const auto image = TextureFactory::loadUVTextureRGB(R"(D:\uv_checker.jpg)");
	const auto matte = std::make_shared<MatteMaterial>(image);
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	const ColorRGB backgroundColor{1, 1, 1};
	const auto domeLight = std::make_shared<DomeLight>(backgroundColor);

	SceneBuilder sb;
	sb.addEnvironment(domeLight);
	sb.addPrimitive(prim);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	const Vector3f camPos{0, 1.7, 0};
	const Vector3f camTarget{0, 0, 0};
	const Vector3f camUp{0, 0, 1};
	const Vector3f camRight{0, 1, 0};
	const auto camVerticalFov = toRadians(90.f);
	const uint32_t width = 500u;
	const uint32_t height = 500u;
	auto cam = CameraFactory::createPerspCamera(camPos, camTarget, camUp.normalized(),
												camVerticalFov, 1, width, height);
	const auto film = cam->getFilm();

	const auto sampler = std::make_shared<SimpleSampler>(10);
#if 0
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::SHADOW_HIT);
	integrator.setCamera(cam);
	integrator.render(*scene);
	film->saveToFile(R"(D:\analytical_scene_test_debug.hdr)");
#endif
	// direct
	{
		// TBB_SERIAL
		film->clear();
		MIDirectIntegrator integrator{sampler};
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\image_mtl_scene_test.hdr)");
	}
}

TEST(IntegratorTestSuite, PureColorTest)
{
	// Note: this scene is analytical. The result should be matteColor * backgroundColor
	const ColorRGB matteColor{0.5, 0.5, 0.5};
	const auto matte = std::make_shared<MatteMaterial>(matteColor);
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	const ColorRGB backgroundColor{1, 1, 1};
	const auto domeLight = std::make_shared<DomeLight>(backgroundColor);

	SceneBuilder sb;
	sb.addEnvironment(domeLight);
	sb.addPrimitive(prim);
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();

	const Vector3f camPos{0, 1.7, 0};
	const Vector3f camTarget{0, 0, 0};
	const Vector3f camUp{0, 0, 1};
	const auto camVerticalFov = toRadians(90.f);
	const uint32_t width = 100u;
	const uint32_t height = 100u;
	auto cam = CameraFactory::createPerspCamera(camPos, camTarget, camUp.normalized(),
												camVerticalFov, 1, width, height);
	const auto film = cam->getFilm();

	const auto sampler = std::make_shared<SimpleSampler>(100);
#if 0
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::SINGLE_IRRADIANCE);
	integrator.setLightIndex(0u);
	integrator.setCamera(cam);
	integrator.render(*scene);
	film->saveToFile(R"(D:\pure_color_scene_test.hdr)");
#else
	// direct
	{
		film->clear();
		MIDirectIntegrator integrator{sampler};
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\pure_color_scene_test.hdr)");
	}
#endif
}