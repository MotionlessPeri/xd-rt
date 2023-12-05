//
// Created by Frank on 2023/8/25.
//
#include "HitSolver.h"
#include "Primitive.h"
#include "Scene.h"
using namespace xd;
HitSolver::HitSolver(const std::shared_ptr<Scene>& scene) : sceneRef(scene) {}
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
	bool hit = false;
	for (const auto primitive : primitives) {
		if (primitive->hit(ray, rec)) {
			return true;
		}
	}
	return false;
}
BVHHitSolver::BVHHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene)
{
	std::vector<const Model*> models;
	for (const auto prim : scene->getPrimitives()) {
		models.emplace_back(prim.get());
	}
	root = new BVHNode{models};
}
bool BVHHitSolver::hit(const Ray& ray, HitRecord& rec) const
{
	return root->hit(ray, rec);
}
bool BVHHitSolver::hitAnything(const Ray& ray, HitRecord& rec) const
{
	return root->hitAnything(ray, rec);
}
#include "3rdParty/embree/EmbreeGlobal.h"
EmbreeHitSolver::EmbreeHitSolver(const std::shared_ptr<Scene>& scene) : HitSolver(scene)
{
	std::vector<const Primitive*> models;
	for (const auto& prim : scene->getPrimitives()) {
		models.emplace_back(prim.get());
	}
	accel = std::make_shared<EmbreeAccel>(EmbreeGlobal::get().device, models);
}
bool EmbreeHitSolver::hit(const Ray& ray, HitRecord& rec) const
{
	return accel->hit(ray, rec);
}
bool EmbreeHitSolver::hitAnything(const Ray& ray, HitRecord& rec) const
{
	return accel->hitAnything(ray, rec);
}
