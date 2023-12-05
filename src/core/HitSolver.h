//
// Created by Frank on 2023/8/25.
//

#ifndef XD_RT_HITSOLVER_H
#define XD_RT_HITSOLVER_H

#include "CoreTypes.h"
#include "HitAccel.h"
#include "HitRecord.h"
#include "Ray.h"

namespace xd {
/**
 * @brief Hit acceleration structure specified on the whole scene
 */
class HitSolver : public HitAccel {
public:
	HitSolver(const std::shared_ptr<Scene>& scene);
	virtual ~HitSolver() = default;

protected:
	std::weak_ptr<Scene> sceneRef;
};

class NaiveHitSolver : public HitSolver {
public:
	explicit NaiveHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	std::unique_ptr<NoAccel> accel;
};

class BVHHitSolver : public HitSolver {
public:
	explicit BVHHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	BVHNode* root;
};

class EmbreeHitSolver : public HitSolver {
public:
	explicit EmbreeHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	std::shared_ptr<EmbreeAccel> accel;
};
}  // namespace xd
#endif	// XD_RT_HITSOLVER_H
