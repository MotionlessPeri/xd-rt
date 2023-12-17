//
// Created by Frank on 2023/11/29.
//

#include "SceneBuilder.h"
#include "Scene.h"
#include "hitSolver/BVHHitSolver.h"
#include "hitSolver/EmbreeHitSolver.h"
#include "hitSolver/NaiveHitSolver.h"
using namespace xd;
std::shared_ptr<Scene> SceneBuilder::build() const
{
	auto scene = std::shared_ptr<Scene>(new Scene{primitives, lights});
	scene->setHitSolver(buildHitSolver(scene, hitSolverType));
	scene->setEnvironment(environment);
	return scene;
}
std::unique_ptr<HitSolver> SceneBuilder::buildHitSolver(const std::shared_ptr<Scene>& scene,
														HitSolverType type) const
{
	switch (type) {
		case HitSolverType::NAIVE:
			return std::make_unique<NaiveHitSolver>(scene);
		case HitSolverType::BVH:
			return std::make_unique<BVHHitSolver>(scene);
		case HitSolverType::EMBREE:
			return std::make_unique<EmbreeHitSolver>(scene);
	}
	return nullptr;
}
