//
// Created by Frank on 2023/12/17.
//

#include "NaiveHitSolver.h"
#include "Primitive.h"
#include "Scene.h"
using namespace xd;
NaiveHitSolver::NaiveHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene) {}

bool NaiveHitSolver::hit(const Ray& ray, HitRecord& rec) const
{
	const auto scene = sceneRef.lock();
	const auto& primitives = scene->getPrimitives();
	bool hit = false;
	for (const auto primitive : primitives) {
		if (primitive->hit(ray, rec)) {
			hit = true;
		}
	}
	return hit;
}
bool NaiveHitSolver::hitAnything(const Ray& ray, HitRecord& rec) const
{
	const auto scene = sceneRef.lock();
	const auto& primitives = scene->getPrimitives();
	for (const auto primitive : primitives) {
		if (primitive->hit(ray, rec)) {
			return true;
		}
	}
	return false;
}