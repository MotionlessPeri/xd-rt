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
Primitive::Primitive(const std::shared_ptr<Model>& model,
					 const std::shared_ptr<Material>& material,
					 const Transform& localToWorld)
	: model(model),
	  material(material),
	  modelToWorld(localToWorld),
	  worldToModel(localToWorld.inverse())
{
}

bool Primitive::hit(const Ray& ray, HitRecord& rec) const
{
	const Ray localRay{worldToModel * ray.o, (worldToModel.linear() * ray.d).normalized()};
	if (model->hit(localRay, rec)) {
		rec.primitive = shared_from_this();
		rec.p = modelToWorld * rec.p;
		rec.n = modelToWorld.linear().inverse().transpose() * rec.n;
		rec.n.normalize();
		rec.dpdu = modelToWorld.linear() * rec.dpdu;
		rec.dpdv = modelToWorld.linear() * rec.dpdv;
		rec.buildFrames();
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
