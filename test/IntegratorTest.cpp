//
// Created by Frank on 2023/12/13.
//

#include "Film.h"
#include "Macros.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
#include "integrator/DebugIntegrator.h"
#include "integrator/DirectIntegrator.h"
#include "integrator/PathIntegrator.h"
#include "light/DomeLight.h"
#include "loader/TextureFactory.h"
#include "material/MatteMaterial.h"
#include "model/Sphere.h"
#include "sampler/SimpleSampler.h"
using namespace xd;

TEST(IntegratorTestSuite, SamplePerPixelTest)
{
	SceneBuilder sb;
	sb.setHitSolverType(HitSolverType::NAIVE);
	const auto scene = sb.build();
	const Vector3f camPos{1, 0, 0};
	const Vector3f camTarget{0, 0, 0};
	const Vector3f camUp{0, 0, 1};
	const Vector3f camRight{0, 1, 0};
	constexpr uint32_t width = 100u;
	constexpr uint32_t height = 100u;
	auto cam = CameraFactory::createOrthoCamera(camPos, camTarget, camUp.normalized(),
												camRight.norm(), camUp.norm(), width, height);
	const auto film = cam->getFilm();
	constexpr int samplePerPixel = 3ull;
	const auto testFunc = [&](std::shared_ptr<Integrator> integrator) {
		integrator->setCamera(cam);
		integrator->render(*scene);
		for (auto row = 0u; row < height; ++row) {
			for (auto col = 0u; col < width; ++col) {
				const auto& filmPixel = (*film)(row, col);
				EXPECT_FLOAT_EQ(filmPixel.weight, float(samplePerPixel));
			}
		}
		film->clear();
	};
	testFunc(std::make_shared<PathIntegrator>(std::make_shared<SimpleSampler>(samplePerPixel), 8));
	testFunc(std::make_shared<MIDirectIntegrator>(std::make_shared<SimpleSampler>(samplePerPixel)));
}

TEST(IntegratorTestSuite, AnalyticalSceneTest)
{
	const ColorRGB matteColor{1, 0, 0};
	const auto matte = std::make_shared<MatteMaterial>(matteColor);
	const auto sphere = std::make_shared<Sphere>(1.f);
	const auto prim = std::make_shared<Primitive>(sphere, matte);

	const ColorRGB backgroundColor{1, 1, 1};
	// const auto domeLight = std::make_shared<DomeLight>(backgroundColor);
	const auto domeLight = std::make_shared<DomeLight>(R"(D:\dome_pure_white.hdr)");
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

	const auto sampler = std::make_shared<SimpleSampler>(100);
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
	// path
	{
		film->clear();
		PathIntegrator integrator{{false}, sampler, 8};
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\analytical_scene_test_path.hdr)");
	}
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
	// const auto image = TextureFactoryNew::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	LoadTextureOptions options;
	options.filterType = FilterType::NEAREST;
	const auto image = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)", options);
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
	const uint32_t width = 1000u;
	const uint32_t height = 1000u;
	auto cam = CameraFactory::createPerspCamera(camPos, camTarget, camUp.normalized(),
												camVerticalFov, 1, width, height);
	const auto film = cam->getFilm();

	const auto sampler = std::make_shared<SimpleSampler>(10);
	// direct
	{
		// TBB_SERIAL
		film->clear();
		MIDirectIntegrator integrator{sampler};
		// DebugIntegrator integrator;
		// integrator.setDebugChannel(DebugChannel::BXDF);
		integrator.setCamera(cam);
		integrator.render(*scene);
		film->saveToFile(R"(D:\image_mtl_scene_test_nearest.hdr)");
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

	const auto sampler = std::make_shared<SimpleSampler>(1000);
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