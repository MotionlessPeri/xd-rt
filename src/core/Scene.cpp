//
// Created by Frank on 2023/8/25.
//
#include "Scene.h"
using namespace xd;
void Scene::addPrimitive(const std::shared_ptr<Primitive>& primitive)
{
	primitives.emplace_back(primitive);
}
void Scene::addLight(const std::shared_ptr<Light>& light)
{
	lights.emplace_back(light);
}
