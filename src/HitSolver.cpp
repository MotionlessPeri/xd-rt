//
// Created by Frank on 2023/8/25.
//
#include "HitSolver.h"
#include "Primitive.h"
#include "Scene.h"
xd::HitSolver::HitSolver(const std::shared_ptr<Scene>& scene) : sceneRef(scene) {}
xd::NaiveHitSolver::NaiveHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene) {}
bool xd::NaiveHitSolver::solve(const Ray& ray, HitRecord& record) const
{
	const auto scene = sceneRef.lock();
	const auto& primitives = scene->getPrimitives();
	bool hit = false;
	record.tHit = std::numeric_limits<float>::max();
	for (const auto primitive : primitives) {
		auto model = primitive->getModel();
		if (model->hit(ray, record)) {
			hit = true;
			record.primitive = primitive;
		}
	}
	return hit;
}
xd::BVHHitSolver::BVHHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene)
{
	std::vector<const Model*> models;
	for (const auto prim : scene->getPrimitives()) {
		models.emplace_back(prim->getModel().get());
	}
	root = new BVHNode{models};
}
bool xd::BVHHitSolver::solve(const Ray& ray, HitRecord& record) const
{
	return root->hit(ray, record);
}
