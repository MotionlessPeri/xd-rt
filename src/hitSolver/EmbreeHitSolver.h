//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_EMBREEHITSOLVER_H
#define XD_RT_EMBREEHITSOLVER_H

#include "HitSolver.h"
#include "hitAccel/HitAccelTypes.h"
namespace xd {
class EmbreeHitSolver : public HitSolver {
public:
	explicit EmbreeHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	std::shared_ptr<EmbreeAccel> accel;
};
}  // namespace xd
#endif	// XD_RT_EMBREEHITSOLVER_H
