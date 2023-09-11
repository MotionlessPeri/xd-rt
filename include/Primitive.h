//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_PRIMITIVE_H
#define XD_RT_PRIMITIVE_H
#include "Material.h"
#include "Model.h"
namespace xd {
class Primitive {
public:
	Primitive(const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& material);
	std::shared_ptr<Model> getModel() const { return model; }
	std::shared_ptr<Material> getMaterial() const { return material; }

protected:
	std::shared_ptr<Model> model;
	std::shared_ptr<Material> material;
};
}  // namespace xd
#endif	// XD_RT_PRIMITIVE_H
