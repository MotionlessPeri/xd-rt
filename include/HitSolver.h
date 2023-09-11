//
// Created by Frank on 2023/8/25.
//

#ifndef XD_RT_HITSOLVER_H
#define XD_RT_HITSOLVER_H

#include "CoreTypes.h"
#include "HitRecord.h"
#include "Ray.h"
namespace xd {
class HitSolver : public std::enable_shared_from_this<HitSolver> {
public:
	HitSolver(const std::shared_ptr<Scene>& scene);
	virtual bool solve(const Ray& ray, HitRecord& record) = 0;

protected:
	std::weak_ptr<Scene> sceneRef;
};

class NaiveHitSolver : public HitSolver {
public:
	explicit NaiveHitSolver(const std::shared_ptr<Scene>& scene);
	bool solve(const Ray& ray, HitRecord& record) override;
};
}  // namespace xd
#endif	// XD_RT_HITSOLVER_H
