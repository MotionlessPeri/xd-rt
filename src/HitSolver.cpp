//
// Created by Frank on 2023/8/25.
//
#include "HitSolver.h"
#include "Primitive.h"
#include "Scene.h"
using namespace xd;
HitSolver::HitSolver(const std::shared_ptr<Scene>& scene) : sceneRef(scene) {}
NaiveHitSolver::NaiveHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene) {}
bool NaiveHitSolver::solve(const Ray& ray, HitRecord& record) const
{
	const auto scene = sceneRef.lock();
	const auto& primitives = scene->getPrimitives();
	bool hit = false;
	for (const auto primitive : primitives) {
		auto model = primitive->getModel();
		if (model->hit(ray, record)) {
			hit = true;
			record.primitive = primitive;
		}
	}
	return hit;
}
BVHHitSolver::BVHHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene)
{
	std::vector<const Model*> models;
	for (const auto prim : scene->getPrimitives()) {
		models.emplace_back(prim->getModel().get());
	}
	root = new BVHNode{models};
}
bool BVHHitSolver::solve(const Ray& ray, HitRecord& record) const
{
	return root->hit(ray, record);
}
#include "EmbreeGlobal.h"
EmbreeHitSolver::EmbreeHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene)
{
	std::vector<const Model*> models;
	for (const auto& prim : scene->getPrimitives()) {
		models.emplace_back(prim->getModel().get());
	}
	accel = std::make_shared<EmbreeAccel>(EmbreeGlobal::get().device, models);
}
bool EmbreeHitSolver::solve(const Ray& ray, HitRecord& record) const
{
	return accel->hit(ray, record);
}
