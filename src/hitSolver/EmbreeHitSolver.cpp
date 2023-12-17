//
// Created by Frank on 2023/12/17.
//

#include "EmbreeHitSolver.h"
#include "3rdParty/embree/EmbreeGlobal.h"
#include "Scene.h"
#include "hitAccel/EmbreeHitAccel.h"
using namespace xd;
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
