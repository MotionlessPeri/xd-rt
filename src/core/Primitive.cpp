//
// Created by Frank on 2023/8/26.
//
#include "Primitive.h"
#include "AABB.h"
using namespace xd;
Primitive::Primitive(std::shared_ptr<Model> model, std::shared_ptr<Material> material)
	: model(std::move(model)), material(std::move(material))
{
}
Primitive::Primitive(std::shared_ptr<Model> model,
					 std::shared_ptr<Material> material,
					 const Transform& modelToWorld)
	: model(std::move(model)),
	  material(std::move(material)),
	  modelToWorld(modelToWorld),
	  worldToModel(modelToWorld.inverse())
{
}

bool Primitive::hit(const Ray& ray, HitRecord& rec) const
{
	// Note: seems the transform itself(worldToModel) suffers from precision issues
	// for example, rotate around z axis for 90 degree will cuz a minor scale
	// We might need better approach to build transform than using Eigen's
	const auto localRay = applyTransformToRay(worldToModel, ray);
	if (model->hit(localRay, rec)) {
		rec.primitive = std::dynamic_pointer_cast<const Primitive>(shared_from_this());
		rec.applyTransform(FrameCategory::WORLD, modelToWorld);
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
