//
// Created by Frank on 2023/9/13.
//
#include "HitAccel.h"
#include "Model.h"
#include "Primitive.h"
using namespace xd;
NoAccel::NoAccel(const std::vector<const Model*>& models) : models(models) {}
bool NoAccel::hit(const Ray& ray, HitRecord& rec) const
{
	bool hit = false;
	for (const auto& model : models) {
		if (model->hit(ray, rec)) {
			hit = true;
		}
	}
	return hit;
}
bool NoAccel::hitAnything(const Ray& ray, HitRecord& rec) const
{
	for (const auto& model : models) {
		if (model->hit(ray, rec)) {
			return true;
		}
	}
	return false;
}
