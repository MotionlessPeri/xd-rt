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
					 const Transform& modelToWorld)
	: model(model),
	  material(material),
	  modelToWorld(modelToWorld),
	  worldToModel(modelToWorld.inverse())
{
}

bool Primitive::hit(const Ray& ray, HitRecord& rec) const
{
	Ray localRay = ray;
	// Note: seems the transform itself(worldToModel) suffers from precision issues
	// for example, rotate around z axis for 90 degree will cuz a minor scale
	// We might need better approach to build transform than using Eigen's
	applyTransformToRay(worldToModel, localRay);
	if (model->hit(localRay, rec)) {
		rec.primitive = shared_from_this();
		rec.frame = FrameCategory::WORLD;
		applyTransformToPoint(modelToWorld, rec.p, &rec.pError);
		rec.n = modelToWorld.linear().inverse().transpose() * rec.n;
		rec.n.normalize();
		rec.dpdu = modelToWorld.linear() * rec.dpdu;
		rec.dpdv = modelToWorld.linear() * rec.dpdv;
		return true;
	}
	return false;
}
float Primitive::getArea() const
{
	// TODO: modify area according to modelToWorld
	// assert(false);
	return model->getArea();
}
AABB Primitive::getAABB() const
{
	// TODO: modify aabb according to modelToWorld
	// assert(false);
	return model->getAABB();
}
