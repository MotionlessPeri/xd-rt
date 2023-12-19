//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_PRIMITIVE_H
#define XD_RT_PRIMITIVE_H
#include "Hitable.h"
#include "Material.h"
#include "Model.h"
namespace xd {
class Primitive : public Model {
public:
	Primitive(std::shared_ptr<Model> model, std::shared_ptr<Material> material);
	Primitive(std::shared_ptr<Model> model,
	          std::shared_ptr<Material> material,
	          const Transform& modelToWorld);
	std::shared_ptr<Model> getModel() const { return model; }
	std::shared_ptr<Material> getMaterial() const { return material; }
	bool hit(const Ray& ray, HitRecord& rec) const override;
	float getArea() const override;
	AABB getAABB() const override;
	Transform getModelToWorld() const { return modelToWorld; }
	Transform getWorldToModel() const { return worldToModel; }

protected:
	std::shared_ptr<Model> model;
	std::shared_ptr<Material> material;
	Transform modelToWorld = Transform::Identity();
	Transform worldToModel = Transform::Identity();
};
}  // namespace xd
#endif	// XD_RT_PRIMITIVE_H
