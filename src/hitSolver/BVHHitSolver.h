//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_BVHHITSOLVER_H
#define XD_RT_BVHHITSOLVER_H
#include "HitSolver.h"
#include "hitAccel/HitAccelTypes.h"
namespace xd {
class BVHHitSolver : public HitSolver {
public:
	explicit BVHHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	BVHNode* root;
};
}  // namespace xd

#endif	// XD_RT_BVHHITSOLVER_H
