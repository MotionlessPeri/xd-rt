//
// Created by Frank on 2023/10/2.
//
#include <oneapi/tbb.h>
#include "Film.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "camera/PerspCamera.h"
#include "gtest/gtest.h"
#include "integrator/DebugIntegrator.h"
#include "light/DomeLight.h"
#include "loader/GLTFSceneLoader.h"
#include "loader/TextureFactory.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
TEST(GLTFSceneLoaderTestSuite, loadTest)
{
	// const std::string path = R"(D:\cube_test.gltf)";
	const std::string path = R"(D:\buster_drone\scene.gltf)";
	GLTFSceneLoader loader;
	SceneBuilder builder;
	loader.loadToSceneBuilder(path, {}, builder);
	builder.setHitSolverType(HitSolverType::EMBREE);

	constexpr uint32_t width = 1024u;
	constexpr uint32_t height = 768u;
	const Vector3f center{0, 0, 1.5};
	const Vector3f target{0, -1, 0};
	const auto domeLight = std::make_shared<DomeLight>(R"(D:/dome.hdr)");
	builder.addEnvironment(domeLight);

	const auto verticalFov = 90.f / 180.f * PI;
	auto cam =
		CameraFactory::createPerspCamera(center, target, {0, 1, 0}, verticalFov, 1, width, height);

	auto film = cam->getFilm();

	auto sampler = std::make_shared<SimpleSampler>(1);
	const auto scene = builder.build();
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::SHADING_NORMAL);
	integrator.setCamera(cam);
	integrator.render(*scene);
	EXPECT_NO_THROW(film->saveToFile(R"(D:\gltf_load_test_debug_shading_normal.hdr)", {}));
	film->clear();
	integrator.setDebugChannel(DebugChannel::GEOM_NORMAL);
	integrator.render(*scene);
	EXPECT_NO_THROW(film->saveToFile(R"(D:\gltf_load_test_debug_geom_normal.hdr)", {}));
}