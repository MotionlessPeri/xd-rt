//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_HITRECORD_H
#define XD_RT_HITRECORD_H

#include "CoreTypes.h"
#include "MathType.h"

namespace xd {
struct HitRecord {
	float tHit = FLT_MAX;
	Vector3f tPoint;
	// param representation
	Vector2f uv;
	// local frame
	Vector3f dpdu;
	Vector3f dpdv;
	Vector3f n;
	std::shared_ptr<const Primitive> primitive = nullptr;
	Matrix3f getLocalToWorld() const
	{
		Matrix3f localToWorld;
		constexpr float eps = 1e-4;
		const Vector3f tx = dpdu.normalized();
		Vector3f ty;
		if (dpdu.dot(dpdv) < eps) {
			ty = dpdv.normalized();
		}
		else {
			ty = n.cross(dpdu.normalized());
		}
		localToWorld << tx, ty, n;
		return localToWorld;
	}
	uint32_t debug = 0;
};
}  // namespace xd
#endif	// XD_RT_HITRECORD_H
