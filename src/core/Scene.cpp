//
// Created by Frank on 2023/8/25.
//
#include "Scene.h"
#include <cassert>
using namespace xd;
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

Scene::Scene(std::vector<std::shared_ptr<Primitive>> primitives,
			 std::vector<std::shared_ptr<Light>> lights,
			 std::vector<std::shared_ptr<Light>> infiniteLights)
	: primitives(std::move(primitives)),
	  lights(std::move(lights)),
	  infiniteLights(std::move(infiniteLights))
{
}
