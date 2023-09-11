//
// Created by Frank on 2023/8/25.
//
#include "HitSolver.h"
#include "Primitive.h"
#include "Scene.h"
xd::HitSolver::HitSolver(const std::shared_ptr<Scene>& scene) : sceneRef(scene) {}
xd::NaiveHitSolver::NaiveHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene) {}
bool xd::NaiveHitSolver::solve(const xd::Ray& ray, xd::HitRecord& record)
{
	const auto scene = sceneRef.lock();
	const auto& primitives = scene->getPrimitives();
	bool hit = false;
	record.tHit = std::numeric_limits<float>::max();
	for (const auto primitive : primitives) {
		auto model = primitive->getModel();
		HitRecord nowRec;
		if (model->hit(ray, nowRec) && nowRec.tHit < record.tHit) {
			hit = true;
			record = nowRec;
			record.primitive = primitive;
		}
	}
	return hit;
}
