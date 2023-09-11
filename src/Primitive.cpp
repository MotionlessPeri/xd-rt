//
// Created by Frank on 2023/8/26.
//
#include "Primitive.h"
using namespace xd;
Primitive::Primitive(const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& material)
	: model(model), material(material)
{
}
