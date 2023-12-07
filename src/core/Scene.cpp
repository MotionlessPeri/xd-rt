//
// Created by Frank on 2023/8/25.
//
#include "Scene.h"
using namespace xd;
Scene::Scene(const std::vector<std::shared_ptr<Primitive>>& primitives,
			 const std::vector<std::shared_ptr<Light>>& lights)
	: primitives(primitives), lights(lights)
{
}

bool Scene::hit(const Ray& ray, HitRecord& rec) const
{
	assert(hitSolver);
	return hitSolver->hit(ray, rec);
}

bool Scene::hitAnything(const Ray& ray, HitRecord& rec) const
{
	assert(hitSolver);
	return hitSolver->hitAnything(ray, rec);
}