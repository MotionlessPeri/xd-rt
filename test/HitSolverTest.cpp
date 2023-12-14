//
// Created by Frank on 2023/12/5.
//
#include "Distribution.h"
#include "Film.h"
#include "HitSolver.h"
#include "Integrator.h"
#include "Material.h"
#include "Model.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "camera/CameraFactory.h"
#include "gtest/gtest.h"
using namespace xd;

TEST(HitSolverTestSuite, EmbreeBuildTest)
{
	UniformDistribution<1> dis{};
	SceneBuilder sb;
	constexpr float MAX_RADIUS = 100;
	constexpr float HALF_WIDTH = 1000;
	for (auto i : std::views::iota(0u, 100u)) {
		const auto radius = dis.sample() * MAX_RADIUS;
		const Vector3f center{dis.sample() * HALF_WIDTH * 2 - HALF_WIDTH,
							  dis.sample() * HALF_WIDTH * 2 - HALF_WIDTH,
							  dis.sample() * HALF_WIDTH * 2 - HALF_WIDTH};
		const Transform transform{Eigen::Translation3f{center}};
		const auto model = std::make_shared<Sphere>(radius);
		const auto material = std::make_shared<PerfectReflectionMaterial>();
		const auto prim = std::make_shared<Primitive>(model, material, transform);
		sb.addPrimitive(prim);
	}
	EXPECT_NO_THROW(EmbreeHitSolver(sb.build()););
}

#include "HitRecord.h"
#include "Triangle.h"
TEST(HitSolverTestSuite, ConsistencyTest1)
{
	const float radius = 400.f;
	SceneBuilder sceneBuilder;
	const auto sphere = std::make_shared<Sphere>(radius);
	const auto triangulated = sphere->getTriangulatedMesh();
	const auto matte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	// const auto primitive = std::make_shared<Primitive>(sphere, matte);
	const auto primitive = std::make_shared<Primitive>(triangulated, matte);
	sceneBuilder.addPrimitive(primitive);
	sceneBuilder.setHitSolverType(xd::HitSolverType::NAIVE);
	const auto scene = sceneBuilder.build();

	const auto naiveHitSolver = NaiveHitSolver(scene);
	const auto bvhHitSolver = BVHHitSolver(scene);
	const auto embreeHitSolver = EmbreeHitSolver(scene);

	constexpr uint32_t width = 3u;
	constexpr uint32_t height = 3u;
	const Vector3f center = Vector3f{0, 1.5f * radius, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);
	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), radius, radius,
												width, height);
	Vector2f filmSample{1 + 0.5, 1 + 0.5};
	Vector3f expectedPos{0, radius, 0};
	Vector2f expectedUV{0.25, 0.5};
	// Note: the first strip(also last strip)'s uv is diffrent from analytical form because the
	// endpoint is reused
	const auto ray = cam->generateRay(filmSample);
	std::array<HitRecord, 3> records;
	naiveHitSolver.hit(ray, records[0]);
	bvhHitSolver.hit(ray, records[1]);
	embreeHitSolver.hit(ray, records[2]);
	for (const auto& rec : records) {
		EXPECT_TRUE(rec.p.isApprox(expectedPos));
		EXPECT_TRUE(rec.uv.isApprox(expectedUV));
	}
}

#include "oneapi/tbb.h"
TEST(HitSolverTestSuite, ConsistencyTest2)
{
	// TODO: we need to apply user-defined geometry in embree

	const float radius = 400.f;
	SceneBuilder sceneBuilder;
	const ColorRGB white{1, 1, 1};
	const auto sphere = std::make_shared<Sphere>(radius);
	const auto triangulated = sphere->getTriangulatedMesh();
	const auto matte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	// const auto primitive = std::make_shared<Primitive>(sphere, matte);
	const auto primitive = std::make_shared<Primitive>(triangulated, matte);

	sceneBuilder.addPrimitive(primitive);
	sceneBuilder.setHitSolverType(xd::HitSolverType::NAIVE);
	const auto naiveScene = sceneBuilder.build();
	sceneBuilder.setHitSolverType(xd::HitSolverType::BVH);
	const auto bvhScene = sceneBuilder.build();
	sceneBuilder.setHitSolverType(xd::HitSolverType::EMBREE);
	const auto embreeScene = sceneBuilder.build();

	constexpr uint32_t width = 200u;
	constexpr uint32_t height = 200u;
	const Vector3f center = Vector3f{0, 1.5f * radius, 0};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right = towards.cross(z).normalized();
	const Vector3f up = right.cross(towards);
	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), radius, radius,
												width, height);
	auto film = cam->getFilm();

	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
	// 1);
	std::array<Film, 3> films;
	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::UV);
	integrator.setCamera(cam);
	const auto renderScene = [&](const std::shared_ptr<Scene>& scene, Film& dstFilm) -> void {
		film->clear();
		integrator.render(*scene);
		dstFilm = *film;
	};

	renderScene(naiveScene, films[0]);
	renderScene(bvhScene, films[1]);
	renderScene(embreeScene, films[2]);

	for (auto row = 0u; row < height; ++row) {
		for (auto col = 0u; col < width; ++col) {
			const auto& pixel0 = films[0](row, col);
			const auto& pixel1 = films[1](row, col);
			const auto& pixel2 = films[2](row, col);
			EXPECT_EQ(pixel0, pixel1);
			EXPECT_EQ(pixel0, pixel2);
		}
	}
}

TEST(HitSolverTestSuite, ConsistencyTest3)
{
	const float radius = 400.f;
	SceneBuilder sceneBuilder;
	const auto sphere = std::make_shared<Sphere>(radius);
	const auto triangulated = sphere->getTriangulatedMesh();
	const auto matte = std::make_shared<MatteMaterial>(Vector3f{1, 1, 1});
	const auto primitive = std::make_shared<Primitive>(sphere, matte);

	sceneBuilder.addPrimitive(primitive);
	sceneBuilder.setHitSolverType(xd::HitSolverType::NAIVE);
	const auto naiveScene = sceneBuilder.build();
	sceneBuilder.setHitSolverType(xd::HitSolverType::BVH);
	const auto bvhScene = sceneBuilder.build();
	sceneBuilder.setHitSolverType(xd::HitSolverType::EMBREE);
	const auto embreeScene = sceneBuilder.build();

	constexpr uint32_t width = 1000u;
	constexpr uint32_t height = 1000u;
	const Vector3f center = Vector3f{0, 0, 1.5f * radius};
	const Vector3f z{0, 0, 1};
	const Vector3f target{0, 0, 0};
	const Vector3f towards = (target - center).normalized();
	const Vector3f right{0, -1, 0};
	const Vector3f up{1, 0, 0};
	auto cam = CameraFactory::createOrthoCamera(center, target, up.normalized(), radius, radius,
												width, height);
	auto film = cam->getFilm();

	DebugIntegrator integrator;
	integrator.setDebugChannel(DebugChannel::UV);
	integrator.setCamera(cam);
	const auto renderScene = [&](const std::shared_ptr<Scene>& scene,
								 const std::string& name) -> void {
		film->clear();
		integrator.render(*scene);
		std::string path = R"(D:\hitSolver_consistency_test3_)" + name + "_solver.hdr";
		EXPECT_NO_THROW(film->saveToFile(path););
	};

	renderScene(naiveScene, "naive");
	renderScene(bvhScene, "bvh");
	renderScene(embreeScene, "embree");
}