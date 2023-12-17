//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_NAIVEHITSOLVER_H
#define XD_RT_NAIVEHITSOLVER_H

#include "HitSolver.h"
#include "hitAccel/NoAccel.h"
namespace xd {
class NaiveHitSolver : public HitSolver {
public:
	explicit NaiveHitSolver(const std::shared_ptr<Scene>& scene);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	std::unique_ptr<NoAccel> accel;
};

}  // namespace xd

#endif	// XD_RT_NAIVEHITSOLVER_H
