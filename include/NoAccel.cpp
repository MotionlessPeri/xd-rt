//
// Created by Frank on 2023/9/13.
//
#include "HitAccel.h"
#include "Model.h"
using namespace xd;
NoAccel::NoAccel(const std::vector<const Model*>& models) : models(models) {}
bool NoAccel::hit(const Ray& ray, HitRecord& rec) const
{
	bool hit = false;
	for (const auto* model : models) {
		if (model->hit(ray, rec)) {
			hit = true;
		}
	}
	return hit;
}
