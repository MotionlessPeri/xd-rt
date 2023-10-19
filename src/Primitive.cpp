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
	  localToWorld(localToWorld),
	  worldToLocal(localToWorld.inverse())
{
}

bool Primitive::hit(const Ray& ray, HitRecord& rec) const
{
	const Ray localRay{worldToLocal * ray.o,
					   worldToLocal.linear() * ray.d};
	if (model->hit(localRay, rec)) {
		rec.primitive = std::static_pointer_cast<const Primitive>(shared_from_this());
		rec.tPoint = localToWorld * rec.tPoint;
		rec.n = localToWorld.linear().inverse().transpose() * rec.n;
		rec.n.normalize();
		rec.dpdu = localToWorld.linear() * rec.dpdu;
		rec.dpdu.normalize();
		rec.dpdv = localToWorld.linear() * rec.dpdv;
		rec.dpdv.normalize();
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
