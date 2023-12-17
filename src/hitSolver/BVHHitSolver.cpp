//
// Created by Frank on 2023/12/17.
//

#include "BVHHitSolver.h"
#include "Primitive.h"
#include "Scene.h"
#include "hitAccel/BVHNode.h"
using namespace xd;
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