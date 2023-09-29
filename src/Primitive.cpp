//
// Created by Frank on 2023/8/26.
//
#include "Primitive.h"
#include "AABB.h"
using namespace xd;
Primitive::Primitive(const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& material)
	: model(model), material(material)
{
}
bool Primitive::hit(const Ray& ray, HitRecord& rec) const
{
	if (model->hit(ray, rec)) {
		rec.primitive = std::static_pointer_cast<const Primitive>(shared_from_this());
		return true;
	}
	return false;
}
float Primitive::getArea() const
{
	return model->getArea();
}
AABB Primitive::getAABB() const
{
	return model->getAABB();
}
