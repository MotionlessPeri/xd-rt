//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_SCENE_H
#define XD_RT_SCENE_H

#include <memory>
#include <vector>
#include "CoreTypes.h"
#include "HitSolver.h"

namespace xd {
class Scene {
public:
	void addPrimitive(const std::shared_ptr<Primitive>& primitive);
	const std::vector<std::shared_ptr<Primitive>>& getPrimitives() const { return primitives; }

	void addLight(const std::shared_ptr<Light>& light);
	const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }

protected:
	std::vector<std::shared_ptr<Primitive>> primitives;
	std::vector<std::shared_ptr<Light>> lights;
};
}  // namespace xd
#endif	// XD_RT_SCENE_H
