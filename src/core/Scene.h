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
class Scene : public HitAggregate {
public:
	friend class SceneBuilder;
	const std::vector<std::shared_ptr<Primitive>>& getPrimitives() const { return primitives; }
	const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;
	std::shared_ptr<Light> getEnvironment() const { return environment; }

protected:
	Scene(std::vector<std::shared_ptr<Primitive>> primitives,
	      std::vector<std::shared_ptr<Light>> lights);
	void setHitSolver(std::unique_ptr<HitSolver> solver) { hitSolver = std::move(solver); }
	void setEnvironment(const std::shared_ptr<Light>& env) { environment = env; }
	std::vector<std::shared_ptr<Primitive>> primitives;
	std::vector<std::shared_ptr<Light>> lights;
	std::shared_ptr<Light> environment = nullptr;
	std::unique_ptr<HitSolver> hitSolver = nullptr;
};
}  // namespace xd
#endif	// XD_RT_SCENE_H
