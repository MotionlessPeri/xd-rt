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
	HitSolver(const std::shared_ptr<Scene>& scene) : sceneRef(scene) {}
	virtual ~HitSolver() = default;

protected:
	std::weak_ptr<Scene> sceneRef;
};

}  // namespace xd
#endif	// XD_RT_HITSOLVER_H
